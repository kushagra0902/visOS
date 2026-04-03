add_library(visos_kernel STATIC
    kernel.cpp
    simulation_engine.cpp
)
target_link_libraries(visos_kernel PUBLIC visos_services visos_util)
