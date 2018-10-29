add_executable(executor
    executor.cc
)
target_link_libraries(executor krc)

add_executable(fibonacci
    fibonacci.cc
)
target_link_libraries(fibonacci krc)
