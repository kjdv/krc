add_library(krc
    channel.hh
    closed_exception.hh
    executor.hh
    executor.cc
    queue.hh
    zero_queue.hh
)
target_include_directories(krc
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

if(APPLE)
    target_compile_options(krc PUBLIC -DNO_UCONTEXT)
endif()
