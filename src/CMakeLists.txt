add_library(krc
    channel.hh
    channel.cc
    executor.hh
    executor.cc
)
target_include_directories(krc
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
