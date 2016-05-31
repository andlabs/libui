# 30 may 2016

from waflib import Utils

top = '.'
out = '_build'

def _platform(ctx):
	plat = Utils.unversioned_sys_platform()
	if plat == 'win32':
		return 'windows'
	if plat == 'darwin':
		return 'darwin'
	return 'unix'

def _platsrcsuffix(plat):
	if plat == 'windows':
		return 'cpp'
	if plat == 'darwin':
		return 'm'
	return 'c'

def _plathsuffix(plat):
	if plat == 'windows':
		return 'hpp'
	return 'h'

def _toolchain(ctx):
	return ctx.env['CC_NAME']

def options(ctx):
	ctx.add_option('--toolchain', action='store', default=None, help='Toolchain to use, in waf terms; otheriwse compiler_c will determine')
	ctx.load('compiler_c')
	ctx.load('compiler_cxx')

def configure(ctx):
	if ctx.options.toolchain is not None:
		conf.options.check_c_compiler = [ctx.options.toolchain]
		conf.options.check_cxx_compiler = [ctx.options.toolchain]
	ctx.load('compiler_c')
	ctx.load('compiler_cxx')
	
	ctx.setenv('shared', env = ctx.env.derive())
	ctx.write_config_header('ignore/shared.h', remove = False)
	ctx.setenv('static', env = ctx.env.derive())
	ctx.write_config_header('ignore/static.h', remove = False)

def _common_flags(platform, toolchain, buildmode, gcclang, msvclang):
	if toolchain == "msvc":
		return list([
			msvclang,
		])
	l = list([
		"-Wall", "-Wextra", "-pedantic",
		"-Wno-unused-parameter",
		"-Wno-switch",
		gcclang,
	])
	if buildmode == "shared" and platform != "windows":
		l += ["-fPIC"]
	if platform == "darwin":
		l += [
			"-mmacosx-version-min=10.8",
			"-DMACOSX_DEPLOYMENT_TARGET=10.8",
		]
	return l

def _common_cflags(platform, toolchain, buildmode):
	return _common_flags(platform, toolchain, buildmode,
		'--std=c99', '-TC')

def _common_cxxflags(platform, toolchain, buildmode):
	return _common_flags(platform, toolchain, buildmode,
		'--std=c99', '-TP')

def _common_ldflags(platform, toolchain, buildmode):
	if toolchain == "msvc":
		return list([])
	l = list([])
	if buildmode == "shared" and platform != "windows":
		l += ["-fPIC"]
	if platform == "darwin":
		l += [
			"-mmacosx-version-min=10.8",
		]
	return l

def _libui_cflags(toolchain):
	if toolchain == "msvc":
		return list([
		])
	return list([
		"""-D_UI_EXTERN=__attribute__((visibility("default"))) extern""",
		"-fvisibility=hidden",
	])

def _libui_ldflags(toolchain):
	if toolchain == "msvc":		# no additional options
		return list([])
	return list([
		"-fvisibility=hidden",
	])

def _platform_libs(platform, toolchain):
	if toolchain == "msvc":
		return list([])
	if platform == "darwin":
		return list([
			"-lobjc",
			"-framework", "Foundation",
			"-framework", "AppKit",
		])
	if platform == "windows":
		return list([])
	return list([])

def _libui_sharedldflags(platform, toolchain):
	if toolchain == "msvc":
		return list([])
	if platform == "darwin":
		return list([
			"-install_name", "@rpath/",
		])
	if platform == "windows":
		return list([])
	return list([])

def _exe_sharedldflags(platform, toolchain):
	if toolchain == "msvc":
		return list([])
	if platform == "darwin":
		return list([
			# the / is needed by older OS X versions
			"-rpath", "@executable_path/",
		])
	if platform == "windows":
		return list([])
	return list([])

def _libui(ctx, platform, toolchain, buildmode):
	headers = ctx.path.ant_glob(
		"ui.h ui_%s.h %s/*.%s" %
			(platform, platform, _plathsuffix(platform)))
	sources = ctx.path.ant_glob(
		"common/*.c %s/*.%s" %
			(platform, _platsrcsuffix(platform)))
	cflags = _common_cflags(platform, toolchain, buildmode)
	cxxflags = _common_cxxflags(platform, toolchain, buildmode)
	ldflags = _common_ldflags(platform, toolchain, buildmode)
	cflags += _libui_cflags(toolchain)
	cxxflags += _libui_cflags(toolchain)
	ldflags += _libui_ldflags(toolchain)
	ldflags += _platform_libs(platform, toolchain)
	ldflags += _libui_sharedldflags(platform, toolchain)
	target = "ui"
	if platform == "windows":
		target = "libui"
	func = ctx.shlib
	if buildmode == "static":
		func = ctx.stlib
	func(
		target = target,
		source = sources,
		includes = headers,
		cflags = cflags,
		cxxflags = cxxflags,
		ldflags = ldflags,
	)

def build(ctx):
	platform = _platform(ctx)
	toolchain = _toolchain(ctx)
	buildmode = ctx.variant
	if not buildmode:
		ctx.fatal("Use either 'waf build_shared` or `waf build_static`")

	if platform == "windows" and toolchain != "msvc" and buildmode == "shared":
		ctx.fatal("Sorry, building a DLL on Windows with MinGW is not yet supported. Either build a static library or use MSVC.")
	
	_libui(ctx, platform, toolchain, buildmode)

# from https://github.com/waf-project/waf/blob/master/demos/variants/wscript
def init(ctx):
	from waflib.Build import BuildContext, CleanContext, InstallContext, UninstallContext
	for x in 'shared static'.split():
		for y in (BuildContext, CleanContext, InstallContext, UninstallContext):
			name = y.__name__.replace('Context','').lower()
			class tmp(y):
				cmd = name + '_' + x
				variant = x
	def buildall(ctx):
		import waflib.Options
		for x in ('build_shared', 'build_static'):
			waflib.Options.commands.insert(0, x)

# via https://github.com/waf-project/waf/blob/master/demos/mac_app/wscript
from waflib import TaskGen
@TaskGen.extension('.m')
def m_hook(self, node):
	return self.create_compiled_task('c', node)
