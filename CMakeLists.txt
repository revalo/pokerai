cmake_minimum_required(VERSION 3.10)

project(PokerAI)

set (CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
add_subdirectory(pai/third_party/absl-cpp)
add_subdirectory(pai/third_party/leveldb)
add_subdirectory(pai/third_party/ompeval)
# add_subdirectory(pai/third_party/pkmeans)
# add_subdirectory(pai/third_party/isomorph)

include_directories(
  pai/third_party
  pai/third_party/absl-cpp
  pai/third_party/leveldb/include
  pai/third_party/ompeval
)

set(ROOT_DIR pai/mccfr)
set(ABSTRACTION_DIR pai/abstraction)

set(LIB_SOURCES
  ${ROOT_DIR}/infoset.cc
  ${ROOT_DIR}/infoset.h
  ${ROOT_DIR}/infotable.cc
  ${ROOT_DIR}/infotable.h
  ${ROOT_DIR}/rng.cc
  ${ROOT_DIR}/rng.h
  ${ROOT_DIR}/external_sampling_mccfr.cc
  ${ROOT_DIR}/external_sampling_mccfr.h
  ${ROOT_DIR}/game/liars_dice.cc
  ${ROOT_DIR}/game/liars_dice.h
  ${ROOT_DIR}/game/game.h
  ${ABSTRACTION_DIR}/hand_distribution.cc
  ${ABSTRACTION_DIR}/hand_distribution.h
  ${ABSTRACTION_DIR}/hand_indexer.h
  ${ABSTRACTION_DIR}/hand_indexer.cc
)

set(TEST_SOURCES
  ${ROOT_DIR}/infoset_test.cc
  ${ROOT_DIR}/infotable_test.cc
  ${ROOT_DIR}/rng_test.cc
  ${ROOT_DIR}/game/liars_dice_test.cc
)

add_library(pokerai ${LIB_SOURCES})
target_link_libraries(pokerai absl::strings absl::flat_hash_map leveldb omp)

add_executable(playground pai/mccfr/playground.cc)
add_executable(train_liars pai/mccfr/train_liars.cc)
add_executable(play_liars pai/mccfr/play_liars.cc)
add_executable(omp_playground pai/abstraction/omp_playground.cc)
add_executable(generate_distributions pai/abstraction/generate_distributions.cc)
add_executable(hand_indexer_playground pai/abstraction/hand_indexer_playground.cc)
add_executable(cluster pai/abstraction/cluster.cc)
target_link_libraries(train_liars pokerai absl::strings absl::flags absl::flags_parse)
target_link_libraries(play_liars pokerai absl::strings absl::flags absl::flags_parse)
target_link_libraries(omp_playground pokerai absl::strings absl::flags absl::flags_parse)
target_link_libraries(generate_distributions pokerai absl::strings absl::flags absl::flags_parse)
target_link_libraries(hand_indexer_playground pokerai absl::strings absl::flags absl::flags_parse)
target_link_libraries(cluster pokerai absl::strings absl::flags absl::flags_parse)

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
