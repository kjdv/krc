file(GLOB sources *.c *.cc *.cpp *.h *.hh)

add_executable(krc_test ${sources})
target_link_libraries(krc_test krc ${GMOCK_LIBRARIES} GTest::GTest GTest::Main)

gtest_discover_tests(krc_test)
