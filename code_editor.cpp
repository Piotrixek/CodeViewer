#include "code_editor.h"
#include "imgui.h"
#include "code_capture.h"

#include "tinyfiledialogs.h" 
#include <fstream>           
#include <sstream>           
#include <vector>
#include <string>
#include <unordered_set>     
#include <algorithm>         
#include <exception>         
#include <cctype>            
#include <cmath>             


const std::unordered_set<std::string> cppKeywords = {
    "int", "float", "double", "char", "bool", "void", "class", "struct", "enum", "union",
    "if", "else", "switch", "case", "default", "for", "while", "do", "break", "continue",
    "return", "goto", "const", "static", "public", "private", "protected", "namespace",
    "using", "template", "typename", "try", "catch", "throw", "new", "delete", "nullptr",
    "auto", "constexpr", "virtual", "override", "final", "#include", "#define", "#ifdef",
    "#ifndef", "#endif", "#pragma"
};
const std::unordered_set<std::string> pythonKeywords = { 
    "False", "None", "True", "and", "as", "assert", "async", "await", "break", "class", "continue", "def", "del", "elif", "else", "except", "finally", "for", "from", "global", "if", "import", "in", "is", "lambda", "nonlocal", "not", "or", "pass", "raise", "return", "try", "while", "with", "yield"
};
const std::unordered_set<std::string> jsKeywords = { 
    "abstract", "arguments", "await", "boolean", "break", "byte", "case", "catch", "char", "class", "const", "continue", "debugger", "default", "delete", "do", "double", "else", "enum", "eval", "export", "extends", "false", "final", "finally", "float", "for", "function", "goto", "if", "implements", "import", "in", "instanceof", "int", "interface", "let", "long", "native", "new", "null", "package", "private", "protected", "public", "return", "short", "static", "super", "switch", "synchronized", "this", "throw", "throws", "transient", "true", "try", "typeof", "var", "void", "volatile", "while", "with", "yield"
};
const std::unordered_set<std::string> cssKeywords = { 
    "color", "background-color", "font-size", "font-family", "font-weight", "text-align", "margin", "padding", "border", "width", "height", "display", "position", "top", "left", "right", "bottom", "float", "clear", "overflow", "z-index", "opacity", "border-radius", "box-shadow", "text-decoration", "line-height", "letter-spacing", "content", "cursor", "transition", "transform"
};
const std::unordered_set<std::string> htmlKeywords = { 
    "html", "head", "title", "body", "div", "span", "p", "a", "img", "ul", "ol", "li", "table", "tr", "td", "th", "form", "input", "button", "select", "option", "textarea", "h1", "h2", "h3", "h4", "h5", "h6", "strong", "em", "br", "hr", "link", "meta", "style", "script", "header", "footer", "nav", "section", "article", "aside"
};




std::string get_file_ext(const std::string& fname) {
    size_t dot_pos = fname.find_last_of(".");
    if (dot_pos == std::string::npos) return "";
    return fname.substr(dot_pos + 1);
}


int detect_lang(const std::string& fname) {
    std::string ext = get_file_ext(fname);
    
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
    if (ext == "cpp" || ext == "h" || ext == "hpp" || ext == "cxx" || ext == "hxx" || ext == "c") return 0; 
    if (ext == "py" || ext == "pyw") return 1; 
    if (ext == "html" || ext == "htm") return 2; 
    if (ext == "css") return 3; 
    if (ext == "js") return 4; 
    return -1; 
}


bool load_file_str(const char* path, std::string& content_out) {
    std::ifstream file(path, std::ios::binary | std::ios::ate); 
    if (!file.is_open()) {
        tinyfd_messageBox("Error", "Cant open file", "ok", "error", 1);
        return false;
    }
    std::streamsize len = file.tellg(); 
    file.seekg(0, std::ios::beg); 

    
    const long long max_size = 20LL * 1024 * 1024; 
    if (len > max_size || len < 0) {
        tinyfd_messageBox("Error", len > max_size ? "File too big (> 20MB)" : "Bad file size", "ok", "error", 1);
        file.close();
        return false;
    }

    try {
        content_out.resize(static_cast<size_t>(len)); 
        if (len > 0) {
            file.read(&content_out[0], len); 
            if (!file) { 
                tinyfd_messageBox("Error", "Error readin file content", "ok", "error", 1);
                file.close();
                content_out.clear(); 
                return false;
            }
        }
    }
    catch (const std::exception& e) { 
        std::string err_msg = "Error readin file: ";
        err_msg += e.what();
        tinyfd_messageBox("Error", err_msg.c_str(), "ok", "error", 1);
        file.close();
        return false;
    }
    catch (...) { 
        tinyfd_messageBox("Error", "Unknown error readin file", "ok", "error", 1);
        file.close();
        return false;
    }

    file.close();
    return true; 
}


