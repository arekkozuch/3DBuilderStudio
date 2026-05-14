get_filename_component(_TEST_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

add_executable(${_TEST_NAME}_tests
    test_network_stub.cpp
    test_build_sanity.cpp
    test_undo_redo_baseline.cpp
    test_ui_tabs.cpp
    test_calibration_removed.cpp
    test_slicing_disabled.cpp
    test_sidebar_objectlist.cpp
    test_no_profiles.cpp
    test_transform_free_z.cpp
    test_primitive_cube.cpp
    test_primitive_sphere.cpp
    test_primitive_cylinder.cpp
    test_primitive_cone.cpp
    test_measure_gizmo.cpp
    test_boolean_subtract.cpp
    test_boolean_subtract_ui.cpp
    test_boolean_union.cpp
    test_boolean_intersect.cpp
    test_plane_cut.cpp
    test_settle.cpp
    test_fileio_robustness.cpp
    test_obj_robustness.cpp
    test_memory_stress.cpp
    test_stl_export.cpp
    test_3mf_export.cpp
)

target_link_libraries(${_TEST_NAME}_tests test_common libslic3r_gui libslic3r Catch2::Catch2WithMain)
target_include_directories(${_TEST_NAME}_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
set_property(TARGET ${_TEST_NAME}_tests PROPERTY FOLDER "tests")

cmake_path(CONVERT "${CMAKE_SOURCE_DIR}/tests/fixtures" TO_NATIVE_PATH_LIST _FIXTURES_DIR NORMALIZE)
target_compile_definitions(${_TEST_NAME}_tests PRIVATE FIXTURES_DIR=R"\(${_FIXTURES_DIR}\)")

orcaslicer_copy_test_dlls()

catch_discover_tests(${_TEST_NAME}_tests PROPERTIES LABELS unit)
