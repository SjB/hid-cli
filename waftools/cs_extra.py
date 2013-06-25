#!/usr/bin/env python
# encoding: utf-8
# Copyright © 2012 SjB <steve@nca.uwo.ca>. All Rights Reserved.

import os, shutil

from waflib.TaskGen import feature, extension, after, before, taskgen_method
from waflib.Task import Task
from waflib.Configure import conf
from waflib.Tools import ccroot
from waflib import Utils, Options, Context, Errors

#
# waf tools to extend the build-in cs tool provided with the default waf build system.
#

def options(ctx):
    ctx.load('cs')
    ctx.add_option('--sdk', type='string', dest='sdk_version', help='Specifies SDK version of referenced assemlies')
    ctx.add_option('--with-resgen-binary', type='string', dest='resgenbinary', help='localtion of the resgen binary')
    ctx.add_option('--package-dep', dest='package_dep_lib', action='store_true', default=False, help='Package all dependent library with project')

def configure(ctx):
    csc = getattr(Options.options, 'cscbinary', None)
    if csc:
        conf.env.MCS = csc
    else:
        # Added the dmcs mono compile to the default list.
        # Users  can define their own list by assigning an list of compiler
        # to the ctx.cscbinary variable
        cscbinary = getattr(ctx, 'cscbinary', ['csc', 'dmcs', 'gmcs', 'mcs'])
        ctx.find_program(cscbinary, var='MCS')

    resgen = getattr(Options.options, 'resgenbinary', None)
    if resgen:
        ctx.env.RESGEN = resgen
    else:
        ctx.find_program(['resgen'], var='RESGEN')

    ctx.env.package_dep_lib = getattr(Options.options, 'package_dep_lib', False)

    ctx.load('cs')
    ccroot.lib_patterns['csshlib'] = ['%s.dll']

    # new variable that allow the sdk version to be specified at the command line.
    sdk_version = getattr(Options.options, 'sdk_version', None)
    if sdk_version:
        self.env.append_value('CSFLAGS', '/sdk:%s' % sdk_version)




# converts the *.cs.in source files into *.cs files replacing all @var@ with
# the appropriate values in the related variable in the ctx.Define array
@extension('.cs.in')
def process_in(self, node):
    for x in self.env['DEFINES']:
        (k, v) = x.split('=')
        setattr(self.cs_task.generator, k, v)

    tgt = node.change_ext('.cs', ext_in='.cs.in').path_from(self.bld.bldnode)
    tsk = self.create_task('subst', node, tgt)


class resources(Task):
    inst_to = None
    run_str = '${RESGEN} ${SRC} ${TGT}'


@extension('.resx')
def process_resx(self, node):
    tsk = self.create_task('resources', node, node.change_ext('.resources', ext_in='.resx'))


# Make sure we assign a value to name. (this should be included in the original tool)
@feature('cs')
@before('apply_cs')
def fix_assign_gen_to_name(self):
    name = getattr(self, 'name', None) or self.gen
    setattr(self, 'name', name)


@feature('cs')
@after('apply_cs')
@before('use_cs')
def pkg_cs(self):
    names = self.to_list(getattr(self, 'use', []))
    use = names[:]

    for x in names:
        pkg = 'PKG_%s' % Utils.quote_define_name(x)
        if  pkg in getattr(self.env, 'packages', []):
            self.env.append_value('CSFLAGS', '/pkg:%s' % x)
            use.remove(x)

    self.use = ' '.join(use)


@feature('cs')
@after('apply_cs')
@before('use_cs')
def use_extlib(self):
    names = self.to_list(getattr(self, 'use', []))
    use = names[:]

    for x in names:
        lib_name = Utils.quote_define_name(x)

        for lib_type in ccroot.lib_patterns.keys():
            if lib_name in getattr(self.env, 'ext_%s' % lib_type, []):
                name = getattr(self.env, '%s_NAME' % lib_name, None)

                # replace uselib with real assembly name
                if name:
                    use.remove(x)
                    use.append(name);

    self.use = ' '.join(use)