std::string strip_em(const std::string& code, int lang) {
    std::string result = "";
    std::stringstream ss(code);
    std::string line;
    bool in_multi_comment = false;

    while (std::getline(ss, line)) {
        std::string processed_line = "";
        bool line_had_content = false; 

        
        if (lang == 0 || lang == 4 || lang == 3) {
            size_t current_pos = 0;
            while (current_pos < line.length()) {
                if (in_multi_comment) {
                    size_t end_comment = line.find("*/", current_pos);
                    if (end_comment != std::string::npos) {
                        current_pos = end_comment + 2;
                        in_multi_comment = false; 
                    }
                    else {
                        break; 
                    }
                }
                else {
                    
                    size_t single_comment = (lang == 0 || lang == 4) ? line.find("//", current_pos) : std::string::npos;
                    size_t multi_comment_start = line.find("/*", current_pos);

                    size_t next_comment_start = std::string::npos;
                    if (single_comment != std::string::npos && multi_comment_start != std::string::npos) {
                        next_comment_start = std::min(single_comment, multi_comment_start);
                    }
                    else if (single_comment != std::string::npos) {
                        next_comment_start = single_comment;
                    }
                    else {
                        next_comment_start = multi_comment_start;
                    }

                    
                    if (next_comment_start == std::string::npos) {
                        processed_line += line.substr(current_pos);
                        
                        if (!line.substr(current_pos).empty() && line.substr(current_pos).find_first_not_of(" \t\r\n") != std::string::npos) {
                            line_had_content = true;
                        }
                        break; 
                    }

                    
                    processed_line += line.substr(current_pos, next_comment_start - current_pos);
                    if (!line.substr(current_pos, next_comment_start - current_pos).empty() && line.substr(current_pos, next_comment_start - current_pos).find_first_not_of(" \t\r\n") != std::string::npos) {
                        line_had_content = true;
                    }


                    
                    if (next_comment_start == single_comment) {
                        break; 
                    }
                    else { 
                        
                        size_t end_comment = line.find("*/", next_comment_start + 2);
                        if (end_comment != std::string::npos) {
                            current_pos = end_comment + 2; 
                        }
                        else {
                            in_multi_comment = true; 
                            break; 
                        }
                    }
                }
            }
        }
        
        else if (lang == 1) {
            size_t comment_pos = line.find("#");
            processed_line += line.substr(0, comment_pos); 
            if (!processed_line.empty() && processed_line.find_first_not_of(" \t\r\n") != std::string::npos) {
                line_had_content = true;
            }
            
        }
        
        else {
            processed_line += line;
            if (!processed_line.empty() && processed_line.find_first_not_of(" \t\r\n") != std::string::npos) {
                line_had_content = true;
            }
        }

        
        size_t endpos = processed_line.find_last_not_of(" \t\r\n");
        if (std::string::npos != endpos) {
            processed_line = processed_line.substr(0, endpos + 1);
        }
        else {
            processed_line.clear(); 
        }

        
        if (!processed_line.empty() || line_had_content) {
            result += processed_line + "\n";
        }
    }
    return result;
}



void process_code(CodeDocument& doc) {
    if (doc.showComments) {
        
        if (doc.processedContent.size() != doc.content.size() || doc.processedContent != doc.content) {
            doc.processedContent = doc.content;
        }
    }
    else {
        
        std::string stripped = strip_em(doc.content, doc.language);
        
        if (doc.processedContent.size() != stripped.size() || doc.processedContent != stripped) {
            doc.processedContent = std::move(stripped); 
        }
    }
}



