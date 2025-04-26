cmake_minimum_required(VERSION 3.14)

set(DECISION_TREE_GENERATOR_TARGET_NAME "BullsAndCows.DecisionTreeGenerator")

message(STATUS "BullsAndCows.SC: CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")

include_directories( ${CMAKE_CURRENT_LIST_DIR}/src)

set(sources "")
file(
        GLOB_RECURSE

        sources

        "${CMAKE_CURRENT_LIST_DIR}/*.h"
        "${CMAKE_CURRENT_LIST_DIR}/*.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

message(STATUS "[decision_tree_generator] target name: ${DECISION_TREE_GENERATOR_TARGET_NAME}")

if(WIN32)
    add_link_options(-Wl,-subsystem=windows)
endif()

add_executable( ${DECISION_TREE_GENERATOR_TARGET_NAME} ${sources} )

set(calc_libraries "")
set(calc_libraries ${calc_libraries} "${MODEL_TARGET_NAME}")
message(STATUS "[decision_tree_generator] project dependencies: ${calc_libraries}")

target_link_libraries(${DECISION_TREE_GENERATOR_TARGET_NAME} ${calc_libraries})

set_target_properties(${DECISION_TREE_GENERATOR_TARGET_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

install(
        CODE "set( DESTINATION_PATH \"${CMAKE_INSTALL_BINDIR}/decision_tree_generator\")"
)

install( TARGETS ${DECISION_TREE_GENERATOR_TARGET_NAME} DESTINATION "${CMAKE_INSTALL_BINDIR}/decision_tree_generator" )
