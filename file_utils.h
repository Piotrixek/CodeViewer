#pragma once

#include <string>

struct CodeDocument;

bool load_file_str(const char* path, std::string& content_out);

int detect_lang(const std::string& fname);
std::string strip_comments(const std::string& code, int lang);
void process_code(CodeDocument& doc);
