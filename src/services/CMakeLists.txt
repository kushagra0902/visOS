add_library(visos_services STATIC
    ready_queue.cpp
    blocked_queue.cpp
    memory_manager.cpp
    program_store.cpp
    syscall_handler.cpp
    clock_service.cpp
    timer_interrupt.cpp
    cpu.cpp
    scheduler.cpp
    io/io_device_simulator.cpp
    scheduling/fcfs_strategy.cpp
    scheduling/round_robin_strategy.cpp
    scheduling/priority_strategy.cpp
)
target_link_libraries(visos_services PUBLIC visos_core)
target_include_directories(visos_services PUBLIC ${CMAKE_SOURCE_DIR}/src)
