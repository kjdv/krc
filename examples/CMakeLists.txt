add_executable(context_switch
    context_switch.cc
)
target_link_libraries(context_switch krc krc_internal)
add_test(context_switch context_switch)

add_executable(executor
    executor.cc
)
target_link_libraries(executor krc)
add_test(executor executor)

add_executable(fibonacci
    fibonacci.cc
)
target_link_libraries(fibonacci krc)
add_test(fibonacci fibonacci 10)

add_executable(io
    io.cc
)
target_link_libraries(io krc)
add_test(io io )

add_executable(multithreading
    multithreading.cc
)
target_link_libraries(multithreading krc)
add_test(multithreading multithreading)

