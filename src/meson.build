installdir = join_paths(get_option('libexecdir'), 'obmc-study')

utils_lib = static_library(
    'utils',
    'utils.cpp',
    dependencies: [default_deps],
)

src_inc = include_directories('.')

subdir('boost-study')
subdir('sd-socket')

subdir('test')
