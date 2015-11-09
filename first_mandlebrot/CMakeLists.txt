include_directories(${fractals_SOURCE_DIR}/first_mandlebrot)
link_directories(${fractals_SOURCE_DIR}/first_mandlebrot)

add_executable(first_mandlebrot
    main.cpp
    passthrough.vert
    mand.frag
    mand_single.frag
)

target_link_libraries(first_mandlebrot
    ${ALL_LIBS}
)
# Xcode and Visual working directories
set_target_properties(first_mandlebrot PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/first_mandlebrot/")

SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*frag$" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*vert$" )

add_custom_target(run
    COMMAND first_mandlebrot
    DEPENDS first_mandlebrot
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)