if not get_option('sd-socket').allowed()
    subdir_done()
endif

executable(
    'sd-socket',
    'main.cpp',
    'socket.cpp',
    dependencies: [default_deps, conf_h_dep],
    include_directories: src_inc,
    link_with: utils_lib,
    install: true,
    install_dir: installdir,
)
