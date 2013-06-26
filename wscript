#!/usr/bin/env python
# enccoding: utf-8
# Steve Beaulac, 2013 (SjB)

import os
from waflib import Options

MAJOR = 0
MINOR = 1
PATCH = 1

VERSION="{0}.{1}.{2}".format(MAJOR, MINOR, PATCH)
APPNAME="hid-cli"

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

	ctx.add_option('--build_win32',
					dest='Win32Platform',
					action='store_true',
					default=False,
					help='build for win32 platform')

def configure(ctx):
	ctx.load('cs_extra', tooldir='waftools')

	ctx.env.VERSION = VERSION
	ctx.env.Win32Build = Options.options.Win32Platform

	if ctx.env.Win32Build:
		platform = 'i586-mingw32msvc-{0}'
		os.environ['CC'] = platform.format('cc')
		os.environ['CXX'] = platform.format('c++')

	ctx.load('compiler_c compiler_cxx')
	ctx.check_tool('c cxx')

	if Options.options.debug:
		ctx.env.CCFLAGS = ['-DDEBUG', '-ggdb3', '-Wall']
	else:
		ctx.env.CCFLAGS = ['-O2']

	ctx.env.default_app_install_path = '${PREFIX}/lib/%s' % APPNAME

def build(bld):
	projects = ['SjB.Hid', 'Test']

	if bld.env.Win32Build:
		projects.append('win32-dll')
	else:
		projects.append('src')


	bld.recurse(projects)

