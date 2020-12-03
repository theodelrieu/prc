#include <catch2/catch.hpp>

#include <prc/add.hpp>

TEST_CASE("add")
{
  SECTION("2 + 2 = 4")
  {
    CHECK(prc::add(2, 2) == 4);
  }
}
