#!/usr/bin/python
import os
import sys

# how this should work:
# read basedir/CVS/Entries
#  each directory has a D/aaa at the beginning, each file a /bbb
#  recursivly subdivide into basedir/aaa
#  add basedir/bbb to return list
#  returns list of files in cvs
def findCVS(basedir):
	f = open(basedir + os.sep + 'CVS' + os.sep + 'Entries')
	lines = f.readlines()
	mydirs = []
	myfiles = []
	for l in lines:
		if l.startswith('D/'):
			l2 = l[2:]
			i = l2.find('/')
			if i != -1:
				l3 = l2[0:i]
				mydirs += [l3]
		elif l.startswith('/'):
			l2 = l[1:]
			i = l2.find('/')
			if i != -1:
				l3 = l2[0:i]
				myfiles += [basedir + os.sep + l3]
#	print mydirs
#	print myfiles
	for d in mydirs:
		nextdir = basedir + os.sep + d
		if os.path.isdir(nextdir):
			myfiles += findCVS(nextdir)
	return myfiles

allfiles = findCVS('.')

f = open('files.txt', 'w')
for i in allfiles:
	f.write(i + '\n')
f.close()

# fixme: add version to name.
# fixme: store data in subdirectory in zip file? yes...
# name it dangerdeep-data-version
# can be easily created with links...
# find all directories...
alldatafiles = [x for x in allfiles if (x.startswith('./data') and not(x.startswith('./data/xcf')))]
alldirs = []
for x in alldatafiles:
	# remove ./data/ from head
	y = x[7:]
	posofslash = y.rfind('/')
	if posofslash >= 0:
		# is a dir
		alldirs.append(y[:posofslash])
alldirs.sort()
uniq = {}
for i in alldirs:
	uniq[i] = 1
alldirs = uniq.keys()
f = open('version.txt', 'r')
version = f.readlines()[0][:-1]
f.close()
databasedir = 'dangerdeep-data-' + version
os.system('rm -rf ' + databasedir)
os.system('mkdir -p ' + databasedir + '/data')
os.system('rm -rf ' + databasedir + '.zip')
pwd = os.getcwd()
for x in alldirs:
	os.system('mkdir -p ' + databasedir + '/data/' + x)
for x in alldatafiles:
	os.system('ln -s "' + pwd + '/' + x[2:] + '" "' + databasedir + '/data/' + x[7:] + '"')
os.system('cd ' + databasedir + ' && zip -r ' + databasedir + '.zip data > /dev/null && mv ' + databasedir + '.zip ..')
os.system('rm -rf ' + databasedir)
