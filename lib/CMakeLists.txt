cmake_minimum_required(VERSION 3.3)

add_library(libprc
  src/equilab/serialize.cpp
  src/pio/serialize.cpp
  src/pio/parse.cpp
  src/pio/api_def.cpp
  src/equilab/parse.cpp
  src/equilab/api_def.cpp
  src/gtoplus/api_def.cpp
  src/gtoplus/serialize.cpp
  src/combo.cpp
  src/unpaired_hand.cpp
  src/paired_hand.cpp
  src/hand.cpp
  src/hand_range.cpp
  src/range_elem.cpp
  src/range.cpp
  src/folder.cpp
  src/card.cpp
  src/api_def.cpp
)

set_target_properties(libprc PROPERTIES PREFIX "")

target_include_directories(libprc
  PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_definitions(libprc PUBLIC BOOST_SPIRIT_X3_UNICODE)

target_link_libraries(libprc CONAN_PKG::boost CONAN_PKG::mgs)

if (BUILD_TESTING)
  add_subdirectory(test)
endif()
