cmake_minimum_required(VERSION 3.14)

set(GAME_VIEW_TARGET_NAME "BullsAndCows.GameView")

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

message(STATUS "[game_view] target name: ${GAME_VIEW_TARGET_NAME}")

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_LIST_DIR}/gui)

add_link_options(-Wl,-subsystem=windows)

add_executable( ${GAME_VIEW_TARGET_NAME} ${sources} )

set(game_view_libraries "")
set(game_view_libraries ${game_view_libraries} "${MODEL_TARGET_NAME}")
message(STATUS "[game_view] project dependencies: ${game_view_libraries}")

target_link_libraries(${GAME_VIEW_TARGET_NAME} ${game_view_libraries})
target_link_libraries(${GAME_VIEW_TARGET_NAME} Qt5::Widgets)
target_link_libraries(${GAME_VIEW_TARGET_NAME} Qt5::Core)

set_target_properties(${GAME_VIEW_TARGET_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

install(
        CODE "set( DESTINATION_PATH \"${CMAKE_INSTALL_BINDIR}/game_view\")"
)

install( TARGETS ${GAME_VIEW_TARGET_NAME} DESTINATION "${CMAKE_INSTALL_BINDIR}/game_view" )
