#!/usr/bin/env python
# enccoding: utf-8
# Steve Beaulac, 2013 (SjB)

import os
from waflib import Options, Utils

MAJOR = 0
MINOR = 1
PATCH = 1

VERSION="{0}.{1}.{2}".format(MAJOR, MINOR, PATCH)
APPNAME="SjB.Hid"

top = '.'
out = 'build'

def options(ctx):
	ctx.load('cs_extra', tooldir='waftools')

	ctx.tool_options('compiler_cc compiler_cxx c cxx')

	ctx.add_option('--debug', '-d',
                dest='debug',
                action='store_true',
                default=False,
                help='Enable debug build')

	ctx.add_option('--crosscompile-win32',
					dest='crosscompile_win32',
					action='store_true',
					default=False,
					help='crosscompile win32 code')

def configure(ctx):
	ctx.load('cs_extra', tooldir='waftools')

	ctx.env.VERSION = VERSION
	ctx.env.build_win32 = False

	ctx.env.build_win32 = ('win32' == Utils.unversioned_sys_platform())

	if Options.options.crosscompile_win32 and not ctx.env.build_win32:
		platform = 'i586-mingw32msvc-{0}'
		os.environ['CC'] = platform.format('cc')
		os.environ['CXX'] = platform.format('c++')
		ctx.env.build_win32 = True;

	ctx.load('compiler_c compiler_cxx')
	ctx.check_tool('c cxx')

	if Options.options.debug:
		ctx.env.CXXFLAGS = ctx.env.CFLAGS = ['-DDEBUG', '-g', '-Wall']
	else:
		ctx.env.CXXFLAGS = ctx.env.CFLAGS = ['-O2']

	ctx.env.default_app_install_path = '${PREFIX}/lib/%s' % APPNAME

def build(bld):
	projects = ['SjB.Hid', 'Test']

	if bld.env.build_win32:
		projects.append('win32-dll')
	else:
		projects.append('src')

	bld.recurse(projects)

