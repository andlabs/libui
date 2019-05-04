# 23 march 2019

libui_test_sources = [
	'initmain.c',
	'main.c',
]

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
