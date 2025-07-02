#include "code_editor.h"
#include "file_utils.h"
#include "ui_addons.h"
#include "imgui.h"
#include "code_capture.h"
#include "tinyfiledialogs.h"
#include <vector>
#include <string>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>

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

void ShowCodeViewerUI(bool* p_open, std::vector<CodeDocument>& docs, int& active_doc_idx)
{
    if (p_open && !*p_open) {
        return;
    }

    static const SyntaxColors syntaxColors;
    ImGuiWindowFlags win_flags = ImGuiWindowFlags_MenuBar;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 center_pos = ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.5f, viewport->WorkPos.y + viewport->WorkSize.y * 0.5f);
    ImVec2 initial_size = ImVec2(viewport->WorkSize.x * 0.7f, viewport->WorkSize.y * 0.7f);
    ImGui::SetNextWindowPos(center_pos, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(initial_size, ImGuiCond_FirstUseEver);

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
                        docs.emplace_back(path_str, name_str, content_str);
                        docs.back().language = detect_lang(name_str);
                        process_code(docs.back());
                        active_doc_idx = docs.size() - 1;
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
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Search", "Ctrl+F", false, active_doc_idx != -1)) {
                if (active_doc_idx >= 0 && active_doc_idx < docs.size()) {
                    docs[active_doc_idx].searchState.active = true;
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (ImGui::BeginTabBar("CodeTabs", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_FittingPolicyScroll)) {
        int doc_to_close_idx = -1;
        static std::vector<bool> in_multiline_comment_states;
        if (in_multiline_comment_states.size() < docs.size()) {
            in_multiline_comment_states.resize(docs.size(), false);
        }

        for (int n = 0; n < docs.size(); ++n) {
            if (n >= docs.size()) continue;
            CodeDocument& current_doc = docs[n];
            if (!current_doc.open) continue;

            bool tab_visible = ImGui::BeginTabItem(current_doc.fileName.c_str(), &current_doc.open, ImGuiTabItemFlags_None);
            if (!current_doc.open) {
                doc_to_close_idx = n;
            }

            if (tab_visible) {
                active_doc_idx = n;
                if (ImGui::Checkbox("Show Comments", &current_doc.showComments)) {
                    process_code(current_doc);
                }
                ImGui::SameLine();
                if (ImGui::Button("Save as Image")) {
                    extern ImFont* g_pCodeFont;
                    capture_code_to_image(current_doc, syntaxColors, g_pCodeFont);
                }

                ImGui::Separator();

                int line_count = 0;
                for (char c : current_doc.processedContent) { if (c == '\n') line_count++; }
                if (!current_doc.processedContent.empty() && current_doc.processedContent.back() != '\n') line_count++;
                line_count = std::max(1, line_count);

                float footer_height = ImGui::GetFrameHeightWithSpacing() * (current_doc.searchState.active ? 2.5f : 1.0f);
                ImGui::BeginChild("CodeAreaChild", ImVec2(0, -footer_height), false, ImGuiWindowFlags_HorizontalScrollbar);

                float line_height = ImGui::GetTextLineHeightWithSpacing();
                if (current_doc.searchState.scrollToMatch) {
                    int line_to_scroll = current_doc.searchState.lineToScrollTo;

                    if (line_to_scroll == -1 && current_doc.searchState.currentMatch != -1) {
                        size_t match_pos = current_doc.searchState.matchPositions[current_doc.searchState.currentMatch];
                        line_to_scroll = 0;
                        for (size_t i = 0; i < match_pos; ++i) {
                            if (current_doc.processedContent[i] == '\n') {
                                line_to_scroll++;
                            }
                        }
                        line_to_scroll++;
                    }

                    float target_y = ((line_to_scroll - 1) * line_height) - (ImGui::GetWindowHeight() / 2.0f);
                    ImGui::SetScrollY(target_y);
                    current_doc.searchState.scrollToMatch = false;
                    current_doc.searchState.lineToScrollTo = -1;
                }

                extern ImFont* g_pCodeFont;
                if (g_pCodeFont) ImGui::PushFont(g_pCodeFont);

                char line_no_fmt[16];
                int max_digits = (line_count == 0) ? 1 : ((int)log10(line_count) + 1);
                sprintf_s(line_no_fmt, sizeof(line_no_fmt), "%%-%dd | ", max_digits);
                char max_line_no_str[16];
                sprintf_s(max_line_no_str, sizeof(max_line_no_str), "%d | ", line_count);
                float line_no_width = ImGui::CalcTextSize(max_line_no_str).x;

                std::stringstream ss(current_doc.processedContent);
                std::string line;
                int current_line_num = 1;
                size_t char_offset = 0;

                while (std::getline(ss, line)) {
                    ImGui::TextDisabled(line_no_fmt, current_line_num++);
                    ImGui::SameLine(line_no_width);

                    if (current_doc.searchState.active && !current_doc.searchState.matchPositions.empty()) {
                        ImDrawList* draw_list = ImGui::GetWindowDrawList();
                        const ImVec2 p = ImGui::GetCursorScreenPos();
                        float line_height_nodraw = ImGui::GetTextLineHeight();
                        size_t query_len = strlen(current_doc.searchState.query);

                        for (size_t match_pos : current_doc.searchState.matchPositions) {
                            if (match_pos >= char_offset && match_pos < char_offset + line.length() + 1) {
                                std::string line_substr = line.substr(0, match_pos - char_offset);
                                float highlight_x_start = p.x + ImGui::CalcTextSize(line_substr.c_str()).x;
                                float highlight_x_end = highlight_x_start + ImGui::CalcTextSize(line.substr(match_pos - char_offset, query_len).c_str()).x;
                                draw_list->AddRectFilled(ImVec2(highlight_x_start, p.y), ImVec2(highlight_x_end, p.y + line_height_nodraw), IM_COL32(100, 100, 0, 100));
                            }
                        }
                    }

                    size_t current_pos = 0;
                    while (current_pos < line.length()) {
                        size_t start_token = current_pos;
                        size_t end_token = current_pos;
                        ImVec4 current_color = syntaxColors.default_text;

                        auto get_next_token = [&]() {
                            while (end_token < line.length() && !isspace(line[end_token]) && !ispunct(line[end_token])) {
                                end_token++;
                            }
                            if (end_token == start_token) {
                                end_token++;
                            }
                        };

                        if (in_multiline_comment_states[n]) {
                            size_t end_comment = line.find("*/", start_token);
                            if (end_comment == std::string::npos) {
                                end_token = line.length();
                            }
                            else {
                                end_token = end_comment + 2;
                                in_multiline_comment_states[n] = false;
                            }
                            current_color = syntaxColors.comment;
                        }
                        else {
                            char c = line[start_token];
                            if (c == '/' && start_token + 1 < line.length() && line[start_token + 1] == '/') {
                                end_token = line.length();
                                current_color = syntaxColors.comment;
                            }
                            else if (c == '/' && start_token + 1 < line.length() && line[start_token + 1] == '*') {
                                size_t end_comment = line.find("*/", start_token + 2);
                                if (end_comment == std::string::npos) {
                                    end_token = line.length();
                                    in_multiline_comment_states[n] = true;
                                }
                                else {
                                    end_token = end_comment + 2;
                                }
                                current_color = syntaxColors.comment;
                            }
                            else if (c == '#') {
                                end_token = line.length();
                                current_color = syntaxColors.preprocessor;
                            }
                            else if (c == '"' || c == '\'') {
                                end_token = start_token + 1;
                                while (end_token < line.length()) {
                                    if (line[end_token] == c) {
                                        end_token++;
                                        break;
                                    }
                                    if (line[end_token] == '\\' && end_token + 1 < line.length()) {
                                        end_token++;
                                    }
                                    end_token++;
                                }
                                current_color = syntaxColors.string_literal;
                            }
                            else if (isdigit(c)) {
                                end_token = start_token;
                                while (end_token < line.length() && (isdigit(line[end_token]) || line[end_token] == '.')) {
                                    end_token++;
                                }
                                current_color = syntaxColors.number_literal;
                            }
                            else if (isalpha(c) || c == '_') {
                                end_token = start_token;
                                while (end_token < line.length() && (isalnum(line[end_token]) || line[end_token] == '_')) {
                                    end_token++;
                                }
                                std::string token = line.substr(start_token, end_token - start_token);
                                if (cppKeywords.count(token)) {
                                    current_color = syntaxColors.keyword;
                                }
                            }
                            else {
                                end_token = start_token + 1;
                            }
                        }

                        std::string token_str = line.substr(start_token, end_token - start_token);
                        ImGui::PushStyleColor(ImGuiCol_Text, current_color);
                        ImGui::TextUnformatted(token_str.c_str());
                        ImGui::PopStyleColor();

                        if (end_token < line.length()) {
                            ImGui::SameLine(0, 0);
                        }
                        current_pos = end_token;
                    }

                    char_offset += line.length() + 1;
                }

                if (g_pCodeFont) ImGui::PopFont();
                ImGui::EndChild();

                ShowCodeEditorAddons(current_doc, line_count);

                ImGui::EndTabItem();
            }
        }

        if (doc_to_close_idx != -1) {
            docs.erase(docs.begin() + doc_to_close_idx);
            in_multiline_comment_states.erase(in_multiline_comment_states.begin() + doc_to_close_idx);
            if (active_doc_idx >= doc_to_close_idx) {
                active_doc_idx = std::max(0, (int)docs.size() - 1);
            }
        }
        ImGui::EndTabBar();
    }
    else if (docs.empty()) {
        ImGui::TextDisabled("Open a file or drag one into the window.");
    }

    ImGui::End();
}