# Add define params to the compile command line
@conf
def set_define(self, *k, **kw):
    if k[0]:
        kw['defines'] = k[0]

    kw['defines'] = Utils.to_list(kw['defines'])
    if len(kw['defines']):
        self.env.append_value('CSFLAGS', '/define:%s' % ';'.join(kw['defines']))


# Set the sdk version
@conf
def set_sdk_version(self, *k, **kw):
    if k:
        kw['sdk_version'] = k[0]

    if 'sdk_version' in kw:
        v = kw['sdk_version']
        self.msg("Setting .Net SDK version", v)
        self.env.append_value('CSFLAGS', '/sdk:%s' % v)


# check if a pkg-config package in install on the system
@conf
def check_pkg(self, *k, **kw):
    self.check_cfg(**kw)
    if self.get_define(self.have_define(kw['package'])):
        self.env.append_value('packages', 'PKG_%s' % Utils.quote_define_name(kw['package']))


# check if an external lib is available to the compiler.
@conf
def check_extlib(self, *k, **kw):

    env = self.env

    if not 'package' in kw:
        kw['package'] = k[0]

    if not 'msg' in kw:
        kw['msg'] = 'Checking for %s' % kw['package']

    paths = ['.']
    if "path_list" in kw:
        for p in Utils.to_list(kw['path_list']):
            if p:
                paths.append(p)

    if not 'lib_type' in kw:
        kw['lib_type'] = 'shlib'

    names = [x % kw['package'] for x in ccroot.lib_patterns[kw['lib_type']]]
    ret = self.find_file(names, path_list=paths)

    if ret:
        uselib = Utils.quote_define_name(kw.get('uselib_store', kw['package']))

        env.append_value('ext_%s' % kw['lib_type'], uselib)
        setattr(env, '%s_NAME' % uselib, os.path.basename(ret))
        setattr(env, '%s_LIBPATH' % uselib, os.path.dirname(ret))

        self.define(self.have_define(kw.get('uselib_store', kw['package'])), 1, 0)

    self.msg(kw['msg'], ret, "GREEN")


@conf
def read_assembly(self, assembly, install_path = None):
    """
    Read a foreign .net assembly that was validated via check_assembl
    """
    env = self.env
    uselib = Utils.quote_define_name(assembly)

    if not uselib in getattr(env, 'ext_csshlib', []):
        self.fatal('Assembly %s not registered as a C sharp assembly' % assembly)

    name = getattr(env, '%s_NAME' % uselib, None)
    path = getattr(env, '%s_LIBPATH' % uselib, None)

    tg = self(name=name,
              features='fake_lib',
              lib_paths=[path],
              lib_type='csshlib')

    if install_path:
        d = self.root.find_node(path)
        if not d:
            self.fatal('Can\'t find assembly path')
        f = d.find_node(name)
        self.install_files(install_path, f)

    return tg


#check if an external assembly is available to the compiler
@conf
def check_assembly(self, *k, **kw):
    kw['lib_type'] = 'csshlib'
    check_extlib(self, *k, **kw)


# Simple Copy file Task
class copy_file(Task):
    chmod = Utils.O644
    inst_to = None

    def run(self):
        infile = self.inputs[0].abspath()
        outfile = self.outputs[0].abspath()
        try:
            shutil.copy2(infile, outfile)
        except (OSError, IOError):
            return 1
        else:
            if self.chmod: os.chmod(outfile, self.chmod)
            return 0


# Copy all external (USE) library in the the build directory. This will allow use to
# run the codes from within that build directory.
@feature('cs_dev')
@after('use_cs')
def copy_dependent_library(self):
    names = self.to_list(getattr(self, 'use', []))

    for x in names:
        tgen = self.bld.get_tgen_by_name(x)
        for tsk in tgen.tasks:
            lib = tsk.outputs[0]
            copy_lib(self, lib)
            copy_config(self, lib)

def copy_lib(tgen, target):
    out = tgen.path.find_or_declare(target.name)
    tgen.copy_dependent_lib_task = tgen.create_task('copy_file', target, out)

def copy_config(tgen, target):
    config = target.change_ext('.dll.config');
    if os.path.isfile(config.abspath()):
        out = tgen.path.find_or_declare(config.name)
        tgen.copy_dependent_lib_config_task = tgen.create_task('copy_file', config, out)
