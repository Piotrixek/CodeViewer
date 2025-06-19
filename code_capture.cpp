#include "code_capture.h"
#include "dx_setup.h" 
#include "imgui.h"
#include "imgui_internal.h" 
#include "tinyfiledialogs.h"
#include <vector>
#include <string>
#include <sstream>
#include <algorithm> 
#include <cmath>     
#include <comdef.h>  



#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" 
#include <imgui_impl_dx11.h>


const int PADDING = 10; 
const int MAX_TEXTURE_DIM = 8192; 



bool capture_code_to_image(const CodeDocument& doc, const SyntaxColors& colors, ImFont* font) {
    if (!font) {
        tinyfd_messageBox("Capture Error", "Code font not available.", "ok", "error", 1);
        return false;
    }

    ID3D11Device* device = GetDevice();
    ID3D11DeviceContext* context = GetImmediateContext();
    if (!device || !context) {
        tinyfd_messageBox("Capture Error", "DirectX device not available.", "ok", "error", 1);
        return false;
    }

    
    const std::string& content_to_render = doc.processedContent;

    
    int img_width = 0;
    int img_height = 0;
    float line_height = font->FontSize + ImGui::GetStyle().ItemSpacing.y; 

    
    int line_count = 0;
    for (char c : content_to_render) { if (c == '\n') line_count++; }
    if (!content_to_render.empty() && content_to_render.back() != '\n') line_count++;
    line_count = std::max(1, line_count);
    char line_no_fmt[16];
    int max_digits = (line_count == 0) ? 1 : ((int)log10(line_count) + 1);
    sprintf_s(line_no_fmt, sizeof(line_no_fmt), "%%-%dd | ", max_digits); 
    char max_line_no_str[16]; sprintf_s(max_line_no_str, sizeof(max_line_no_str), "%d | ", line_count);
    int line_num_width = (int)ImGui::CalcTextSize(max_line_no_str, NULL, false, -1.0f).x; 

    calculate_image_size(content_to_render, font, line_height, img_width, img_height, line_num_width);

    
    img_width += PADDING * 2;
    img_height += PADDING * 2;

    
    img_width = std::min(img_width, MAX_TEXTURE_DIM);
    img_height = std::min(img_height, MAX_TEXTURE_DIM);

    if (img_width <= PADDING * 2 || img_height <= PADDING * 2) {
        tinyfd_messageBox("Capture Error", "Calculated image size is invalid.", "ok", "error", 1);
        return false;
    }

    
    ID3D11Texture2D* render_texture = nullptr;
    ID3D11RenderTargetView* render_texture_rtv = nullptr;

    D3D11_TEXTURE2D_DESC tex_desc = {};
    tex_desc.Width = img_width;
    tex_desc.Height = img_height;
    tex_desc.MipLevels = 1;
    tex_desc.ArraySize = 1;
    tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; 
    tex_desc.SampleDesc.Count = 1;
    tex_desc.Usage = D3D11_USAGE_DEFAULT; 
    tex_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; 
    tex_desc.CPUAccessFlags = 0;
    tex_desc.MiscFlags = 0;

    HRESULT hr = device->CreateTexture2D(&tex_desc, nullptr, &render_texture);
    if (FAILED(hr)) {
        _com_error err(hr);
        tinyfd_messageBox("Capture Error", ("Failed to create render texture: " + std::string(err.ErrorMessage())).c_str(), "ok", "error", 1);
        return false;
    }

    
    D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
    rtv_desc.Format = tex_desc.Format;
    rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtv_desc.Texture2D.MipSlice = 0;

    hr = device->CreateRenderTargetView(render_texture, &rtv_desc, &render_texture_rtv);
    if (FAILED(hr)) {
        _com_error err(hr);
        tinyfd_messageBox("Capture Error", ("Failed to create render target view: " + std::string(err.ErrorMessage())).c_str(), "ok", "error", 1);
        if (render_texture) render_texture->Release();
        return false;
    }

    

    
    
    ImDrawList* draw_list = ImGui::GetForegroundDrawList(); 
    
    

    
    
    

    
    ID3D11RenderTargetView* old_rtv = nullptr;
    ID3D11DepthStencilView* old_dsv = nullptr;
    context->OMGetRenderTargets(1, &old_rtv, &old_dsv);

    
    context->OMSetRenderTargets(1, &render_texture_rtv, nullptr); 

    
    D3D11_VIEWPORT vp = {};
    vp.Width = (float)img_width;
    vp.Height = (float)img_height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    context->RSSetViewports(1, &vp);

    
    float clear_color[4] = { 0.11f, 0.12f, 0.13f, 1.00f }; 
    context->ClearRenderTargetView(render_texture_rtv, clear_color);

    
    
    ImDrawList* offscreen_draw_list = IM_NEW(ImDrawList)(ImGui::GetDrawListSharedData());
    offscreen_draw_list->_ResetForNewFrame(); 
    offscreen_draw_list->PushTextureID(io.Fonts->TexID); 
    offscreen_draw_list->PushClipRect(ImVec2(0, 0), ImVec2((float)img_width, (float)img_height), false); 

    render_code_to_drawlist(offscreen_draw_list, doc, colors, font, line_height, line_num_width, ImVec2((float)PADDING, (float)PADDING));

    offscreen_draw_list->PopClipRect();
    offscreen_draw_list->PopTextureID();

    
    ImDrawData temp_draw_data;
    temp_draw_data.Valid = true;
    temp_draw_data.CmdLists = &offscreen_draw_list;
    temp_draw_data.CmdListsCount = 1;
    temp_draw_data.TotalIdxCount = offscreen_draw_list->IdxBuffer.Size;
    temp_draw_data.TotalVtxCount = offscreen_draw_list->VtxBuffer.Size;
    temp_draw_data.DisplayPos = ImVec2(0.0f, 0.0f);
    temp_draw_data.DisplaySize = ImVec2((float)img_width, (float)img_height);
    temp_draw_data.FramebufferScale = ImVec2(1.0f, 1.0f); 

    ImGui_ImplDX11_RenderDrawData(&temp_draw_data); 

    
    IM_DELETE(offscreen_draw_list);


    
    context->OMSetRenderTargets(1, &old_rtv, old_dsv);
    if (old_rtv) old_rtv->Release();
    if (old_dsv) old_dsv->Release();
    

    
    std::vector<unsigned char> pixels(img_width * img_height * 4); 
    bool copy_ok = get_texture_pixels(render_texture, img_width, img_height, pixels);

    
    if (render_texture_rtv) render_texture_rtv->Release();
    if (render_texture) render_texture->Release();

    if (!copy_ok) {
        
        return false;
    }

    
    const char* filters[] = { "*.png" }; 
    std::string default_name = doc.fileName;
    
    size_t dot_pos = default_name.find_last_of('.');
    if (dot_pos != std::string::npos) {
        default_name = default_name.substr(0, dot_pos);
    }
    default_name += ".png";

    const char* save_path = tinyfd_saveFileDialog(
        "Save Code Image As...",
        default_name.c_str(),
        1, 
        filters,
        "PNG Image"
    );

    if (!save_path) {
        
        return false;
    }

    
    int success = stbi_write_png(
        save_path,
        img_width,
        img_height,
        4, 
        pixels.data(),
        img_width * 4 
    );

    if (!success) {
        tinyfd_messageBox("Save Error", "Failed to write PNG image file.", "ok", "error", 1);
        return false;
    }

    tinyfd_messageBox("Success", ("Code image saved to:\n" + std::string(save_path)).c_str(), "ok", "info", 1);
    return true;
}




