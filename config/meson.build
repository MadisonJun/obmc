conf_data = configuration_data()
conf_data.set('max_events', get_option('max_events'))
conf_data.set('sd_socket_port', get_option('sd_socket_port'))

usetcp = get_option('usetcp')
conf_data.set10('usetcp', usetcp)

useudp = get_option('useudp')
conf_data.set10('useudp', useudp)

conf_h_dep = declare_dependency(
    include_directories: include_directories('.'),
    sources: configure_file(
        input: 'config.h.meson',
        output: 'config.h',
        configuration: conf_data,
    ),
)