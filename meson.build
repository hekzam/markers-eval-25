project('hekzam-marqueurs', 'cpp',
        version : '0.1.0',
        default_options : ['cpp_std=c++17'])

# Dépendances
opencv_dep = dependency('opencv4')
json_dep = dependency('nlohmann_json')
zbar_dep = dependency('zbar')
zxing_dep = dependency('zxing')

deps = [opencv_dep, json_dep, zbar_dep, zxing_dep]

# Include directories
inc_dirs = include_directories(
    'src',
    'src/utils',
    'src/parser',
    'src/external-tools',
    'include'
)
# Source files
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
    'src/create-copy.cpp',
    'src/external-tools/create_copy.cpp'
]

benchmark_src = [
    'src/bench/gen_parse.cpp',
    'src/bench/config_analysis.cpp'
]

utils_src = [
    'src/utils/benchmark_helper.cpp',
    'src/utils/benchmark_helper.h',
    'src/utils/cli_helper.cpp',
    'src/utils/cli_helper.h',
    'src/utils/csv_utils.h',
    'src/utils/draw_helper.cpp',
    'src/utils/draw_helper.h',
    'src/utils/json_helper.cpp',
    'src/utils/json_helper.h',
    'src/utils/math_utils.cpp',
    'src/utils/math_utils.h',
    'src/utils/parser_helper.cpp',
    'src/utils/parser_helper.h',
    'src/utils/string_helper.cpp',
    'src/utils/string_helper.h',
]

parser_src = [
    'src/parser/aruco_parser.cpp',
    'src/parser/aruco_parser.h',
    'src/parser/center_marker_parser.cpp',
    'src/parser/center_marker_parser.h',
    'src/parser/circle_parser.cpp',
    'src/parser/circle_parser.h',
    'src/parser/custom_marker_parser.cpp',
    'src/parser/custom_marker_parser.h',
    'src/parser/default_parser.cpp',
    'src/parser/default_parser.h',
    'src/parser/qrcode_empty_parser.cpp',
    'src/parser/qrcode_empty_parser.h',
    'src/parser/qrcode_parser.cpp',
    'src/parser/qrcode_parser.h',
    'src/parser/shape_parser.cpp',
    'src/parser/shape_parser.h',
]


# Create executables
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