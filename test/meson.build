# 19 january 2020

# Using files() is the cleanest way to ensure the python script below gets the right filenames regardless of how meson sandboxes the command it's running.
libui_test_sources = files([
	'controls.c',
	'initmain.c',
	'noinitwrongthread.c',
	'window.c',
])
libui_allcalls_headers = files([
	'allcalls.h',
])
if libui_OS == 'windows'
	libui_test_sources += files([
		'controls_windows.c',
		'noinitwrongthread_windows.c',
		'window_windows.c',
	])
	libui_allcalls_headers += files([
		'allcalls_windows.h',
	])
elif libui_OS == 'darwin'
	libui_test_sources += files([
		'controls_darwin.m',
		'noinitwrongthread_darwin.m',
		'window_darwin.m',
	])
	libui_allcalls_headers += files([
		'allcalls_darwin.h',
	])
elif libui_OS == 'haiku'
	libui_test_sources += files([
		'controls_haiku.cpp',
		'noinitwrongthread_haiku.cpp',
		'window_haiku.cpp',
	])
	libui_allcalls_headers += files([
		'allcalls_haiku.hpp',
	])
else
	libui_test_sources += files([
		'controls_unix.c',
		'noinitwrongthread_unix.c',
		'window_unix.c',
	])
	libui_allcalls_headers += files([
		'allcalls_unix.h',
	])
endif

libui_test_sources_without_cases = [
	'errors.c',
	'main.c',
	'utf8.c',
]
if libui_OS == 'windows'
	libui_test_sources_without_cases += [
		'thread_windows.c',
		'utf8_windows.c',
		windows.compile_resources('resources_' + libui_mode + '.rc',
			args: libui_manifest_args,
			depend_files: ['test_' + libui_mode + '.manifest']),
	]
else
	libui_test_sources_without_cases += [
		'thread_notwindows.c',
	]
endif

libui_test_deps = [
	dependency('threads',
		required: true),
]
# TODO deduplicate these
if libui_OS == 'windows'
	# static mode already gives us these dependencies
	if libui_mode != 'static'
		libui_test_deps += [
			meson.get_compiler('c').find_library('kernel32',
				required: true),
			meson.get_compiler('c').find_library('user32',
				required: true),
		]
	endif
elif libui_OS == 'darwin'
	# static mode already gives us these dependencies
	if libui_mode != 'static'
		libui_test_deps += [
			meson.get_compiler('objc').find_library('objc',
				required: true),
			dependency('appleframeworks',
				modules: ['Foundation', 'AppKit'],
				required: true),
		]
	endif
elif libui_OS == 'haiku'
	# static mode already gives us these dependencies
	if libui_mode != 'static'
		libui_test_deps += [
			meson.get_compiler('cpp').find_library('root',
				required: true),
			meson.get_compiler('cpp').find_library('be',
				required: true),
		]
	endif
else
	# static mode already gives us these dependencies
	if libui_mode != 'static'
		libui_test_deps += [
			dependency('gtk+-3.0',
				version: '>=3.10.0',
				method: 'pkg-config',
				required: true),
		]
	endif
endif

pymod = import('python')
python = pymod.find_installation()
# Likewise, use files() here to make sure the Python script itself will always have the right path.
libui_testlist_py = files(['testlist.py'])

libui_testlist_h = custom_target(
	'testlist.h',
	input: libui_test_sources + libui_allcalls_headers,
	output: ['testlist.h'],
	command: [python, libui_testlist_py, 'header', '@OUTPUT@', '@INPUT@'])
libui_testlist_c = custom_target(
	'testlist.c',
	input: libui_test_sources + libui_allcalls_headers,
	output: ['testlist.c'],
	command: [python, libui_testlist_py, 'source', '@OUTPUT@', '@INPUT@'])

# TODO once we upgrade to 0.49.0, add pie: true
# TODO once we upgrade to 0.50.0, add protocol: 'exitcode'
libui_testrunner = executable('testrunner', libui_test_sources + libui_test_sources_without_cases + [libui_testlist_c, libui_testlist_h],
	dependencies: libui_binary_deps + libui_test_deps,
	link_with: libui_libui,
	gui_app: false,
	install: false)

runresult = run_command(python, libui_testlist_py + ['list'] + libui_test_sources + libui_allcalls_headers)
if runresult.returncode() != 0
	error('testlist.py failed; cannot compute list of test cases. Exit code @0@; stderr:\n@1@'.format(runresult.returncode(), runresult.stderr()))
endif
foreach casename : runresult.stdout().split()
	test(casename, libui_testrunner,
		args: [casename],
		is_parallel: false,
		should_fail: false)
endforeach
