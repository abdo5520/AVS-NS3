#! /usr/bin/env python
# encoding: utf-8
# WARNING! Do not edit! http://waf.googlecode.com/svn/docs/wafbook/single.html#_obtaining_the_waf_file

import waflib.Tools.asm
from waflib.Tools import ar
def configure(conf):
	conf.find_program(['gas','as','gcc'],var='AS')
	conf.env.AS_TGT_F='-o'
	conf.find_ar()
