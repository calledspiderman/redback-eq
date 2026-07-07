find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE REQUIRED pipewire-0.3)

add_executable(radioform-filter
    main.c
)

target_include_directories(radioform-filter
    PRIVATE
        ${DSP_SOURCE_DIR}/include
        ${PIPEWIRE_INCLUDE_DIRS}
)

target_compile_options(radioform-filter PRIVATE ${PIPEWIRE_CFLAGS})

target_link_libraries(radioform-filter
    PRIVATE
        radioform_dsp
        ${PIPEWIRE_LIBRARIES}
        m
)

install(TARGETS radioform-filter
    RUNTIME DESTINATION bin
)
