#!/usr/bin/env python
# encoding: utf-8
# Copyright (c) 2013 SjB <steve@sagacity.ca>. All Rights Reserved.

#
# Waf script to comfigure and use EtoForm

import os, sys
from waflib import Options, Utils
from waflib.Configure import conf

def options(ctx):
	ctx.load('cs_extra', tooldir='waftools')
	ctx.add_option('--with-EtoForms', type='string', dest='etoform_dir',
                    help='Specify the path where the EtoForms library are located')


@conf
def check_etoform(self, *k, **kw):

	etoform_dir = Options.options.etoform_dir
	if not etoform_dir:
		etoform_dir = os.environ.get('ETO_DIR', './libs')

	etoform_dir = [etoform_dir]
	if 'path_list' in kw:
		etoform_dir.extend(Utils.to_list(kw['path_list']))

	self.check_assembly('Eto', path_list = etoform_dir)

	os_platform = Utils.unversioned_sys_platform()
	uselib_etoplatform = 'Eto.Platform'

	if 'linux' == os_platform:
		self.check_assembly('Eto.Platform.Gtk', path_list = etoform_dir, uselib_store=uselib_etoplatform)
	elif 'win32' == os_platform:
		self.check_assembly('Eto.Platform.Windows', path_list = etoform_dir, uselib_store=uselib_etoplatform)
	else:
		self.fatal('Platform not supported')

@conf
def read_etoform(self, install_path = None):
	self.read_assembly('Eto', install_path)
	self.read_assembly('Eto.Platform', install_path)

