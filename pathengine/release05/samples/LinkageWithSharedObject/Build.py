#!/usr/bin/env python
#**********************************************************************
#
# Copyright (c) 2007
# PathEngine
# Lyon, France
#
# All Rights Reserved
#
#**********************************************************************

import os

def printOutputFromPipe(pipe):
	while 1:
		line = pipe.readline()
		if not line:
			break
		os.write(1, line)    

def doCommand(command):
	print command
	pipe = os.popen(command)
	printOutputFromPipe(pipe)

compileCommand = 'g++'
compileCommand += ' -I ' + os.path.join('..', '..', 'interface')
compileCommand += ' -o ' + 'Main.o'
compileCommand += ' -c ' + 'Main.cpp'
doCommand(compileCommand)

linkCommand = 'g++'
linkCommand += ' -o LinkageWithSharedObject.exe'
linkCommand += ' -ldl' # may need to be removed on some platforms
linkCommand += ' Main.o'
doCommand(linkCommand)
