systemd = dependency('systemd')
systemd_system_unit_dir = systemd.get_variable(
    'systemd_system_unit_dir',
    pkgconfig_define: ['prefix', get_option('prefix')],
)

unit_files = [
    ['boost-study','xyz.openbmc_project.boost-study.service']
]

fs = import('fs')
foreach tuple : unit_files
    if get_option(tuple[0]).allowed()
        fs.copyfile(
            tuple[1],
            install: true,
            install_dir: systemd_system_unit_dir,
        )
    endif
endforeach