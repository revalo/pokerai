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

set(ROOT_DIR pai/mccfr)

set(LIB_SOURCES
  ${ROOT_DIR}/infoset.cc
  ${ROOT_DIR}/infoset.h
  ${ROOT_DIR}/infotable.cc
  ${ROOT_DIR}/infotable.h
  ${ROOT_DIR}/rng.cc
  ${ROOT_DIR}/rng.h
  ${ROOT_DIR}/game/liars_dice.cc
  ${ROOT_DIR}/game/liars_dice.h
  ${ROOT_DIR}/game/game.h
)

set(TEST_SOURCES
  ${ROOT_DIR}/infoset_test.cc
  ${ROOT_DIR}/infotable_test.cc
  ${ROOT_DIR}/rng_test.cc
  ${ROOT_DIR}/game/liars_dice_test.cc
)

add_library(pokerai ${LIB_SOURCES})

add_executable(playground pai/mccfr/playground.cc)

enable_testing()
include(GoogleTest)

# For each test, add an executable that runs the test.
# This executable is added to the test target.
foreach(test_source ${TEST_SOURCES})
  get_filename_component(test_name ${test_source} NAME_WE)
  add_executable(${test_name} ${test_source})
  target_link_libraries(${test_name} gtest_main pokerai)
  gtest_discover_tests(${test_name})
endforeach()
