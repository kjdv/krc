add_executable(krc_test
  channel.cc
  executor.cc
  mutex.cc
  ptr_translate.cc
  queue.cc
  zero_queue.cc
)
target_link_libraries(krc_test krc gtest)

add_test(krc_test krc_test)
