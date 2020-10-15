from building import *
Import('rtconfig')

cwd     = GetCurrentDir()
src	= Glob('*.c')
path = [cwd]

group = DefineGroup('sm9541', src, depend = ['PKG_USING_SM95_SENSOR'], CPPPATH = path)

Return('group')
