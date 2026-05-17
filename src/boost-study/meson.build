if not get_option('boost-study').allowed()
    subdir_done()
endif

executable(
    'boost-study',
    'BoostStudyMain.cpp',
    dependencies: [default_deps],
    include_directories: src_inc,
    link_with: utils_lib,
    install: true,
    install_dir: installdir,
)
