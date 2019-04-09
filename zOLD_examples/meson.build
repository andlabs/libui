# 24 march 2019

libui_example_sources = []
libui_example_link_args = []
libui_example_cpp_extra_args = []
if libui_OS == 'windows'
	libui_example_manifest = 'example.manifest'
	if libui_mode == 'static'
		libui_example_manifest = 'example.static.manifest'
	endif
	libui_example_sources += [
		windows.compile_resources('resources.rc',
			args: libui_manifest_args,
			depend_files: [libui_example_manifest]),
	]
	# because Microsoft's toolchain is dumb
	if libui_MSVC
		libui_example_link_args += ['/ENTRY:mainCRTStartup']
	endif
elif libui_OS == 'darwin'
	# since we use a deployment target of 10.8, the non-C++11-compliant libstdc++ is chosen by default; we need C++11
	# see issue #302 for more details
	libui_example_cpp_extra_args += ['--stdlib=libc++']
endif

libui_examples = {
	'controlgallery': {
		'sources':		['controlgallery/main.c'],
	},
	'histogram': {
		'sources':		['histogram/main.c'],
	},
	'cpp-multithread': {
		'sources':		['cpp-multithread/main.cpp'],
		'deps':		[
			dependency('threads',
				required: true),
		],
		'cpp_args':	libui_example_cpp_extra_args,
		'link_args':	libui_example_cpp_extra_args,
	},
	'drawtext': {
		'sources':		['drawtext/main.c'],
	},
	'timer': {
		'sources':		['timer/main.c'],
	},
	'datetime': {
		'sources':		['datetime/main.c'],
	},
}
foreach name, args : libui_examples
	# TODO once we upgrade to 0.49.0, add pie: true
	executable(name, args['sources'] + libui_example_sources,
		dependencies: args.get('deps', []) + libui_binary_deps,
		link_with: libui_libui,
		cpp_args: args.get('cpp_args', []),
		link_args: args.get('link_args', []) + libui_example_link_args,
		gui_app: false,
		install: false)
endforeach
