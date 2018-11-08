add_library(krc_internal INTERFACE) # interface librarie for those who have a legitimate reason to include internal headers (like uni tests)
target_include_directories(krc_internal INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(krc
    include/channel.hh
    include/io.hh
    include/runtime.hh

    include/internal/queue.hh
    include/internal/zero_queue.hh
    include/internal/mutex.hh
    include/internal/no_copy.hh

    context.hh
    context.cc
    executor.hh
    executor.cc
    io.cc
    mutex.cc
    ptr_translate.cc
    ptr_translate.hh
    runtime.cc
)
target_link_libraries(krc PRIVATE krc_internal)
target_include_directories(krc
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(APPLE)
    # for osx we need to pass -D_XOPEN_SOURCE to be able to use the 'deprecated' {set,swap,make}context functions
    # alsi, we need to set the minimal deployement target to 10.14 to be able to use certain c++17 features (i.e.
    # variant visitors, optional access)
    target_compile_options(krc PUBLIC -D_XOPEN_SOURCE -Wno-deprecated-declarations -mmacosx-version-min=10.14)
    target_link_libraries(krc PUBLIC -mmacosx-version-min=10.14)
endif()
