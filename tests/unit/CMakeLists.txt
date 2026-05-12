get_filename_component(_TEST_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

set(_FIXTURES_DIR ${CMAKE_SOURCE_DIR}/tests/fixtures)
file(TO_NATIVE_PATH "${_FIXTURES_DIR}" _FIXTURES_DIR_NATIVE)

add_executable(${_TEST_NAME}_tests
    test_network_stub.cpp
    test_build_sanity.cpp
    test_undo_redo_baseline.cpp
    test_ui_tabs.cpp
)

target_compile_definitions(${_TEST_NAME}_tests PRIVATE TEST_DATA_DIR=R"\(${_FIXTURES_DIR_NATIVE}\)")
target_link_libraries(${_TEST_NAME}_tests test_common libslic3r_gui libslic3r Catch2::Catch2WithMain)
target_include_directories(${_TEST_NAME}_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
set_property(TARGET ${_TEST_NAME}_tests PROPERTY FOLDER "tests")

orcaslicer_copy_test_dlls()

catch_discover_tests(${_TEST_NAME}_tests PROPERTIES LABELS unit)
