message(STATUS "BullsAndCows.Test: CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")

set(t_libraries "")
set(t_libraries ${t_libraries} "${MODEL_TARGET_NAME}")
message(STATUS "[tests] project dependencies: ${t_libraries}")

add_library(catch_main OBJECT "src/catch_main.cpp")

file(GLOB files "src/unit_tests/unit-*.cpp")

foreach(file ${files})
    get_filename_component(file_basename ${file} NAME_WE)
    string(REGEX REPLACE "unit-([^$]+)" "test-\\1" testcase ${file_basename})

    message(STATUS "[tests] test case name: ${testcase}")

    add_executable(${testcase} $<TARGET_OBJECTS:catch_main> ${file})
    target_link_libraries(${testcase} ${t_libraries})
    add_test(${testcase} ${testcase})
endforeach()
