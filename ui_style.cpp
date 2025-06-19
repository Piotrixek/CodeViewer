#include "ui_style.h"
#include "imgui.h" 


void ApplyCodeViewerStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    
    ImVec4 bg_main = ImVec4(0.11f, 0.12f, 0.13f, 1.00f); 
    ImVec4 bg_secondary = ImVec4(0.16f, 0.17f, 0.19f, 1.00f); 
    ImVec4 bg_frame = ImVec4(0.08f, 0.08f, 0.09f, 1.00f); 
    ImVec4 accent = ImVec4(0.20f, 0.55f, 0.75f, 1.00f); 
    ImVec4 accent_hover = ImVec4(0.30f, 0.65f, 0.85f, 1.00f);
    ImVec4 accent_active = ImVec4(0.40f, 0.75f, 0.95f, 1.00f);
    ImVec4 text_color = ImVec4(0.90f, 0.91f, 0.92f, 1.00f); 
    ImVec4 text_disabled = ImVec4(0.45f, 0.46f, 0.47f, 1.00f);
    ImVec4 border_color = ImVec4(0.25f, 0.27f, 0.30f, 0.50f);

    colors[ImGuiCol_Text]                   = text_color;
    colors[ImGuiCol_TextDisabled]           = text_disabled;
    colors[ImGuiCol_WindowBg]               = bg_main;
    colors[ImGuiCol_ChildBg]                = bg_secondary; 
    colors[ImGuiCol_PopupBg]                = bg_frame;
    colors[ImGuiCol_Border]                 = border_color;
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = bg_frame;
    colors[ImGuiCol_FrameBgHovered]         = bg_secondary;
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.22f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TitleBg]                = bg_main;
    colors[ImGuiCol_TitleBgActive]          = accent;
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = bg_secondary;
    colors[ImGuiCol_ScrollbarBg]            = bg_frame;
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]              = accent;
    colors[ImGuiCol_SliderGrab]             = accent;
    colors[ImGuiCol_SliderGrabActive]       = accent_active;
    colors[ImGuiCol_Button]                 = accent;
    colors[ImGuiCol_ButtonHovered]          = accent_hover;
    colors[ImGuiCol_ButtonActive]           = accent_active;
    colors[ImGuiCol_Header]                 = accent; 
    colors[ImGuiCol_HeaderHovered]          = accent_hover;
    colors[ImGuiCol_HeaderActive]           = accent_active;
    colors[ImGuiCol_Separator]              = border_color;
    colors[ImGuiCol_SeparatorHovered]       = accent_hover;
    colors[ImGuiCol_SeparatorActive]        = accent_active;
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.3f, 0.3f, 0.3f, 0.4f);
    colors[ImGuiCol_ResizeGripHovered]      = accent_hover;
    colors[ImGuiCol_ResizeGripActive]       = accent_active;
    colors[ImGuiCol_Tab]                    = bg_secondary;
    colors[ImGuiCol_TabHovered]             = accent_hover;
    colors[ImGuiCol_TabActive]              = bg_main; 
    colors[ImGuiCol_TabUnfocused]           = bg_secondary;
    colors[ImGuiCol_TabUnfocusedActive]     = bg_main;
    colors[ImGuiCol_DockingPreview]         = accent_active;
    colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.1f, 0.1f, 0.1f, 1.00f); 
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.16f, 0.17f, 0.19f, 1.00f); 
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.04f);
    colors[ImGuiCol_TextSelectedBg]         = accent;
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = accent_hover;
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.1f, 0.1f, 0.12f, 0.6f); 

    
    style.WindowRounding    = 6.0f;
    style.ChildRounding     = 4.0f;
    style.FrameRounding     = 4.0f;
    style.PopupRounding     = 4.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding      = 4.0f;
    style.TabRounding       = 4.0f;

    
    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 0.0f; 
    style.PopupBorderSize   = 1.0f;
    style.ChildBorderSize   = 1.0f; 
    style.TabBorderSize     = 0.0f;

    
    style.WindowPadding     = ImVec2(8.0f, 8.0f);
    style.FramePadding      = ImVec2(6.0f, 4.0f);
    style.ItemSpacing       = ImVec2(6.0f, 5.0f);
    style.ItemInnerSpacing  = ImVec2(5.0f, 4.0f);
    style.ScrollbarSize     = 14.0f;
    style.GrabMinSize       = 12.0f;

    
    style.WindowTitleAlign  = ImVec2(0.0f, 0.5f); 
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ButtonTextAlign   = ImVec2(0.5f, 0.5f); 
}
