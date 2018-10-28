cmake_minimum_required(VERSION 3.9)

project(KlaasRoutineandChannelLibrary)
set(CMAKE_CXX_STANDARD 17)

find_package (Threads REQUIRED)
link_libraries(${CMAKE_THREAD_LIBS_INIT})

find_package(GTest REQUIRED)
find_library(GMOCK_LIBRARIES gmock)
add_library(gtest INTERFACE)
target_include_directories(gtest
    INTERFACE ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
)
target_link_libraries(gtest
    INTERFACE ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} ${GTEST_MAIN_LIBRARIES})

enable_testing()

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(examples)