void ShowCodeViewerUI(bool* p_open, std::vector<CodeDocument>& docs, int& active_doc_idx)
{
    
    if (p_open && !*p_open) {
        return;
    }

    
    static const SyntaxColors syntaxColors;

    
    ImGuiWindowFlags win_flags = ImGuiWindowFlags_MenuBar;

    
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    if (viewport) {
        ImVec2 center_pos = ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.5f,
            viewport->WorkPos.y + viewport->WorkSize.y * 0.5f);
        ImVec2 initial_size = ImVec2(viewport->WorkSize.x * 0.7f, viewport->WorkSize.y * 0.7f);
        ImGui::SetNextWindowPos(center_pos, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f)); 
        ImGui::SetNextWindowSize(initial_size, ImGuiCond_FirstUseEver);
    }
    else { 
        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    }

    
    if (!ImGui::Begin("Code Viewer", p_open, win_flags)) {
        
        ImGui::End();
        return;
    }

    
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open File...")) {
                const char* filters[8] = { "*.cpp", "*.h", "*.hpp", "*.c", "*.py", "*.html", "*.css", "*.js" };
                const char* file_path = tinyfd_openFileDialog("Open Code File", "", 8, filters, NULL, 0);
                if (file_path != NULL) {
                    std::string content_str;
                    if (load_file_str(file_path, content_str)) { 
                        std::string path_str = file_path;
                        std::string name_str = path_str.substr(path_str.find_last_of("/\\") + 1);
                        bool already_open = false;
                        int open_idx = -1;
                        for (int i = 0; i < docs.size(); ++i) {
                            if (docs[i].filePath == path_str) {
                                already_open = true; open_idx = i; break;
                            }
                        }
                        if (!already_open) {
                            docs.emplace_back(path_str, name_str, content_str);
                            docs.back().language = detect_lang(name_str); 
                            process_code(docs.back()); 
                            active_doc_idx = docs.size() - 1;
                        }
                        else {
                            active_doc_idx = open_idx;
                        }
                    }
                }
            }
            if (ImGui::MenuItem("Close Current", NULL, false, active_doc_idx >= 0 && !docs.empty())) {
                if (active_doc_idx >= 0 && active_doc_idx < docs.size()) {
                    docs[active_doc_idx].open = false;
                }
            }
            if (ImGui::MenuItem("Exit")) {
                if (p_open) *p_open = false;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }


    
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_FittingPolicyScroll;
    if (ImGui::BeginTabBar("CodeTabs", tab_bar_flags)) {
        int doc_to_close_idx = -1;

        if (active_doc_idx >= docs.size()) {
            active_doc_idx = docs.empty() ? -1 : docs.size() - 1;
        }

        
        static std::vector<bool> in_multiline_comment_states;
        
        if (in_multiline_comment_states.size() < docs.size()) {
            in_multiline_comment_states.resize(docs.size(), false);
        }


        for (int n = 0; n < docs.size(); ++n) {
            
            if (n >= in_multiline_comment_states.size()) {
                in_multiline_comment_states.resize(n + 1, false);
            }

            CodeDocument& current_doc = docs[n];
            if (!current_doc.open) continue;

            ImGuiTabItemFlags tab_flags = ImGuiTabItemFlags_None;
            bool tab_visible = ImGui::BeginTabItem(current_doc.fileName.c_str(), &current_doc.open, tab_flags);

            if (!current_doc.open) {
                doc_to_close_idx = n;
            }

            if (tab_visible) {
                if (ImGui::IsItemActivated() || (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) && active_doc_idx != n)) {
                    active_doc_idx = n;
                    
                    
                }

                
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
                if (ImGui::Checkbox("Show Comments", &current_doc.showComments)) {
                    process_code(current_doc); 
                    
                    if (n < in_multiline_comment_states.size()) in_multiline_comment_states[n] = false; 
                }
                ImGui::SameLine(); ImGui::TextDisabled("(Lang: %d)", current_doc.language); 

                
                ImGui::SameLine(); 
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.7f, 0.3f, 1.0f)); 
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.8f, 0.4f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.9f, 0.5f, 1.0f));
                if (ImGui::Button("Save as Image")) {
                    
                    capture_code_img(current_doc);
                }
                ImGui::PopStyleColor(3); 
                


                ImGui::Separator();
                ImGui::PopStyleVar(); 

                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
                ImGui::BeginChild("CodeAreaChild", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

                extern ImFont* g_pCodeFont;
                if (g_pCodeFont) ImGui::PushFont(g_pCodeFont);

                
                int line_count = 0;
                for (char c : current_doc.processedContent) { if (c == '\n') line_count++; }
                if (!current_doc.processedContent.empty() && current_doc.processedContent.back() != '\n') line_count++;
                line_count = std::max(1, line_count);
                char line_no_fmt[16];
                int max_digits = (line_count == 0) ? 1 : ((int)log10(line_count) + 1); 
                sprintf_s(line_no_fmt, sizeof(line_no_fmt), "%%-%dd | ", max_digits);
                
                char max_line_no_str[16];
                sprintf_s(max_line_no_str, sizeof(max_line_no_str), "%d | ", line_count);
                float line_no_width = ImGui::CalcTextSize(max_line_no_str).x;

                
                std::stringstream ss(current_doc.processedContent);
                std::string line;
                int current_line_num = 1;
                

                while (std::getline(ss, line)) {
                    ImGui::TextDisabled(line_no_fmt, current_line_num++);
                    ImGui::SameLine(line_no_width);

                    size_t current_pos = 0;
                    while (current_pos < line.length()) {

                        
                        
                        if (current_doc.showComments && n < in_multiline_comment_states.size() && in_multiline_comment_states[n]) {
                            size_t end_comment_pos = line.find("*/", current_pos);
                            size_t render_until = line.length();
                            bool closed_on_this_line = false;

                            if (end_comment_pos != std::string::npos) {
                                render_until = end_comment_pos + 2;
                                closed_on_this_line = true;
                            }

                            ImGui::PushStyleColor(ImGuiCol_Text, syntaxColors.comment);
                            ImGui::TextUnformatted(line.c_str() + current_pos, line.c_str() + render_until);
                            ImGui::PopStyleColor();

                            current_pos = render_until;

                            if (closed_on_this_line) {
                                in_multiline_comment_states[n] = false; 
                            }

                            if (current_pos < line.length()) { ImGui::SameLine(0.0f, 0.0f); }
                            continue;
                        }
                        

                        
                        size_t start_token = current_pos;
                        while (start_token < line.length() && isspace(line[start_token])) {
                            start_token++;
                        }
                        if (start_token > current_pos) {
                            ImGui::PushStyleColor(ImGuiCol_Text, syntaxColors.default_text);
                            ImGui::TextUnformatted(line.c_str() + current_pos, line.c_str() + start_token);
                            ImGui::PopStyleColor();
                            if (start_token < line.length()) { ImGui::SameLine(0.0f, 0.0f); }
                            current_pos = start_token;
                        }

                        if (current_pos == line.length()) break;

                        start_token = current_pos;
                        char c = line[current_pos];
                        ImVec4 current_color = syntaxColors.default_text;
                        size_t next_pos = current_pos + 1;

                        
                        bool comment_handled = false;
                        if (current_doc.showComments && n < in_multiline_comment_states.size()) { 
                            size_t single_line_comment_pos = std::string::npos;
                            size_t multi_line_comment_pos = std::string::npos;

                            if (current_doc.language == 0 || current_doc.language == 4) {
                                single_line_comment_pos = line.find("//", current_pos);
                                multi_line_comment_pos = line.find("/*", current_pos);
                            }
                            else if (current_doc.language == 1) {
                                single_line_comment_pos = line.find("#", current_pos);
                            }
                            else if (current_doc.language == 3) {
                                multi_line_comment_pos = line.find("/*", current_pos);
                            }

                            size_t first_comment_pos = std::min(single_line_comment_pos, multi_line_comment_pos);

                            if (first_comment_pos == current_pos) {
                                comment_handled = true;
                                current_color = syntaxColors.comment;

                                if (first_comment_pos == single_line_comment_pos) {
                                    next_pos = line.length();
                                }
                                else { 
                                    next_pos = current_pos + 2;
                                    in_multiline_comment_states[n] = true; 

                                    size_t end_comment_pos = line.find("*/", next_pos);
                                    if (end_comment_pos != std::string::npos) {
                                        next_pos = end_comment_pos + 2;
                                        in_multiline_comment_states[n] = false; 
                                    }
                                    else {
                                        next_pos = line.length();
                                    }
                                }
                            }
                        }
                        

                        
                        if (!comment_handled) {
                            
                            if (current_doc.language == 0 && c == '#') {
                                current_color = syntaxColors.preprocessor;
                                next_pos = line.length();
                            }
                            
                            else if (c == '"' || c == '\'' || (current_doc.language == 4 && c == '`')) {
                                current_color = syntaxColors.string_literal;
                                char string_char = c;
                                next_pos = current_pos + 1;
                                while (next_pos < line.length()) {
                                    if (line[next_pos] == '\\' && next_pos + 1 < line.length()) {
                                        next_pos += 2; continue;
                                    }
                                    if (line[next_pos] == string_char) {
                                        next_pos++; break;
                                    }
                                    next_pos++;
                                }
                            }
                            
                            else if (current_doc.language == 2 && c == '<') {
                                current_color = syntaxColors.html_tag;
                                next_pos = current_pos + 1;
                                while (next_pos < line.length() && line[next_pos] != '>') { next_pos++; }
                                if (next_pos < line.length()) next_pos++;
                            }
                            
                            else if (current_doc.language == 3) {
                                next_pos = current_pos; 
                                if (isalpha(c) || c == '.' || c == '#') {
                                    while (next_pos < line.length() && (isalnum(line[next_pos]) || line[next_pos] == '-' || line[next_pos] == '_' || line[next_pos] == '.' || line[next_pos] == '#')) { next_pos++; }
                                    std::string token = line.substr(start_token, next_pos - start_token);
                                    size_t colon_pos = line.find(':', next_pos);
                                    bool likely_prop = (colon_pos != std::string::npos);
                                    if (likely_prop || cssKeywords.count(token)) { current_color = syntaxColors.css_property; }
                                    else { current_color = syntaxColors.css_selector; }
                                }
                                else if (isdigit(c) || (c == '.' && current_pos + 1 < line.length() && isdigit(line[current_pos + 1]))) {
                                    while (next_pos < line.length() && (isdigit(line[next_pos]) || line[next_pos] == '.' || isalpha(line[next_pos]))) { next_pos++; } 
                                    current_color = syntaxColors.number_literal;
                                }
                                else {
                                    next_pos = current_pos + 1;
                                }
                            }
                            
                            else if (isalpha(c) || c == '_') {
                                next_pos = current_pos;
                                while (next_pos < line.length() && (isalnum(line[next_pos]) || line[next_pos] == '_')) { next_pos++; }
                                std::string token = line.substr(start_token, next_pos - start_token);
                                bool is_keyword = false;
                                if (current_doc.language == 0 && cppKeywords.count(token)) is_keyword = true;
                                else if (current_doc.language == 1 && pythonKeywords.count(token)) is_keyword = true;
                                else if (current_doc.language == 4 && jsKeywords.count(token)) is_keyword = true;
                                if (is_keyword) current_color = syntaxColors.keyword;
                            }
                            
                            else if (isdigit(c) || (c == '.' && current_pos + 1 < line.length() && isdigit(line[current_pos + 1]))) {
                                next_pos = current_pos;
                                while (next_pos < line.length() && (isdigit(line[next_pos]) || line[next_pos] == '.' || tolower(line[next_pos]) == 'f')) { next_pos++; }
                                current_color = syntaxColors.number_literal;
                            }
                            
                            else {
                                next_pos = current_pos + 1;
                            }
                        } 

                        ImGui::PushStyleColor(ImGuiCol_Text, current_color);
                        ImGui::TextUnformatted(line.c_str() + start_token, line.c_str() + next_pos);
                        ImGui::PopStyleColor();

                        current_pos = next_pos;

                        if (current_pos < line.length()) { ImGui::SameLine(0.0f, 0.0f); }

                    } 
                } 

                if (g_pCodeFont) ImGui::PopFont();
                ImGui::EndChild();
                ImGui::PopStyleVar();

                ImGui::EndTabItem();
            } 
            else {
                
                if (n < in_multiline_comment_states.size()) {
                    in_multiline_comment_states[n] = false;
                }
            }

        } 

        
        if (doc_to_close_idx != -1) {
            if (doc_to_close_idx < in_multiline_comment_states.size()) {
                in_multiline_comment_states.erase(in_multiline_comment_states.begin() + doc_to_close_idx);
            }
            docs.erase(docs.begin() + doc_to_close_idx);
            if (docs.empty()) {
                active_doc_idx = -1;
            }
            else if (active_doc_idx >= doc_to_close_idx) {
                active_doc_idx = std::max(0, active_doc_idx - 1);
                active_doc_idx = std::min(active_doc_idx, (int)docs.size() - 1);
            }
        }

        ImGui::EndTabBar();
    }
    else if (docs.empty()) {
        ImVec2 content_avail = ImGui::GetContentRegionAvail();
        ImVec2 text_size = ImGui::CalcTextSize("Open a file using the File menu.");
        ImVec2 padding = ImGui::GetStyle().WindowPadding;
        ImGui::SetCursorPos(ImVec2(padding.x + (content_avail.x - text_size.x) * 0.5f,
            padding.y + (content_avail.y - text_size.y) * 0.5f));
        ImGui::TextDisabled("Open a file using the File menu.");
    }

    ImGui::End(); 
}


