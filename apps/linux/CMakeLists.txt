cmake_minimum_required(VERSION 3.20)
project(radioform-ui VERSION 1.0.0 LANGUAGES C CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(AYATANA REQUIRED ayatana-appindicator3-0.1)
pkg_check_modules(JSONC REQUIRED json-c)

add_executable(radioform-ui
    main.c
    eq_window.c
    preset_manager.c
    control_client.c
)

target_include_directories(radioform-ui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(radioform-ui PRIVATE ${DSP_SOURCE_DIR}/include)

target_link_libraries(radioform-ui PRIVATE
    radioform_dsp
    ${GTK3_LIBRARIES}
    ${AYATANA_LIBRARIES}
    ${JSONC_LIBRARIES}
    m
)

target_compile_options(radioform-ui PRIVATE
    ${GTK3_CFLAGS}
    ${AYATANA_CFLAGS}
    ${JSONC_CFLAGS}
)

install(TARGETS radioform-ui RUNTIME DESTINATION bin)
