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

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
foreach(IMPORTER OBJ FBX GLTF COLLADA 3DS STL PLY)
    set(ASSIMP_BUILD_${IMPORTER}_IMPORTER ON CACHE BOOL "" FORCE)
endforeach()
add_subdirectory(assimp)
target_link_libraries(mdEngine PRIVATE assimp::assimp)

target_include_directories(mdEngine PRIVATE "src" "include" "${IMGUI_DIR}" "${IMGUI_DIR}/backends")

add_custom_command(
    TARGET mdEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:mdEngine>/assets")

if(WIN32)
    target_link_options(mdEngine PRIVATE -static)
endif()