void calculate_image_size(const std::string& text, ImFont* font, float line_height, int& width_out, int& height_out, int line_num_width_pixels) {
    width_out = 0;
    height_out = 0;
    if (!font) return;

    std::stringstream ss(text);
    std::string line;
    int line_count = 0;
    float max_line_width = 0.0f;

    while (std::getline(ss, line)) {
        ImVec2 line_size = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, line.c_str(), line.c_str() + line.length(), nullptr);
        max_line_width = std::max(max_line_width, line_size.x);
        line_count++;
    }
    if (text.empty() || line_count == 0) line_count = 1; 

    width_out = line_num_width_pixels + (int)std::ceil(max_line_width);
    height_out = (int)std::ceil((float)line_count * line_height);
}



void render_code_to_drawlist(
    ImDrawList* draw_list,
    const CodeDocument& doc,
    const SyntaxColors& colors,
    ImFont* font,
    float line_height,
    int line_num_width_pixels,
    const ImVec2& offset)
{
    if (!font || !draw_list) return;

    const std::string& content = doc.processedContent; 
    int lang = doc.language;

    std::stringstream ss(content);
    std::string line;
    int current_line_num = 1;
    float current_y = offset.y;

    
    const std::unordered_set<std::string>* keywords = nullptr;
    if (lang == 0) keywords = &cppKeywords;
    else if (lang == 1) keywords = &pythonKeywords;
    else if (lang == 4) keywords = &jsKeywords;
    

    
    char line_no_fmt[16];
    int max_digits = (current_line_num == 0) ? 1 : ((int)log10(std::max(1, current_line_num)) + 1); 
    sprintf_s(line_no_fmt, sizeof(line_no_fmt), "%%-%dd | ", max_digits);

    ImU32 col_default = ImGui::ColorConvertFloat4ToU32(colors.default_text);
    ImU32 col_keyword = ImGui::ColorConvertFloat4ToU32(colors.keyword);
    ImU32 col_comment = ImGui::ColorConvertFloat4ToU32(colors.comment);
    ImU32 col_string = ImGui::ColorConvertFloat4ToU32(colors.string_literal);
    ImU32 col_number = ImGui::ColorConvertFloat4ToU32(colors.number_literal);
    ImU32 col_preproc = ImGui::ColorConvertFloat4ToU32(colors.preprocessor);
    ImU32 col_linenum = ImGui::ColorConvertFloat4ToU32(ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); 

    
    bool in_multiline_comment = false;

    while (std::getline(ss, line)) {
        float current_x = offset.x;

        
        char line_num_str[16];
        sprintf_s(line_num_str, sizeof(line_num_str), line_no_fmt, current_line_num);
        draw_list->AddText(font, font->FontSize, ImVec2(current_x, current_y), col_linenum, line_num_str);
        current_x += (float)line_num_width_pixels;

        
        size_t current_pos = 0;
        while (current_pos < line.length()) {

            
            if (in_multiline_comment) {
                size_t end_comment = line.find("*/", current_pos);
                size_t render_until = line.length();
                if (end_comment != std::string::npos) {
                    render_until = end_comment + 2;
                    in_multiline_comment = false; 
                }
                
                draw_list->AddText(font, font->FontSize, ImVec2(current_x, current_y), col_comment,
                    line.c_str() + current_pos, line.c_str() + render_until);
                
                ImVec2 text_size = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, line.c_str() + current_pos, line.c_str() + render_until);
                current_x += text_size.x;
                current_pos = render_until;
                continue; 
            }

            
            size_t start_token = current_pos;
            while (start_token < line.length() && isspace(line[start_token])) {
                start_token++;
            }
            if (start_token > current_pos) {
                
                draw_list->AddText(font, font->FontSize, ImVec2(current_x, current_y), col_default,
                    line.c_str() + current_pos, line.c_str() + start_token);
                ImVec2 text_size = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, line.c_str() + current_pos, line.c_str() + start_token);
                current_x += text_size.x;
                current_pos = start_token;
            }

            if (current_pos >= line.length()) break; 

            
            start_token = current_pos;
            size_t end_token = current_pos;
            ImU32 current_color = col_default;
            bool token_found = false;

            
            size_t single_comment_pos = std::string::npos;
            size_t multi_comment_pos = std::string::npos;
            if (lang == 0 || lang == 4) { 
                single_comment_pos = line.find("//", current_pos);
                multi_comment_pos = line.find("/*", current_pos);
            }
            else if (lang == 1) { 
                single_comment_pos = line.find("#", current_pos);
            }
            else if (lang == 3) { 
                multi_comment_pos = line.find("/*", current_pos);
            }
            size_t first_comment_pos = std::min(single_comment_pos, multi_comment_pos);

            
            if (first_comment_pos == current_pos) {
                token_found = true;
                current_color = col_comment;
                if (first_comment_pos == single_comment_pos) { 
                    end_token = line.length();
                }
                else { 
                    end_token = current_pos + 2;
                    in_multiline_comment = true; 
                    
                    size_t end_comment_here = line.find("*/", end_token);
                    if (end_comment_here != std::string::npos) {
                        end_token = end_comment_here + 2;
                        in_multiline_comment = false; 
                    }
                    else {
                        end_token = line.length(); 
                    }
                }
            }
            else {
                
                char c = line[current_pos];
                if (lang == 0 && c == '#') { 
                    token_found = true; current_color = col_preproc; end_token = line.length();
                }
                else if (c == '"' || c == '\'' || (lang == 4 && c == '`')) { 
                    token_found = true; current_color = col_string;
                    char quote = c; end_token = current_pos + 1;
                    while (end_token < line.length()) {
                        if (line[end_token] == '\\' && end_token + 1 < line.length()) { end_token += 2; continue; }
                        if (line[end_token] == quote) { end_token++; break; }
                        end_token++;
                    }
                }
                else if (isalpha(c) || c == '_') { 
                    token_found = true; end_token = current_pos;
                    while (end_token < line.length() && (isalnum(line[end_token]) || line[end_token] == '_')) { end_token++; }
                    std::string token = line.substr(start_token, end_token - start_token);
                    if (keywords && keywords->count(token)) {
                        current_color = col_keyword;
                    }
                    else {
                        current_color = col_default;
                    }
                }
                else if (isdigit(c) || (c == '.' && current_pos + 1 < line.length() && isdigit(line[current_pos + 1]))) { 
                    token_found = true; current_color = col_number; end_token = current_pos;
                    while (end_token < line.length() && (isdigit(line[end_token]) || line[end_token] == '.' || tolower(line[end_token]) == 'f')) { end_token++; }
                }
                else { 
                    token_found = true; end_token = current_pos + 1; current_color = col_default;
                }
            }

            
            draw_list->AddText(font, font->FontSize, ImVec2(current_x, current_y), current_color,
                line.c_str() + start_token, line.c_str() + end_token);
            
            ImVec2 text_size = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, line.c_str() + start_token, line.c_str() + end_token);
            current_x += text_size.x;
            current_pos = end_token;

        } 

        current_y += line_height; 
        current_line_num++;
    } 
}



