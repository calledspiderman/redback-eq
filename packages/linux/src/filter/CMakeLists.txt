add_executable(radioform-filter
    main.c
)

target_include_directories(radioform-filter
    PRIVATE
        ${DSP_SOURCE_DIR}/include
)

target_link_libraries(radioform-filter
    PRIVATE
        radioform_dsp
        pipewire-0.3
        m
)

install(TARGETS radioform-filter
    RUNTIME DESTINATION bin
)
