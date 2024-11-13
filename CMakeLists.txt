cmake_minimum_required(VERSION 3.28)
project(WaylandExample)

include(FetchContent)
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.5)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(PkgConfig REQUIRED)
find_package(Vulkan REQUIRED COMPONENTS glslc)
find_package(Wayland REQUIRED COMPONENTS Client Cursor)
find_package(WaylandProtocols REQUIRED)
find_package(WaylandScanner REQUIRED)
pkg_check_modules(XKB REQUIRED IMPORTED_TARGET xkbcommon)
FetchContent_MakeAvailable(ImGui)

set(GLSLC_ARGS "--target-env=vulkan1.3" CACHE STRING "Additional arguments for glslc")

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    list(APPEND GLSLC_ARGS "-g")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    list(APPEND GLSLC_ARGS "-O")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    list(APPEND GLSLC_ARGS "-O" "-g")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel")
    list(APPEND GLSLC_ARGS "-Os")
endif()

function(add_shader_target target)
    set(outdir ${CMAKE_CURRENT_BINARY_DIR}/shaders)
    file(MAKE_DIRECTORY ${outdir})

    set(${all_binaries} "")
    foreach(shader ${ARGN})
        set(source ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${shader})
        set(binary ${outdir}/${shader}.spv)
        add_custom_command(
            OUTPUT ${binary}
            MAIN_DEPENDENCY ${source}
            COMMAND Vulkan::glslc ${GLSLC_ARGS} ${source} -o ${binary}
            VERBATIM)
        list(APPEND all_binaries ${binary})
    endforeach()
    add_custom_target(${target} DEPENDS ${all_binaries})
endfunction()

set(ImGuiSources ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_demo.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp ${imgui_SOURCE_DIR}/imgui_tables.cpp ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp)

set(WaylandProtocolSources "")
ecm_add_wayland_client_protocol(WaylandProtocolSources PROTOCOL ${WaylandProtocols_DATADIR}/stable/xdg-shell/xdg-shell.xml BASENAME xdg-shell)
ecm_add_wayland_client_protocol(WaylandProtocolSources PROTOCOL ${WaylandProtocols_DATADIR}/stable/viewporter/viewporter.xml BASENAME viewporter)
ecm_add_wayland_client_protocol(WaylandProtocolSources PROTOCOL ${WaylandProtocols_DATADIR}/staging/content-type/content-type-v1.xml BASENAME content-type)
ecm_add_wayland_client_protocol(WaylandProtocolSources PROTOCOL ${WaylandProtocols_DATADIR}/staging/cursor-shape/cursor-shape-v1.xml BASENAME cursor-shape)
ecm_add_wayland_client_protocol(WaylandProtocolSources PROTOCOL ${WaylandProtocols_DATADIR}/staging/fractional-scale/fractional-scale-v1.xml BASENAME fractional-scale)
ecm_add_wayland_client_protocol(WaylandProtocolSources PROTOCOL ${WaylandProtocols_DATADIR}/unstable/tablet/tablet-unstable-v2.xml BASENAME tablet) # dependency of cursor-shape
ecm_add_wayland_client_protocol(WaylandProtocolSources PROTOCOL ${WaylandProtocols_DATADIR}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml BASENAME xdg-decoration)

add_executable(wayland_example main.cpp MainWindow.cpp MappedFd.cpp vk_mem_alloc.cpp volk.c
    vulkan/Common.cpp vulkan/Renderer.cpp vulkan/RendererBase.cpp vulkan/Swapchain.cpp vulkan/SwapchainBase.cpp
    wayland/Display.cpp wayland/Keyboard.cpp wayland/Pointer.cpp wayland/Seat.cpp wayland/Window.cpp
    wayland/cursor/CursorBase.cpp wayland/cursor/CursorManagerBase.cpp
    wayland/cursor/shape/ShapeCursor.cpp wayland/cursor/shape/ShapeCursorManager.cpp
    wayland/cursor/theme/ThemeCursor.cpp wayland/cursor/theme/ThemeCursorManager.cpp
    ${ImGuiSources} ${WaylandProtocolSources}
)
set_target_properties(wayland_example PROPERTIES CXX_STANDARD 23)
target_compile_definitions(wayland_example PRIVATE GLM_FORCE_LEFT_HANDED VK_NO_PROTOTYPES VK_USE_PLATFORM_WAYLAND_KHR)
target_include_directories(wayland_example PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${imgui_SOURCE_DIR})
target_link_libraries(wayland_example PkgConfig::XKB Wayland::Client Wayland::Cursor)

add_shader_target(all_shaders main.frag main.vert)
add_dependencies(wayland_example all_shaders)
