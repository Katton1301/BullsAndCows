cmake_minimum_required(VERSION 3.14)

set(SERVER_TARGET_NAME "BullsAndCows.Server")

message(STATUS "BullsAndCows.GV: CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")

include_directories( ${CMAKE_CURRENT_LIST_DIR}/src)

set(sources "")
file(
        GLOB_RECURSE

        sources

        "${CMAKE_CURRENT_LIST_DIR}/*.h"
        "${CMAKE_CURRENT_LIST_DIR}/*.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

message(STATUS "[server] target name: ${SERVER_TARGET_NAME}")

find_package(Boost REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(WIN32)
    add_link_options(-Wl,-subsystem=windows)
endif()

add_executable( ${SERVER_TARGET_NAME} ${sources} )

set(server_libraries "")
set(server_libraries ${server_libraries} "${MODEL_TARGET_NAME}")
message(STATUS "[server] project dependencies: ${server_libraries}")

target_link_libraries(${SERVER_TARGET_NAME} ${server_libraries})
target_link_libraries(${SERVER_TARGET_NAME} Boost::boost)

if(WIN32)
    target_link_libraries(${SERVER_TARGET_NAME} ws2_32 mswsock)
endif()

set_target_properties(${SERVER_TARGET_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

install(
        CODE "set( DESTINATION_PATH \"${CMAKE_INSTALL_BINDIR}/server\")"
)

install( TARGETS ${SERVER_TARGET_NAME} DESTINATION "${CMAKE_INSTALL_BINDIR}/server" )
