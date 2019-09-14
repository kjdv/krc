file(GLOB public_headers include/*.hh include/*.h)
file(GLOB sources *.cc *.cpp *.c *.hh *.h)

add_library(krc
    ${public_headers}
    ${sources}
)

set_target_properties(krc PROPERTIES PUBLIC_HEADER "${public_headers}")
target_include_directories(krc
    PUBLIC
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
      "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>"
)

target_link_libraries(krc PUBLIC Threads::Threads)

if(APPLE)
    # for osx we need to pass -D_XOPEN_SOURCE to be able to use the 'deprecated' {set,swap,make}context functions
    target_compile_options(krc PRIVATE -D_XOPEN_SOURCE -Wno-deprecated-declarations)
endif()
