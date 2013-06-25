#!/usr/bin/env python
# enccoding: utf-8
# Steve Beaulac, 2013 (SjB)

MAJOR = 0
MINOR = 1
PATCH = 1

VERSION="{0}.{1}.{2}".format(MAJOR, MINOR, PATCH)
APPNAME="hid-cli"

top = '.'
out = 'build'

def options(ctx):
	ctx.load('cs_extra', tooldir='waftools')

	ctx.tool_options('compiler_cc')
	ctx.add_option('--build_kind', action='store', default='debug,release', help='build the selected variants')

def configure(ctx):
	ctx.load('cs_extra', tooldir='waftools')

	ctx.env.VERSION = VERSION
	ctx.check_tool('compiler_c c')

	dbg = ctx.env.copy()
	rel = ctx.env.copy()

	dbg.set_variant('debug')
	ctx.set_env_name('debug', dbg)
	ctx.setenv('debug')
	ctx.env.CCFLAGS = ['-DDEBUG', '-ggdb3', '-Wall']
	ctx.define_cond('DEBUG', True)

	rel.set_variant('release')
	ctx.set_env_name('release', rel)
	ctx.setenv('release')
	ctx.env.CCFLAGS = ['-O2']

	ctx.env.default_app_install_path = '${PREFIX}/lib/%s' % APPNAME

def build(bld):
	bld.recurse([
		'src',
		'SjB.Hid',
		'Test'
	])