bool get_texture_pixels(ID3D11Texture2D* texture, UINT width, UINT height, std::vector<unsigned char>& pixels_out) {
    ID3D11Device* device = GetDevice();
    ID3D11DeviceContext* context = GetImmediateContext();
    if (!device || !context || !texture) return false;

    
    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc); 
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.MiscFlags = 0;

    ID3D11Texture2D* staging_texture = nullptr;
    HRESULT hr = device->CreateTexture2D(&desc, nullptr, &staging_texture);
    if (FAILED(hr)) {
        _com_error err(hr);
        tinyfd_messageBox("Capture Error", ("Failed to create staging texture: " + std::string(err.ErrorMessage())).c_str(), "ok", "error", 1);
        return false;
    }

    
    context->CopyResource(staging_texture, texture);

    
    D3D11_MAPPED_SUBRESOURCE mapped_resource;
    hr = context->Map(staging_texture, 0, D3D11_MAP_READ, 0, &mapped_resource);
    if (FAILED(hr)) {
        _com_error err(hr);
        tinyfd_messageBox("Capture Error", ("Failed to map staging texture: " + std::string(err.ErrorMessage())).c_str(), "ok", "error", 1);
        staging_texture->Release();
        return false;
    }

    
    const unsigned char* source_pixels = static_cast<const unsigned char*>(mapped_resource.pData);
    size_t source_row_pitch = mapped_resource.RowPitch;
    size_t target_row_pitch = width * 4; 

    pixels_out.resize(width * height * 4);

    for (UINT y = 0; y < height; ++y) {
        memcpy(pixels_out.data() + y * target_row_pitch, 
            source_pixels + y * source_row_pitch,     
            target_row_pitch);                        
    }

    
    context->Unmap(staging_texture, 0);
    staging_texture->Release();

    return true;
}
