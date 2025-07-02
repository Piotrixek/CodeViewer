#pragma once

#include "code_editor.h" 
#include <string>
#include <vector> 
#include <d3d11.h>

bool capture_code_to_image(const CodeDocument& doc, const SyntaxColors& colors, ImFont* font);


void calculate_image_size(const std::string& text, ImFont* font, float line_spacing, int& width_out, int& height_out, int line_num_width_pixels);

void render_code_to_drawlist(
    ImDrawList* draw_list,
    const CodeDocument& doc,
    const SyntaxColors& colors,
    ImFont* font,
    float line_height,
    int line_num_width_pixels,
    const ImVec2& offset 
);

bool get_texture_pixels(ID3D11Texture2D* texture, UINT width, UINT height, std::vector<unsigned char>& pixels_out);
