# CMake entry point
cmake_minimum_required (VERSION 3.0)
project (mandlebrots)
find_package(OpenGL REQUIRED)

set(CMAKE_CXX_FLAGS "-g -Wall")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compile external dependencies 
add_subdirectory (external)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   set(CMAKE_CXX_FLAGS "-std=c++11 -Wall")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
   set(CMAKE_CXX_FLAGS "-std=c++11 -Wall")
endif()

include_directories(
	SYSTEM
	external/glm-0.9.4.0/
	external/glfw-3.0.3/include/GLFW/
	external/glew-1.9.0/include/
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	GLFW_303
	GLEW_190
)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

# Messing Around
add_subdirectory(messing_around)
