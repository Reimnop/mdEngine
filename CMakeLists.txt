cmake_minimum_required(VERSION 4.3)
project(mdEngine)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp" "src/*.c" "src/*.h" "src/*.hpp")
file(GLOB_RECURSE EXT_SOURCE_FILES "ext/*.cpp" "ext/*.c")

add_executable(mdEngine ${SOURCE_FILES} ${EXT_SOURCE_FILES})

add_subdirectory(glfw)
target_link_libraries(mdEngine PRIVATE glfw)

target_include_directories(mdEngine PRIVATE "src" "include")

add_custom_command(
    TARGET mdEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:mdEngine>/assets")

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()
