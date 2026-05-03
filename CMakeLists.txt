cmake_minimum_required(VERSION 3.21)
project(HeliosUI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (WIN32)
    set(HELIOS_ALLOWED_BACKENDS "DIRECTX;VULKAN;OPENGL")
elseif (APPLE)
    set(HELIOS_ALLOWED_BACKENDS "METAL;VULKAN;OPENGL")
else()
    set(HELIOS_ALLOWED_BACKENDS "VULKAN;OPENGL")
endif()

option(HELIOS_USE_DIRECTX "Enable DirectX backend" OFF)
option(HELIOS_USE_VULKAN "Enable Vulkan backend" OFF)
option(HELIOS_USE_METAL "Enable Metal backend" OFF)
option(HELIOS_USE_OPENGL "Enable OpenGL backend" ON)

option(HELIOS_PREFER_GLEW "Prefer GLEW instead of GLAD for OpenGL function loading" OFF)
option(HELIOS_ENABLE_DEVTOOLS "Enable development tools (Widget inspector, texture viewer, etc.)" OFF)

add_library(
    ${PROJECT_NAME} STATIC
    src/Actions/Easings.cpp
    src/Actions/Scheduler.cpp
    src/Debug/Log.cpp
    src/Input/InputDispatcher.cpp
    src/Renderer/DrawList.cpp
    src/Renderer/Renderer.cpp
    src/Text/FontManager.cpp
    src/Widgets/Box.cpp
    src/Widgets/Label.cpp
    src/Widgets/LinearLayout.cpp
    src/Widgets/Widget.cpp
    src/Director.cpp
)

function(add_backend backend file)
    if (HELIOS_USE_${backend})
        if (${backend} IN_LIST HELIOS_ALLOWED_BACKENDS)
            message(STATUS "[HeliosUI] Enabling ${backend} backend")
            target_sources(${PROJECT_NAME} PRIVATE src/Backends/${file})
            target_compile_definitions(${PROJECT_NAME} PUBLIC HELIOS_USE_${backend})
        endif()
    endif()
endfunction()

# add_backend(DIRECTX "DirectXBackend.cpp")
# add_backend(VULKAN "VulkanBackend.cpp")
# add_backend(METAL "MetalBackend.cpp")
add_backend(OPENGL "GLBackend.cpp")

target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<BOOL:${HELIOS_PREFER_GLEW}>:HELIOS_PREFER_GLEW>)
if (HELIOS_USE_OPENGL AND NOT HELIOS_PREFER_GLEW)
    add_library(glad STATIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/src/gl.c)
    target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include)
    target_link_libraries(${PROJECT_NAME} PUBLIC glad)
endif()

target_include_directories(
    ${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if (HELIOS_ENABLE_DEVTOOLS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC HELIOS_ENABLE_DEVTOOLS)
    target_sources(${PROJECT_NAME} PRIVATE src/Debug/DevTools.cpp)
endif()

if (NOT TARGET fmt::fmt OR NOT TARGET tint)
    if (NOT COMMAND CPMAddPackage)
        include(cmake/CPM.cmake)
    endif()

    if (NOT TARGET fmt::fmt)
        CPMAddPackage("gh:fmtlib/fmt#e824477")
    endif()

    if (NOT TARGET tint)
        CPMAddPackage("gh:prevter/tint#eea7556")
    endif()

    # get imgui for devtools
    if (HELIOS_ENABLE_DEVTOOLS)
        if (NOT TARGET imgui)
            CPMAddPackage(
                URI "gh:ocornut/imgui@1.92.6-docking"
                DOWNLOAD_ONLY YES
            )

            add_library(
                imgui STATIC
                ${imgui_SOURCE_DIR}/imgui.cpp
                ${imgui_SOURCE_DIR}/imgui_demo.cpp
                ${imgui_SOURCE_DIR}/imgui_draw.cpp
                ${imgui_SOURCE_DIR}/imgui_widgets.cpp
                ${imgui_SOURCE_DIR}/imgui_tables.cpp
                ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
                ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
                ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
            )

            target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)

            find_package(OpenGL REQUIRED)
            target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

            target_link_libraries(${PROJECT_NAME} PUBLIC imgui)
        endif()
    endif()

    target_link_libraries(${PROJECT_NAME} PUBLIC tint::tint fmt::fmt)
endif()

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(Helios::UI ALIAS ${PROJECT_NAME})
