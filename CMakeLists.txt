cmake_minimum_required(VERSION 4.1)
project(mdEngine)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp" "src/*.c")
file(GLOB_RECURSE EXT_SOURCE_FILES "ext/*.cpp" "ext/*.c")

set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

file(GLOB IMGUI_SOURCE_FILES "${IMGUI_DIR}/*.cpp")
set(IMGUI_SOURCE_FILES ${IMGUI_SOURCE_FILES}
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp")

add_executable(mdEngine ${SOURCE_FILES} ${EXT_SOURCE_FILES} ${IMGUI_SOURCE_FILES})

add_subdirectory(glfw)
target_link_libraries(mdEngine PRIVATE glfw)

add_subdirectory(json)
target_link_libraries(mdEngine PRIVATE nlohmann_json)

target_include_directories(mdEngine PRIVATE "src" "include" "${IMGUI_DIR}" "${IMGUI_DIR}/backends")

add_custom_command(
    TARGET mdEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:mdEngine>/assets")

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()
