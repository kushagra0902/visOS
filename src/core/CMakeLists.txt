add_library(visos_core STATIC
    process_control_block.cpp
)
target_include_directories(visos_core PUBLIC ${CMAKE_SOURCE_DIR}/src)
