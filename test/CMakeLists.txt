add_executable(krc_test
  executor.cc
)
target_link_libraries(krc_test krc gtest)

add_test(krc_test krc_test)
