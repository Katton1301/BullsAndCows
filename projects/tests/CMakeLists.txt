message(STATUS "BullsAndCows.Test: CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")

set(t_libraries "")
set(t_libraries ${t_libraries} "${MODEL_TARGET_NAME}")
message(STATUS "[tests] project dependencies: ${t_libraries}")

set(CMAKE_PREFIX_PATH "${CMAKE_INSTALL_PREFIX}/Catch2")

find_package(Catch2 3 REQUIRED)
file(GLOB files "src/unit_tests/unit-*.cpp")

foreach(file ${files})
    get_filename_component(file_basename ${file} NAME_WE)
    string(REGEX REPLACE "unit-([^$]+)" "test-\\1" testcase ${file_basename})

    message(STATUS "[tests] test case name: ${testcase}")
    add_executable(${testcase} ${file})
    target_link_libraries(${testcase} ${t_libraries} Catch2::Catch2WithMain)
    add_test(${testcase} ${testcase})
endforeach()
