cmake_minimum_required(VERSION 3.3)

add_executable(prc main.cpp actions.cpp)
target_link_libraries(prc libprc CONAN_PKG::lyra)
