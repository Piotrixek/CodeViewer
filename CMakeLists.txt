cmake_minimum_required(VERSION 3.15)


project(CodeViewer LANGUAGES CXX C)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


set(IMGUI_DIR "C:/libs/imgui-docking" CACHE PATH "Path to ImGui source directory (docking version)")
set(TINYFILEDIALOGS_DIR "C:/libs/tinyfiledialogs" CACHE PATH "Path to tinyfiledialogs source directory")

if(NOT EXISTS "${IMGUI_DIR}/imgui.h")
    message(FATAL_ERROR "ImGui directory not found or invalid: ${IMGUI_DIR}. Please set IMGUI_DIR correctly.")
endif()
if(NOT EXISTS "${TINYFILEDIALOGS_DIR}/tinyfiledialogs.c")
    message(FATAL_ERROR "tinyfiledialogs directory not found or invalid: ${TINYFILEDIALOGS_DIR}. Please set TINYFILEDIALOGS_DIR correctly.")
endif()

if(NOT EXISTS "${IMGUI_DIR}/imconfig.h")
    message(WARNING "imconfig.h not found in ${IMGUI_DIR}.")
else()
    message(STATUS "Found imconfig.h. Ensure IMGUI_HAS_DOCK/IMGUI_HAS_VIEWPORT are enabled inside!")
endif()


set(PROJECT_SOURCES
    main.cpp
    dx_setup.cpp
    ui_style.cpp
    code_editor.cpp
    window_setup.cpp
    code_capture.cpp
)

set(IMGUI_BACKEND_SOURCES
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
)


set(IMGUI_CORE_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
)


add_library(imgui_lib STATIC
    ${IMGUI_CORE_SOURCES}
    ${IMGUI_BACKEND_SOURCES}
)

target_include_directories(imgui_lib PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

set_target_properties(imgui_lib PROPERTIES CXX_STANDARD ${CMAKE_CXX_STANDARD})


add_library(tinyfd_lib STATIC
    "${TINYFILEDIALOGS_DIR}/tinyfiledialogs.c"
)
target_include_directories(tinyfd_lib PUBLIC
    ${TINYFILEDIALOGS_DIR}
)


add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES})


target_link_libraries(${PROJECT_NAME} PRIVATE
    imgui_lib
    tinyfd_lib
    # DirectX 11 libraries
    d3d11
    d3dcompiler
    dxgi
    # System libraries needed for tinyfiledialogs on Windows
    comdlg32 # Common Dialog Box Library
    ole32    # OLE library
    shell32  # Shell API library (needed for some dialogs)
)


target_include_directories(${PROJECT_NAME} PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${TINYFILEDIALOGS_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)


if(MSVC)
    # For Microsoft Visual C++ compiler
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # For MinGW GCC or Clang on Windows
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-mwindows")
endif()

message(STATUS "Project Name: ${PROJECT_NAME}")
message(STATUS "ImGui Source Directory: ${IMGUI_DIR}")
message(STATUS "TinyFD Source Directory: ${TINYFILEDIALOGS_DIR}")
message(STATUS "Project Sources: ${PROJECT_SOURCES}")
message(STATUS "Configured ImGui with Win32 and DirectX 11 backends.")
message(STATUS "Docking/Viewport features should be enabled via imconfig.h.")
