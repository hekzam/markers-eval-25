project('hekzam-marqueurs', 'cpp',
        version : '0.1.0',
        default_options : ['cpp_std=c++17'])

opencv_dep = dependency('opencv4')
json_dep = dependency('nlohmann_json')
zbar_dep = dependency('zbar')
zxing_dep = dependency('zxing')

deps = [opencv_dep, json_dep, zbar_dep, zxing_dep]

inc_dirs = include_directories(
    'src',
    'src/utils',
    'src/parser',
    'src/external-tools',
    'include'
)
src_files = [
    'src/expl_pars.cpp',
    'src/external-tools/create_copy.cpp'
]

src_bench_files = [
    'src/benchmark.cpp',
    'src/utils/benchmark_helper.cpp',
    'src/external-tools/create_copy.cpp',
    'src/external-tools/modifier.cpp'
]

src_modifieur_files = [
    'src/modifier.cpp',
    'src/utils/math_utils.cpp',
    'src/external-tools/modifier.cpp'
]

src_typst_interface_files = [
    'src/typst_interface.cpp',
    'src/external-tools/create_copy.cpp'
]

benchmark_src = [
    'src/bench/gen_parse.cpp',
    'src/bench/config_analysis.cpp'
]

utils_src = run_command('find', 'src/utils', '-name', '*.cpp', '-o', '-name', '*.h', check: true).stdout().strip().split('\n')

parser_src = run_command('find', 'src/parser', '-name', '*.cpp', '-o', '-name', '*.h', check: true).stdout().strip().split('\n')


executable('parser', 
           src_files + utils_src + parser_src,
           include_directories : inc_dirs,
           dependencies : deps)

executable('bench',
           src_bench_files + utils_src + parser_src + benchmark_src,
           include_directories : inc_dirs,
           dependencies : deps)

executable('create-copy',
           src_typst_interface_files,
           include_directories : inc_dirs,
           dependencies : deps)

executable('modifier',
           src_modifieur_files,
           include_directories : inc_dirs,
           dependencies : deps)