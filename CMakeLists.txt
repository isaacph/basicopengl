cmake_minimum_required(VERSION 3.4.3)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)
set(OpenGL_GL_PREFERENCE GLVND)

project(app)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)
find_package(box2d REQUIRED)

# file(CREATE_LINK ${PROJECT_SOURCE_DIR}/res ${CMAKE_BINARY_DIR}/res SYMBOLIC)

file(GLOB_RECURSE appsourcefiles
     "src/*.h"
     "src/*.cpp"
)

add_executable(myapp ${appsourcefiles} src/glad/glad.c)

target_include_directories(myapp PUBLIC include)
target_link_libraries(myapp glfw ${OpenGL_gl_LIBRARY} freetype gcc m dl box2d)
if (UNIX)
    target_link_libraries(myapp dl)
endif (UNIX)

install(TARGETS myapp DESTINATION bin)
#target_link_options(myapp PRIVATE "-static")
