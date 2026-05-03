if not get_option('boost-study').allowed()
    subdir_done()
endif

executable(
    'boost-study',
    'BoostStudyMain.cpp',
    dependencies: [default_deps],
    install: true,
    install_dir: installdir,
)
