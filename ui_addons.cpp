#include "ui_addons.h"
#include "code_editor.h"
#include "file_utils.h"
#include "imgui.h"
#include <algorithm>
#include <cctype>

std::vector<std::string> g_dropped_files_queue;

void PerformSearch(CodeDocument& doc) {
    doc.searchState.matchPositions.clear();
    doc.searchState.currentMatch = -1;
    if (strlen(doc.searchState.query) == 0) {
        return;
    }

    const std::string& content = doc.processedContent;
    std::string query = doc.searchState.query;

    std::string content_to_search = content;
    if (!doc.searchState.caseSensitive) {
        std::transform(content_to_search.begin(), content_to_search.end(), content_to_search.begin(),
            [](unsigned char c) { return std::tolower(c); });
        std::transform(query.begin(), query.end(), query.begin(),
            [](unsigned char c) { return std::tolower(c); });
    }

    size_t start_pos = 0;
    while ((start_pos = content_to_search.find(query, start_pos)) != std::string::npos) {
        doc.searchState.matchPositions.push_back(start_pos);
        start_pos += query.length();
    }
}

void ShowCodeEditorAddons(CodeDocument& doc, int line_count) {
    ImGuiIO& io = ImGui::GetIO();
    float line_height = ImGui::GetTextLineHeightWithSpacing();

    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_F, false)) {
        doc.searchState.active = !doc.searchState.active;
        if (doc.searchState.active) {
            ImGui::SetKeyboardFocusHere();
        }
    }

    if (doc.searchState.active) {
        ImGui::BeginChild("SearchPanel", ImVec2(0, ImGui::GetFrameHeightWithSpacing() * 1.5f), true, ImGuiWindowFlags_NoScrollbar);
        ImGui::PushItemWidth(250.0f);
        if (ImGui::InputTextWithHint("##Search", "Search (Ctrl+F)", doc.searchState.query, sizeof(doc.searchState.query), ImGuiInputTextFlags_EnterReturnsTrue)) {
            PerformSearch(doc);
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Checkbox("Case Sensitive", &doc.searchState.caseSensitive)) {
            PerformSearch(doc);
        }

        if (!doc.searchState.matchPositions.empty()) {
            ImGui::SameLine();
            ImGui::Text("%d / %d", doc.searchState.currentMatch + 1, (int)doc.searchState.matchPositions.size());
            ImGui::SameLine();
            if (ImGui::ArrowButton("##PrevMatch", ImGuiDir_Up)) {
                doc.searchState.currentMatch = (doc.searchState.currentMatch - 1 + doc.searchState.matchPositions.size()) % doc.searchState.matchPositions.size();
                doc.searchState.scrollToMatch = true;
            }
            ImGui::SameLine();
            if (ImGui::ArrowButton("##NextMatch", ImGuiDir_Down)) {
                doc.searchState.currentMatch = (doc.searchState.currentMatch + 1) % doc.searchState.matchPositions.size();
                doc.searchState.scrollToMatch = true;
            }
        }
        ImGui::EndChild();
    }

    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_G, false)) {
        ImGui::OpenPopup("Go to Line");
    }
    if (ImGui::BeginPopupModal("Go to Line", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        static int line_to_go = 1;
        ImGui::Text("Enter line number (1-%d):", line_count);
        ImGui::PushItemWidth(150);
        if (ImGui::InputInt("##linenum", &line_to_go, 1, 10, ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::Button("Go", ImVec2(120, 0))) {
            doc.searchState.lineToScrollTo = std::max(1, std::min(line_to_go, line_count));
            doc.searchState.scrollToMatch = true; 
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::Separator();
    float scroll_y = ImGui::GetScrollY();
    int current_line = static_cast<int>(scroll_y / line_height) + 1;
    const char* lang_str = "Plain Text";
    switch (doc.language) {
    case 0: lang_str = "C++"; break;
    case 1: lang_str = "Python"; break;
    case 2: lang_str = "HTML"; break;
    case 3: lang_str = "CSS"; break;
    case 4: lang_str = "JavaScript"; break;
    }
    ImGui::Text("Line %d / %d", current_line, line_count);
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 150);
    ImGui::Text("Language: %s", lang_str);
}

void HandleDroppedFiles(std::vector<CodeDocument>& docs, int& active_doc_idx) {
    if (g_dropped_files_queue.empty()) {
        return;
    }

    for (const auto& path : g_dropped_files_queue) {
        std::string content_str;
        if (load_file_str(path.c_str(), content_str)) {
            std::string name_str = path.substr(path.find_last_of("/\\") + 1);

            bool already_open = false;
            for (int i = 0; i < docs.size(); ++i) {
                if (docs[i].filePath == path) {
                    active_doc_idx = i;
                    already_open = true;
                    break;
                }
            }

            if (!already_open) {
                docs.emplace_back(path, name_str, content_str);
                CodeDocument& new_doc = docs.back();
                new_doc.language = detect_lang(name_str);
                process_code(new_doc);
                active_doc_idx = docs.size() - 1;
            }
        }
    }
    g_dropped_files_queue.clear();
}
