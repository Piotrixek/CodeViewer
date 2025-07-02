#define NOMINMAX
#include <Windows.h> 
#include <tchar.h>   
#include <vector>    
#include <string>    

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "dx_setup.h"
#include "ui_style.h"
#include "code_editor.h"   
#include "window_setup.h"

ImFont* g_pCodeFont = nullptr;

int main(int, char**)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    const TCHAR* className = _T("ImGuiCodeViewerClass"); 
    HWND hwnd = SetupWindow(hInstance, className); 
    if (!hwnd) {
        return 1; 
    }

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D(); 
        ::DestroyWindow(hwnd); 
        CleanupWindow(hInstance, className); 
        return 1; 
    }

    IMGUI_CHECKVERSION(); 
    ImGui::CreateContext(); 
    ImGuiIO& io = ImGui::GetIO(); (void)io; 
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   

    io.Fonts->AddFontDefault();

    const char* firaCodePath = "C:/libs/fonts/Fira_Code_v6.2/ttf/FiraCode-Bold.ttf";
    float fontSize = 14.0f; 


    ImFontConfig fontConfig;
    fontConfig.OversampleH = 2; 
    fontConfig.OversampleV = 1;

    g_pCodeFont = io.Fonts->AddFontFromFileTTF(firaCodePath, fontSize, &fontConfig, io.Fonts->GetGlyphRangesDefault());

    if (!g_pCodeFont) {
        MessageBox(hwnd, _T("Failed to load Fira Code font. Using default ImGui font."), _T("Font Warning"), MB_OK | MB_ICONWARNING);
        g_pCodeFont = io.Fonts->Fonts[0]; 
    }
    io.FontDefault = g_pCodeFont;


    ApplyCodeViewerStyle();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f; 
        style.Colors[ImGuiCol_WindowBg].w = 1.0f; 
    }

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(GetDevice(), GetImmediateContext()); 


    std::vector<CodeDocument> openDocuments; 
    int activeDocumentIndex = -1;            
    bool showApp = true;                     


    while (showApp)
    {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg); 
            ::DispatchMessage(&msg); 
            if (msg.message == WM_QUIT) 
                showApp = false; 
        }
        if (!showApp) 
            break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();

        ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        if (main_viewport) {
            io.DisplaySize = main_viewport->Size;
        }

        ImGui::NewFrame();

        ShowCodeViewerUI(&showApp, openDocuments, activeDocumentIndex);


        ImGui::Render(); 

        const float clear_color_with_alpha[4] = { 0.1f, 0.1f, 0.1f, 1.00f }; 
        ID3D11RenderTargetView* mainRenderTargetView = GetMainRenderTargetView(); 
        ID3D11DeviceContext* context = GetImmediateContext(); 

        if (mainRenderTargetView && context) {
            context->OMSetRenderTargets(1, &mainRenderTargetView, NULL); 
            context->ClearRenderTargetView(mainRenderTargetView, clear_color_with_alpha); 
        }

        if (ImGui::GetDrawData()) { 
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        IDXGISwapChain* swapChain = GetSwapChain(); 
        if (swapChain) {
            swapChain->Present(1, 0); 
        }
    } 

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    CleanupWindow(hInstance, className);

    return 0; 
}


