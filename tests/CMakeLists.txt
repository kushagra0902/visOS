add_executable(visos_tests
    core/test_process_control_block.cpp
    core/test_process_state_transitions.cpp
    core/test_program_metadata.cpp
    services/test_cpu.cpp
    services/test_scheduler.cpp
    services/test_memory_manager.cpp
    services/test_clock_service.cpp
    services/test_syscall_handler.cpp
    services/test_program_store.cpp
    services/test_scheduling_strategies.cpp
)

target_link_libraries(visos_tests PRIVATE visos_kernel visos_util GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(visos_tests)
