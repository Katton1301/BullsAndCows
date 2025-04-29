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

find_package(PkgConfig REQUIRED)
pkg_check_modules(RDKAFKA REQUIRED rdkafka)

if(NOT RDKAFKA_FOUND)
    message(WARNING "Falling back to manual paths for librdkafka")
    set(RDKAFKA_LIBRARY_DIRS "/usr/lib/x86_64-linux-gnu")
    set(RDKAFKA_LIBRARIES rdkafka)
endif()

if(NOT RDKAFKA_INCLUDE_DIRS)
    set(RDKAFKA_INCLUDE_DIRS "/usr/include/librdkafka")
endif()


message(STATUS "RDKAFKA LIBRARIES: ${RDKAFKA_LIBRARIES}")
message(STATUS "RDKAFKA LIBRARIES DIRS: ${RDKAFKA_LIBRARY_DIRS}")
message(STATUS "RDKAFKA INCLUDE DIRS: ${RDKAFKA_INCLUDE_DIRS}")

if(WIN32)
    add_link_options(-Wl,-subsystem=windows)
endif()

add_executable( ${SERVER_TARGET_NAME} ${sources} )
target_include_directories(${SERVER_TARGET_NAME} PRIVATE ${RDKAFKA_INCLUDE_DIRS})

set(server_libraries "")
set(server_libraries ${server_libraries} "${MODEL_TARGET_NAME}")
message(STATUS "[server] project dependencies: ${server_libraries}")

target_link_directories(${SERVER_TARGET_NAME} PRIVATE
    ${RDKAFKA_LIBRARY_DIRS}
)

target_link_libraries(${SERVER_TARGET_NAME} ${server_libraries})
target_link_libraries(${SERVER_TARGET_NAME} rdkafka rdkafka++ pthread ssl crypto sasl2 z dl)

if(WIN32)
    target_link_libraries(${SERVER_TARGET_NAME} ws2_32 mswsock)
endif()

set_target_properties(${SERVER_TARGET_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

install(
        CODE "set( DESTINATION_PATH \"${CMAKE_INSTALL_BINDIR}/server\")"
)

install( TARGETS ${SERVER_TARGET_NAME} DESTINATION "${CMAKE_INSTALL_BINDIR}/server" )

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -Wl,--no-undefined")
