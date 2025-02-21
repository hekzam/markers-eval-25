project('hekzam-parser', 'cpp',
  version: '0.1.0',
  license: 'Apache-2.0',
  meson_version: '>=1.2.3',
  default_options : ['cpp_std=c++17']
)

opencv_dep = dependency('opencv4')
zxing_dep = dependency('zxing')
nlohmann_json_dep = dependency('nlohmann_json')
deps = [
  opencv_dep,
  zxing_dep,
  nlohmann_json_dep,
]

src = [
  'src/main.cpp',
]

parser = executable(
  'parser',
  src,
  dependencies: deps,
  install: true
)
