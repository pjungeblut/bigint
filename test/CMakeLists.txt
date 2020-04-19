# Compiler flags.
set (CMAKE_CXX_FLAGS "-std=c++2a -Wall -Wextra -O3")

# Include the necessary files.
include_directories(.)
include_directories(../include)

# Select all *.cpp files.
file(GLOB cppFiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Compile each file.
foreach(cppFile ${cppFiles})
  add_executable("${cppFile}" "${cppFile}")
endForeach(cppFile)
