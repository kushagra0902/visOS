add_library(visos_util STATIC
    pid_generator.cpp
    logger.cpp
)
target_include_directories(visos_util PUBLIC ${CMAKE_SOURCE_DIR}/src)
