add_library(krc_internal INTERFACE) # interface librarie for those who have a legitimate reason to include internal headers (like uni tests)
target_include_directories(krc_internal INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(krc
    include/channel.hh
    include/io.hh
    include/runtime.hh

    include/internal/condition_variable.hh
    include/internal/queue.hh
    include/internal/zero_queue.hh
    include/internal/mutex.hh
    include/internal/no_copy.hh

    condition_variable.cc
    context.hh
    context.cc
    debug.hh
    debug.cc
    executor.hh
    executor.cc
    io.cc
    mutex.cc
    ptr_translate.cc
    ptr_translate.hh
    runtime.cc
    single_executor.hh
    single_executor.cc
)
target_link_libraries(krc PRIVATE krc_internal)
target_include_directories(krc
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(APPLE)
    target_compile_options(krc PUBLIC -D_XOPEN_SOURCE -Wno-deprecated-declarations)
endif()
