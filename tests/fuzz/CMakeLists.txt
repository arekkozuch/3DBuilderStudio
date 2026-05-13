cmake_minimum_required(VERSION 3.20)

# Fuzz targets require a compiler with libFuzzer support (clang -fsanitize=fuzzer).
# They are NOT part of the default build. Enable with:
#   cmake -DMESHFORGE_FUZZ=ON ...
#   make fuzz_stl fuzz_3mf fuzz_obj

option(MESHFORGE_FUZZ "Build libFuzzer harnesses" OFF)

if(MESHFORGE_FUZZ)
    foreach(_target fuzz_stl fuzz_3mf fuzz_obj)
        add_executable(${_target} ${_target}.cpp)
        target_link_libraries(${_target} PRIVATE libslic3r)
        target_include_directories(${_target} PRIVATE ${CMAKE_SOURCE_DIR}/src)
        target_compile_options(${_target} PRIVATE -fsanitize=fuzzer,address -g)
        target_link_options(${_target} PRIVATE -fsanitize=fuzzer,address)
        set_property(TARGET ${_target} PROPERTY FOLDER "fuzz")
    endforeach()
endif()
