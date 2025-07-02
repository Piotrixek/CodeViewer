#include "file_utils.h"
#include "code_editor.h" 
#include "tinyfiledialogs.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

bool load_file_str(const char* path, std::string& content_out) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        tinyfd_messageBox("Error", "Cannot open file", "ok", "error", 1);
        return false;
    }
    std::streamsize len = file.tellg();
    file.seekg(0, std::ios::beg);

    const long long max_size = 20LL * 1024 * 1024; 
    if (len > max_size || len < 0) {
        tinyfd_messageBox("Error", "File is too large (>20MB) or size is invalid.", "ok", "error", 1);
        file.close();
        return false;
    }

    try {
        content_out.resize(static_cast<size_t>(len));
        if (len > 0) {
            file.read(&content_out[0], len);
        }
    }
    catch (const std::exception& e) {
        tinyfd_messageBox("Error", e.what(), "ok", "error", 1);
        file.close();
        return false;
    }
    file.close();
    return true;
}


int detect_lang(const std::string& fname) {
    size_t dot_pos = fname.find_last_of(".");
    if (dot_pos == std::string::npos) return -1;
    std::string ext = fname.substr(dot_pos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
    if (ext == "cpp" || ext == "h" || ext == "hpp" || ext == "cxx" || ext == "hxx" || ext == "c") return 0;
    if (ext == "py" || ext == "pyw") return 1;
    if (ext == "html" || ext == "htm") return 2;
    if (ext == "css") return 3;
    if (ext == "js") return 4;
    return -1; 
}

std::string strip_comments(const std::string& code, int lang) {
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
                    size_t next_comment_start = std::min(single_comment, multi_comment_start);

                    if (next_comment_start == std::string::npos) {
                        processed_line += line.substr(current_pos);
                        if (line.substr(current_pos).find_first_not_of(" \t\r\n") != std::string::npos) line_had_content = true;
                        break;
                    }

                    processed_line += line.substr(current_pos, next_comment_start - current_pos);
                    if (line.substr(current_pos, next_comment_start - current_pos).find_first_not_of(" \t\r\n") != std::string::npos) line_had_content = true;

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
            if (processed_line.find_first_not_of(" \t\r\n") != std::string::npos) line_had_content = true;
        }
        else {
            processed_line += line;
            if (processed_line.find_first_not_of(" \t\r\n") != std::string::npos) line_had_content = true;
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
        doc.processedContent = doc.content;
    }
    else {
        doc.processedContent = strip_comments(doc.content, doc.language);
    }
}
