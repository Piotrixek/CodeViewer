#pragma once

#include <string>
#include <vector>
#include "imgui.h" 


struct CodeDocument {
    std::string filePath;
    std::string fileName;
    std::string content;
    std::string processedContent; 
    bool showProcessed = false;   
    bool showComments = true;    
    int language = 0;             
    bool open = true;             

    
    CodeDocument(std::string path = "", std::string name = "", std::string data = "")
        : filePath(std::move(path)),
        fileName(std::move(name)),
        content(std::move(data)),
        processedContent(content), 
        open(true) 
    {}
};



struct SyntaxColors {
    ImVec4 keyword = ImVec4(0.20f, 0.60f, 0.90f, 1.0f); 
    ImVec4 comment = ImVec4(0.35f, 0.65f, 0.35f, 1.0f); 
    ImVec4 string_literal = ImVec4(0.80f, 0.50f, 0.30f, 1.0f); 
    ImVec4 number_literal = ImVec4(0.70f, 0.70f, 0.40f, 1.0f); 
    ImVec4 preprocessor = ImVec4(0.60f, 0.40f, 0.80f, 1.0f); 
    ImVec4 html_tag = ImVec4(0.90f, 0.30f, 0.30f, 1.0f); 
    ImVec4 html_attribute = ImVec4(0.60f, 0.80f, 0.30f, 1.0f); 
    ImVec4 css_selector = ImVec4(0.80f, 0.30f, 0.80f, 1.0f); 
    ImVec4 css_property = ImVec4(0.30f, 0.50f, 0.95f, 1.0f); 
    ImVec4 default_text = ImVec4(0.90f, 0.91f, 0.92f, 1.0f); 
};


void ShowCodeViewerUI(bool* p_open, std::vector<CodeDocument>& documents, int& activeDocIndex);


bool LoadFileContent(const char* path, std::string& content);


void ProcessCodeContent(CodeDocument& doc);

