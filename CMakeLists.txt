
include_directories("imgui/")

file(GLOB sources 
    "*.cpp",
    "imgui/*.cpp"
    "imgui/misc/cpp/*.cpp",
    "imgui/backends/imgui_impl_glfw.cpp"
    "imgui/backends/imgui_impl_vulkan.cpp"
    )
    

add_executable(Editor ${sources}) 
target_link_libraries(Editor engine-core)
    
    
set_target_properties(Editor PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_custom_target(RunEveryTime ALL
    COMMAND "${CMAKE_CURRENT_LIST_DIR}/Shaders/compile.sh"
    COMMENT "Executing my mandatory post-make task..."
)
