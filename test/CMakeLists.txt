add_executable(krc_test
  context.cc
  channel.cc
  io.cc
  multi_executor.cc
  mutex.cc
  ptr_translate.cc
  queue.cc
  single_executor.cc
  ringbuffer.cc
  unbuffered.cc
  zero_queue.cc
)
target_link_libraries(krc_test krc krc_internal gtest)

add_test(unit_test krc_test)
add_test(soak_test krc_test --gtest_shuffle --gtest_repeat=100)
