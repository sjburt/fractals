include_directories(${fractals_SOURCE_DIR}/second_mandlebrot)
link_directories(${fractals_SOURCE_DIR}/second_mandlebrot)

find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})

add_executable(second_mandlebrot
    main.cpp
    passthrough.vert
    mand.frag
    mand_single.frag
    mand.cpp
    mand.hpp
    text.cpp
    text.hpp
    ../common/text2D.cpp
    ../common/text2D.hpp
    ../common/shader.cpp
    ../common/shader.hpp
    ../common/texture.cpp
    ../common/texture.hpp
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   set(CMAKE_CXX_LINK_FLAGS "-lprofiler")
endif()

target_link_libraries(second_mandlebrot
    ${ALL_LIBS}
    ${FREETYPE_LIBRARIES}
)
# Xcode and Visual working directories
set_target_properties(second_mandlebrot PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/second_mandlebrot/")

SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*frag$" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*vert$" )

add_custom_target(run_second
    COMMAND second_mandlebrot
    DEPENDS second_mandlebrot
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)