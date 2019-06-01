# 23 march 2019

libui_test_sources = [
	'events.c',
	'initmain.c',
	'main.c',
	'noinitwrongthread.c',
]

if libui_OS == 'windows'
	libui_test_sources += [
		windows.compile_resources('resources_' + libui_mode + '.rc',
			args: libui_manifest_args,
			depend_files: ['test_' + libui_mode + '.manifest']),
	]
endif

libui_test_deps = []

subdir('lib')

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
