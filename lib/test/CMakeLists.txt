cmake_minimum_required(VERSION 3.3)

add_executable(test_prc
  test_pio.cpp
  test_equilab.cpp
  test_parser.cpp
  test_prc.cpp
  main.cpp
)

target_link_libraries(test_prc libprc CONAN_PKG::catch2)
add_test(NAME test_prc COMMAND test_prc --test-data-dir=${CMAKE_CURRENT_LIST_DIR}/data)
