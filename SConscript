# for module compiling
import os
Import('RTT_ROOT')
from building import *

cwd = GetCurrentDir()
objs = []
list = os.listdir(cwd)

for d in list:
    path = os.path.join(cwd, d)
    if os.path.isfile(os.path.join(path, 'SConscript')):
        print('scons: ' + os.path.join(d, 'SConscript'))
        objs = objs + SConscript(os.path.join(d, 'SConscript'))
# objs = objs + SConscript('applications\SConscript')
# objs = objs + SConscript('packages\SConscript')
# objs = objs + SConscript('board\SConscript')
# objs = objs + SConscript('libraries\SConscript')

Return('objs')
