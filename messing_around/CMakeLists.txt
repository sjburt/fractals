include_directories(${mandlebrots_SOURCE_DIR}/messing_around)
link_directories(${mandlebrots_SOURCE_DIR}/messing_around)

add_executable(messing_around
    main.cpp
    passthrough.vert
    mand.frag
    mand_single.frag
)

target_link_libraries(messing_around
    ${ALL_LIBS}
)
# Xcode and Visual working directories
set_target_properties(messing_around PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/messing_around/")

SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*frag$" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*vert$" )

add_custom_target(run
    COMMAND messing_around
    DEPENDS messing_around
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)