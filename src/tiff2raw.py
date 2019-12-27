#!/usr/bin/python

import Image
import sys
import array

im = Image.open(sys.argv[1])
print im.format, im.size, im.mode

if im.format != 'TIFF':
    print 'No TIFF!'
    sys.exit(-1)
if im.mode != 'I;16':
    print 'No 16bpc greyscale tiff!'
    sys.exit(-1)

print 'OK'
l = []
for y in range(0, im.size[1]):
    for x in range(0, im.size[0]):
        l.append(im.getdata()[y*im.size[0]+x])
f = open(sys.argv[1] + '.raw', 'wb')
outval = array.array('H')
outval.fromlist(l)
outval.tofile(f)

#for y in range(0, im.size[1]):
#    s = ''
#    for x in range(0, im.size[0]):
#        s += hex(im.getdata()[y*im.size[0]+x]) + ', '
#    s = s[:-2] + '\n'
#    print s

