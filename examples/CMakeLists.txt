add_executable(executor
    executor.cc
)
target_link_libraries(executor krc)

add_executable(fibonacci
    fibonacci.cc
)
target_link_libraries(fibonacci krc)

add_executable(context_switch
    context_switch.cc
)
target_link_libraries(context_switch krc)
