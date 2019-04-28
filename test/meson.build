# 23 march 2019

libui_test_sources = [
	'initmain.c',
	'main.c',
	'testing.c',
]

if libui_OS == 'windows'
	libui_test_sources += ['testing_windows.c']
elif libui_OS == 'darwin'
	libui_test_sources += [
		'testing_darwin.c',
		'testing_darwinunix.c',
	]
else
	libui_test_sources += [
		'testing_darwinunix.c',
		'testing_unix.c',
	]
endif

libui_test_deps = []
if libui_OS == 'windows'
	libui_test_manifest = 'test.manifest'
	if libui_mode == 'static'
		libui_test_manifest = 'test.static.manifest'
	endif
	libui_test_sources += [
		windows.compile_resources('resources.rc',
			args: libui_manifest_args,
			depend_files: [libui_test_manifest]),
	]
	if libui_mode != 'static'
		libui_test_deps += [
			# for QueryPerformanceCounter()/QueryPerformanceFrequency()
			meson.get_compiler('c').find_library('kernel32',
				required: true),
		]
	endif
endif

# TODO once we upgrade to 0.49.0, add pie: true
# TODO once we upgrade to 0.50.0, add protocol: 'exitcode'
libui_tester = executable('tester', libui_test_sources,
	dependencies: libui_binary_deps + libui_test_deps,
	link_with: libui_libui,
	gui_app: false,
	install: false)
test('test', libui_tester,
	is_parallel: false,
	should_fail: false)
