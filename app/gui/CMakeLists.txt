find_package(Qt6 6.7 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(visos_gui
    main.cpp
    adapters/kernel_observer_adapter.cpp
    models/process_table_model.cpp
    models/queue_model.cpp
    models/gantt_model.cpp
    widgets/main_window.cpp
    widgets/process_table_widget.cpp
    widgets/queue_view_widget.cpp
    widgets/simulation_controls_widget.cpp
    widgets/event_log_widget.cpp
    widgets/program_submission_dialog.cpp
    widgets/gantt_widget.cpp
    widgets/process_detail_dialog.cpp
)

target_link_libraries(visos_gui PRIVATE
    visos_sim
    visos_util
    Qt6::Widgets
)

target_include_directories(visos_gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src
)

if(APPLE)
    # AGL framework was removed in macOS 26 SDK but Qt 6.7 still references it.
    # Link the tbd stub directly from an older SDK that still includes it.
    set(_agl_sdk_path "")
    foreach(_sdk_ver "15.4" "15")
        set(_candidate "/Library/Developer/CommandLineTools/SDKs/MacOSX${_sdk_ver}.sdk/System/Library/Frameworks")
        if(EXISTS "${_candidate}/AGL.framework")
            set(_agl_sdk_path "${_candidate}")
            break()
        endif()
    endforeach()
    if(_agl_sdk_path)
        target_link_options(visos_gui PRIVATE "LINKER:-F,${_agl_sdk_path}")
    endif()
endif()
