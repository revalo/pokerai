cmake_minimum_required(VERSION 3.10)

project(PokerAI)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(infoset pai/mccfr/infoset.cc)

add_executable(liars pai/mccfr/liars_dice.cc)

enable_testing()

add_executable(
  infoset_test
  pai/mccfr/infoset_test.cc
)
target_link_libraries(
  infoset_test
  gtest_main
  infoset
)

include(GoogleTest)
gtest_discover_tests(infoset_test)
