find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE REQUIRED libpipewire-0.3)
pkg_check_modules(SPA REQUIRED libspa-0.2)

add_executable(radioform-filter
    main.c
)

target_include_directories(radioform-filter
    PRIVATE
        ${DSP_SOURCE_DIR}/include
        ${PIPEWIRE_INCLUDE_DIRS}
        ${SPA_INCLUDE_DIRS}
)

target_compile_options(radioform-filter PRIVATE ${PIPEWIRE_CFLAGS} ${SPA_CFLAGS})

target_link_libraries(radioform-filter
    PRIVATE
        radioform_dsp
        ${PIPEWIRE_LIBRARIES}
        m
)

install(TARGETS radioform-filter
    RUNTIME DESTINATION bin
)
