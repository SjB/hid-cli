#!/usr/bin/env python
# encoding: utf-8
# Copyright © 2012 SjB <steve@nca.uwo.ca>. All Rights Reserved.

from waflib.Build import BuildContext
class NUnitContext(BuildContext):
    cmd = 'nunit'
    fun = 'build'
