add_executable(krc_test
  context.cc
  channel.cc
  executor.cc
  io.cc
  mutex.cc
  ptr_translate.cc
  queue.cc
  zero_queue.cc
)
target_link_libraries(krc_test krc krc_internal gtest)

add_test(krc_test krc_test)
