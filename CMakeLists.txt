cmake_minimum_required(VERSION 3.10)
include(CheckFunctionExists)

set(PROJECT_MAJOR 0)
set(PROJECT_MINOR 6)
set(PROJECT_MICRO 0)
project(textgen VERSION ${PROJECT_MAJOR}.${PROJECT_MINOR}.${PROJECT_MICRO})

add_compile_options(-g -Wall -Wextra -Werror -pedantic)
add_executable(textgen textgen.c grammar.c options.c operators.c randfunc_ops.c randfuncs.c)
target_link_libraries(textgen jansson m)

check_function_exists(asprintf HAVE_ASPRINTF)
check_function_exists(arc4random HAVE_ARC4RANDOM)

if(HAVE_ASPRINTF AND USE_ASPRINTF)
  set(USE_ASPRINTF 1)
  message(STATUS "Using asprintf: _GNU_SOURCE defined")
else()
  unset(USE_ASPRINTF)
endif()

configure_file (config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h @ONLY)
