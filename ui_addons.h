#pragma once

#include <vector>
#include <string>

struct ImDrawList;
struct CodeDocument;

extern std::vector<std::string> g_dropped_files_queue;

void HandleDroppedFiles(std::vector<CodeDocument>& docs, int& active_doc_idx);

void ShowCodeEditorAddons(CodeDocument& doc, int line_count);
