cmake_minimum_required(VERSION 3.1)

#set(PYTHON_INCLUDE_DIRS /usr/include/python2.7/)
#set(PYTHON_LIBRARIES /usr/lib/python2.7/)

find_package(LLVM REQUIRED CONFIG)
find_package(PythonLibs REQUIRED)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(${PYTHON_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
link_directories(${CMAKE_CURRENT_BINARY_DIR})
link_directories(${PYTHON_LIBRARIES})

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")

add_subdirectory(src)  # Use your pass name here.

message(INFO, "${PYTHON_INCLUDE_DIRS}")
message(INFO, "${PYTHON_LIBRARIES}")
