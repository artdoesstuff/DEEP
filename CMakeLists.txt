cmake_minimum_required(VERSION 3.10)
project(DEEP C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

include_directories(include)

set(DEEP_LIB_SOURCES
    src/bignum.c
    src/game.c
    src/planet.c
    src/player.c
    src/prestige.c
    src/research.c
    src/resources.c
    src/save.c
    src/ship.c
    src/simulation.c
    src/discovery.c
    src/cli.c
)

find_library(MATH_LIBRARY m)

add_executable(deep src/main.c ${DEEP_LIB_SOURCES})
if(MATH_LIBRARY)
    target_link_libraries(deep ${MATH_LIBRARY})
endif()

enable_testing()
add_executable(deep_tests tests/test_deep.c ${DEEP_LIB_SOURCES})
if(MATH_LIBRARY)
    target_link_libraries(deep_tests ${MATH_LIBRARY})
endif()
add_test(NAME deep_unit_tests COMMAND deep_tests)
