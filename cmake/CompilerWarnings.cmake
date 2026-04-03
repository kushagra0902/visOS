function(set_project_warnings target_name)
    set(WARNINGS
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wformat=2
    )

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        list(APPEND WARNINGS -Wduplicated-cond -Wduplicated-branches -Wlogical-op)
    endif()

    target_compile_options(${target_name} PRIVATE ${WARNINGS})
endfunction()
