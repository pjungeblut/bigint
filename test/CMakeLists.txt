# Compiler flags.
set (CMAKE_CXX_FLAGS "-std=c++2a -fconcepts -Wall -Wextra -O3")

# Inlucde directories.
include_directories(../include)

# Build test for big_unsigned.
add_executable(big_unsigned big_unsigned.cpp)
target_link_libraries(big_unsigned gtest_main)
add_test(NAME big_unsigned COMMAND big_unsigned)

# Build test for big_integer.
add_executable(big_integer big_integer.cpp)
target_link_libraries(big_integer gtest_main)
add_test(NAME big_integer COMMAND big_integer)
