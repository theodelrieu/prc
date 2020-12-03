add_library(tests_main OBJECT main.cpp)
get_target_property(CATCH2_INC_DIR CONAN_PKG::catch2 INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(tests_main PRIVATE ${CATCH2_INC_DIR})

add_executable(test_prc
  test_add.cpp
  $<TARGET_OBJECTS:tests_main>
)

target_link_libraries(test_prc prc CONAN_PKG::catch2)
add_test(NAME test_prc COMMAND test_prc)
