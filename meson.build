project(
    'obmc-study',
    'cpp',
    default_options: ['warning_level=3', 'werror=true', 'cpp_std=c++23'],
    license: 'Apache-2.0',
    version: '0.1',
    meson_version: '>=1.1.1',
)

boost = dependency('boost', required: false)
sdbusplus = dependency('sdbusplus', include_type: 'system')
logging = dependency('phosphor-logging')

default_deps = [
    boost,
    sdbusplus,
    logging,
]

subdir('service_files')
subdir('src')
