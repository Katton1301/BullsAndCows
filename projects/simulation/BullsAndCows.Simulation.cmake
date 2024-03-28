cmake_minimum_required(VERSION 3.14)

set(SIMULATION_TARGET_NAME "BullsAndCows.Simulation")

message(STATUS "BullsAndCows.GV: CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")

include_directories( ${CMAKE_CURRENT_LIST_DIR}/src)

set(sources "")
file(
        GLOB_RECURSE

        sources

        "${CMAKE_CURRENT_LIST_DIR}/*.h"
        "${CMAKE_CURRENT_LIST_DIR}/*.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/*.ui"
)

message(STATUS "[game_view] target name: ${SIMULATION_TARGET_NAME}")

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_LIST_DIR}/gui)

add_link_options(-Wl,-subsystem=windows)

add_executable( ${SIMULATION_TARGET_NAME} ${sources} )

set(simulation_libraries "")
set(simulation_libraries ${simualation_libraries} "${MODEL_TARGET_NAME}")
message(STATUS "[simulation_view] project dependencies: $simulation_libraries")

target_link_libraries(${SIMULATION_TARGET_NAME} ${simulation_libraries})
target_link_libraries(${SIMULATION_TARGET_NAME} Qt5::Widgets)
target_link_libraries(${SIMULATION_TARGET_NAME} Qt5::Core)

set_target_properties(${SIMULATION_TARGET_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

install(
        CODE "set( DESTINATION_PATH \"${CMAKE_INSTALL_BINDIR}/simulation_view\")"
)

install( TARGETS ${SIMULATION_TARGET_NAME} DESTINATION "${CMAKE_INSTALL_BINDIR}/simulation_view" )
