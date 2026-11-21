cmake_minimum_required(VERSION 3.20)
project(CorCraft LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

# Warning sets per compiler. Warnings are non-fatal here so that a reader can
# still run a lab that warns while learning what the warning means; the chapter
# rule is zero warnings, and labs/<track>/<stage>/check.sh reports any it sees.
if(MSVC)
  set(CRAFT_WARN_C /W4 /permissive- /utf-8)
  set(CRAFT_WARN_CXX /W4 /permissive- /utf-8 /EHsc)
  set(CRAFT_SAN "")
else()
  set(CRAFT_WARN_C -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
  set(CRAFT_WARN_CXX -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
  set(CRAFT_SAN -fsanitize=address,undefined -fno-omit-frame-pointer)
endif()

# Header-only helpers shared by every lab: assertion macro, CPU timer, file reader.
add_library(craft_common INTERFACE)
target_include_directories(craft_common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(craft_common INTERFACE
  $<$<COMPILE_LANGUAGE:C>:${CRAFT_WARN_C}>
  $<$<COMPILE_LANGUAGE:CXX>:${CRAFT_WARN_CXX}>)
if(NOT MSVC)
  target_compile_options(craft_common INTERFACE ${CRAFT_SAN})
  target_link_options(craft_common INTERFACE ${CRAFT_SAN})
  target_link_libraries(craft_common INTERFACE m)
endif()

enable_testing()

# craft_lab(<target> <source>)
# Registers one single-file lab, links the shared helpers, and adds a smoke test
# so that `ctest` covers every example present on disk.
function(craft_lab target source)
  add_executable(${target} ${source})
  target_link_libraries(${target} PRIVATE craft_common)
  add_test(NAME ${target} COMMAND ${target})
endfunction()

add_subdirectory(c)
add_subdirectory(cpp)
add_subdirectory(bio)

# The first page's example: one file, compiled four times in the prose, listed
# here so that `ctest` proves it still compiles with the project's warnings on.
craft_lab(warn warn.c)
