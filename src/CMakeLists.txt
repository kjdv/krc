add_library(krc
    channel.hh
    context.hh
    context.cc
    executor.hh
    executor.cc
    io.hh
    io.cc
    mutex.hh
    mutex.cc
    ptr_translate.cc
    ptr_translate.hh
    no_copy.hh
    queue.hh
    runtime.hh
    runtime.cc
    zero_queue.hh
)
target_include_directories(krc
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

if(APPLE)
    target_compile_options(krc PUBLIC -D_XOPEN_SOURCE -Wno-deprecated-declarations)
endif()
