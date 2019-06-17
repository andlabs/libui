# 23 march 2019

libui_test_sources = [
	'controls.c',
	'controls_errors.cpp',
	'errors.c',
	'events.c',
	'events_errors.cpp',
	'initmain.c',
	'main.c',
	'noinitwrongthread.cpp',
]

if libui_OS == 'windows'
	libui_test_sources += [
		'controls_windows.c',
	]
elif libui_OS == 'darwin'
	libui_test_sources += [
		'controls_darwin.m',
		'controls_darwin_errors.m',
	]
else
	libui_test_sources += [
		'controls_unix.c',
	]
endif

libui_test_cpp_extra_args = []
if libui_OS == 'windows'
	libui_test_sources += [
		windows.compile_resources('resources_' + libui_mode + '.rc',
			args: libui_manifest_args,
			depend_files: ['test_' + libui_mode + '.manifest']),
	]
elif libui_OS == 'darwin'
	# since we use a deployment target of 10.8, the non-C++11-compliant libstdc++ is chosen by default; we need C++11
	# see issue #302 for more details
	libui_test_cpp_extra_args += ['--stdlib=libc++']
endif

libui_test_deps = []

subdir('lib')

# TODO once we upgrade to 0.49.0, add pie: true
# TODO once we upgrade to 0.50.0, add protocol: 'exitcode'
libui_tester = executable('tester', libui_test_sources,
	dependencies: libui_binary_deps + libui_test_deps,
	link_with: libui_libui,
	cpp_args: libui_test_cpp_extra_args,
	link_args: libui_test_cpp_extra_args,
	gui_app: false,
	install: false)
test('test', libui_tester,
	is_parallel: false,
	should_fail: false)
