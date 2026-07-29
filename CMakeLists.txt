cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)

project(editor)


include_directories("Sources/")
include_directories("imgui/")
include_directories("CoreEngine/")
include_directories("CoreEngine/Vendor/glm")


file(GLOB sources 
	"Sources/*.cpp",
    "imgui/*.cpp"
    "imgui/misc/cpp/*.cpp",
    "imgui/backends/imgui_impl_glfw.cpp"
    "imgui/backends/imgui_impl_vulkan.cpp"
    )


add_subdirectory(CoreEngine)
add_executable(Editor ${sources}) 
target_link_libraries(Editor engine-core)
    
set_target_properties(Editor PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_custom_target(RunEveryTime ALL
    COMMAND "${CMAKE_CURRENT_LIST_DIR}/Shaders/compile.sh"
    COMMENT "Executing my mandatory post-make task..."
)
