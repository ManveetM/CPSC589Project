cmake_minimum_required(VERSION 3.15.0)
project(589-689-3D-skeleton)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Use modern C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(OpenGL_GL_PREFERENCE GLVND)

#-------------------------------------------------------------------------------
# https://github.com/adishavit/argh/releases/tag/v1.3.1
include_directories(SYSTEM thirdparty/argh-1.3.1/)

#-------------------------------------------------------------------------------
# https://glad.dav1d.de/
add_subdirectory(thirdparty/glad)
set(LIBRARIES ${LIBRARIES} glad)

#-------------------------------------------------------------------------------
# https://www.glfw.org/

# Turn off building their docs/tests/examples.
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(thirdparty/glfw-3.3.2)
set(LIBRARIES ${LIBRARIES} glfw)

#-------------------------------------------------------------------------------
# https://github.com/gurki/vivid/releases/tag/v2.2.1
include_directories(SYSTEM thirdparty/vivid-2.2.1/include)
include_directories(SYSTEM thirdparty/vivid-2.2.1/dependencies/glm)

#-------------------------------------------------------------------------------
# https://glm.g-truc.net/0.9.9/index.html
include_directories(SYSTEM thirdparty/glm-0.9.9.7/)

#-------------------------------------------------------------------------------
# https://fmt.dev/latest/index.html
# Turn off building their tests
set(FMT_TES OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/fmt-7.0.3)
set(LIBRARIES ${LIBRARIES} fmt::fmt)
include_directories(SYSTEM thirdparty/fmt-7.0.3/include)

include_directories(SYSTEM thirdparty/stb-2.26)
include_directories(SYSTEM thirdparty/imgui-1.89.2)

include_directories(thirdparty/tinyobjloader-2.0.0rc10)

find_package(OpenGL REQUIRED)
set(LIBRARIES ${LIBRARIES} ${OPENGL_gl_LIBRARY})


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	list(APPEND _589_689_CMAKE_CXX_FLAGS ${_589_689_CMAKE_CXX_FLAGS} "-Wall" "-pedantic")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	# using GCC
	list(APPEND _589_689_CMAKE_CXX_FLAGS ${_589_689_CMAKE_CXX_FLAGS} "-fdiagnostics-color=always" "-Wall" "-pedantic" "-Wconversion")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
	# using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_options("/permissive-")
	add_compile_options("/Zc:preprocessor")
	# Remove the default warning level.
	# https://stackoverflow.com/questions/58708772/cmake-project-in-visual-studio-gives-flag-override-warnings-command-line-warnin
	string(REGEX REPLACE "/W[3|4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	 # The above preprocessor warns about certain things in windows headers.
	 # Turn those off: https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/c5105?view=vs-2019
	add_compile_options("/wd5105")

	set(_589_689_CMAKE_CXX_FLAGS ${_589_689_CMAKE_CXX_FLAGS} /W3)

endif()

if(APPLE)
    set(LIBRARIES ${LIBRARIES} pthread dl)
elseif(UNIX)
    set(LIBRARIES ${LIBRARIES} pthread GL dl)
elseif(WIN32)
endif()


add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD=ON)
# include_directories(SYSTEM thirdparty/imgui thirdparty/imgui/examples)
# include_directories(src)


# Compile our main application
file(GLOB SOURCES
    589-689-skeleton/*
	thirdparty/imgui-1.89.2/*.cpp
)
set(INCLUDES ${INCLUDES} src)

set(APP_NAME "589-689-3D-skeleton")


# Copy all the shaders and tell the build system to re-run CMAKE if one of them changes.
# We use GLOB_RECURSE so that the "shaders" directory can have subfolders.
file(GLOB_RECURSE files 589-689-skeleton/shaders/*)
foreach(file ${files})
	# We need to extract the relative path of each file for when we copy it over.
	file(RELATIVE_PATH fileName ${CMAKE_CURRENT_SOURCE_DIR}/589-689-skeleton/shaders/ ${file})
	configure_file(${file} shaders/${fileName} COPYONLY)
endforeach()

# Do the same for textures.
file(GLOB_RECURSE files_t textures/*)
foreach(file ${files_t})
	file(RELATIVE_PATH fileName ${CMAKE_CURRENT_SOURCE_DIR}/textures/ ${file})
	configure_file(${file} textures/${fileName} COPYONLY)
endforeach()

# Do the same for 3D model files.
file(GLOB_RECURSE files_m models/*)
foreach(file ${files_m})
	file(RELATIVE_PATH fileName ${CMAKE_CURRENT_SOURCE_DIR}/models/ ${file})
	configure_file(${file} models/${fileName} COPYONLY)
endforeach()

add_executable(${APP_NAME} ${SOURCES} "589-689-skeleton/Framebuffer.cpp" "589-689-skeleton/Framebuffer.h" "589-689-skeleton/Renderbuffer.cpp" "589-689-skeleton/Plant.cpp" "589-689-skeleton/Plant.h" "589-689-skeleton/PlantPart.cpp" "589-689-skeleton/PlantPart.h" "589-689-skeleton/Callback.h" "589-689-skeleton/PointsData.h" "589-689-skeleton/ElementBuffer.h" "589-689-skeleton/ElementBuffer.cpp")
target_include_directories(${APP_NAME} PRIVATE ${INCLUDES})
target_link_libraries(${APP_NAME} ${LIBRARIES})
target_compile_definitions(${APP_NAME} PRIVATE ${DEFINITIONS})
target_compile_options(${APP_NAME} PRIVATE ${_589_689_CMAKE_CXX_FLAGS})
set_target_properties(${APP_NAME} PROPERTIES INSTALL_RPATH "./" BUILD_RPATH "./")
