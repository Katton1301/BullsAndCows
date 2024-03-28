cmake_minimum_required(VERSION 3.14)

set(PROJECT_NAME "BullsAndCows")

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_DEBUG_POSTFIX .debug)

set(CMAKE_INSTALL_LIBDIR "lib" )
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_INSTALL_BINDIR "${CMAKE_SOURCE_DIR}/_binaries" )

include_directories( ${CMAKE_CURRENT_LIST_DIR}/model/src/)

add_compile_options(-Wall -Wextra -Wreturn-type -Wunused -Wno-terminate)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    add_compile_options( -O3 )
else()
    add_compile_options( -O0 )
    add_compile_options( -DDEBUG_TARGET)
endif()
add_compile_options( -mno-ms-bitfields )


get_property(include_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
get_property(linker_options DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY LINK_OPTIONS)

set(MODEL_TARGET_NAME "${PROJECT_NAME}.Model")
message(STATUS "[model] target name: ${MODEL_TARGET_NAME}")

include(model/BullsAndCows.Model.cmake)
include(game_view/BullsAndCows.GameView.cmake)
include(simulation/BullsAndCows.Simulation.cmake)
#enable_testing()
#add_subdirectory(tests)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES ${include_dirs})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_OPTIONS ${linker_options})
