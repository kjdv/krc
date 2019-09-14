cmake_minimum_required(VERSION 3.10)

project(krc)
set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

include(GoogleTest)
find_package(GTest MODULE REQUIRED)
find_library(GMOCK_LIBRARIES gmock)
enable_testing()


add_subdirectory(lib)
add_subdirectory(test)
add_subdirectory(examples)

# install rules
install(TARGETS krc
        EXPORT KrcConfig
        ARCHIVE DESTINATION lib COMPONENT lib
        PUBLIC_HEADER DESTINATION include/krc COMPONENT dev
)

export(TARGETS krc NAMESPACE Krc:: FILE ${CMAKE_CURRENT_BINARY_DIR}/KrcConfig.cmake)
install(EXPORT KrcConfig DESTINATION share/krc NAMESPACE Krc::)
