# 19 january 2020

libui_test_sources = [
	'initmain.c',
	'main.c',
]

libui_test_sources_without_cases = []
if libui_OS == 'windows'
	libui_test_sources_without_cases += [
		windows.compile_resources('resources_' + libui_mode + '.rc',
			args: libui_manifest_args,
			depend_files: ['test_' + libui_mode + '.manifest']),
	]
endif

# TODO once we upgrade to 0.49.0, add pie: true
# TODO once we upgrade to 0.50.0, add protocol: 'exitcode'
libui_testparent = executable('testparent', libui_test_sources + libui_test_sources_without_cases,
	dependencies: libui_binary_deps,
	link_with: libui_libui,
	gui_app: false,
	install: false)

pymod = import('python')
python = pymod.find_installation()
# Using files() is the cleanest way to ensure the python script gets the right filenames regardless of how meson sandboxes the command it's running.
runresult = run_command(python, files(['testlist.py'] + libui_test_sources))
if runresult.returncode() != 0
	error('testlist.py failed; cannot compute list of test cases. Exit code @0@; stderr:\n@1@'.format(runresult.returncode(), runresult.stderr()))
endif
# TODO make sure this works properly on Windows
foreach casename : runresult.stdout().split()
	test(casename, libui_testparent,
		args: [casename],
		is_parallel: false,
		should_fail: false)
endforeach
