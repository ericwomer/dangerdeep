#!/usr/bin/python
# transform ascii pgm to ddxml model

import sys

if (len(sys.argv) < 3) or (len(sys.argv) > 4):
    print 'Usage: ' + sys.argv[0] + ' inputfile.pgm outputfile.ddxml [texturefilename]'
    sys.exit()

f1 = open(sys.argv[1])
l1 = f1.readlines()
f1.close()
f2 = open(sys.argv[2], 'w')

texmap = ''
if (len(sys.argv) >= 4):
    texmap = sys.argv[3]

if l1[0][:-1] != 'P2':
    print 'No ascii PGM!'
    sys.exit()

if l1[1][0] != '#':
    print 'Save with gimp and do not modify it!'
    sys.exit()

# w/h
wh = l1[2].split()
w = int(wh[0])
h = int(wh[1])

print 'width=',w,'height=',h

# build output

l2 = ['<dftd-model version="1.1">\n']
if (texmap != ''):
    l2.append('\t<material name="default" id="0">\n')
    l2.append('\t\t<diffuse color="1 1 1" />\n')
    l2.append('\t\t<specular color="1 1 1" />\n')
    l2.append('\t\t<shininess exponent="60" />\n')
    l2.append('\t\t<map type="diffuse" filename="' + texmap + '" uscal="1" vscal="1" uoffset="0" voffset="0" angle="0" />')
    l2.append('\t</material>\n')
meshstr = '\t<mesh name="' + sys.argv[1] + '" id="0"'
if (texmap != ''):
    meshstr += ' material="0"'
meshstr += '>\n'
l2.append(meshstr)

nrv = w*h
nri = (w-1)*(h-1)*6
# per line (w-1) quads, so *2 tri's, plus 4 degenerated. add 2 start indices, remove 4 last degenerated.
ipl = (w-1)*2+4 # indices per line
nri2 = 2+(h-1)*ipl-4;
strv = '\t\t<vertices nr="' + str(nrv) + '">'
strt = '\t\t<texcoords>'
ptr = 4
heightmult = 1.0
xscal = 1.0
yscal = 1.0
for y in range(0, h):
    for x in range(0, w):
        ih = (int(l1[ptr][:-1]) - 128) * heightmult
        strv += str((x-w*0.5)*xscal) + ' ' + str((y-h*0.5)*yscal) + ' ' + str(ih) + ' '
        strt += str(float(x)/(w-1)) + ' ' + str(float(y)/(h-1)) + ' '
        ptr += 1
strv += '</vertices>\n'
strt += '</texcoords>\n'
stri = '\t\t<indices nr="' + str(nri) + '">'
stri2 = '\t\t<indices type="triangle_strip" nr="' + str(nri2) + '">' + str(w) + ' 0 '
left_to_right = True
for y in range(0, h-1):
    for x in range(0, w-1):
        b = y*w+x
        stri += str(b) + ' ' + str(b+1) + ' ' + str(b+w) + ' ' + str(b+w) + ' ' + str(b+1) + ' ' + str(b+w+1) + ' '
    if left_to_right:
        for x in range(1, w):
            stri2 += str(x + (y+1)*w) + ' ' + str(x + y*w) + ' '
        # append degenerated
        if y + 1 < h:
            stri2 += str(w-1 + (y+1)*w) + ' ' + str(w-1 + (y+1)*w) + ' ' + str(w-1 + (y+1)*w) + ' ' + str(w-1 + (y+2)*w) + ' '
    else:
        for x in range(1, w):
            stri2 += str(w-1-x + y*w) + ' ' + str(w-1-x + (y+1)*w) + ' '
        # append degenerated
        if y + 1 < h:
            stri2 += str((y+1)*w) + ' ' + str((y+1)*w) + ' ' + str((y+2)*w) + ' ' + str((y+1)*w) + ' '
    left_to_right = not left_to_right
stri += '</indices>\n'
stri2 += '</indices>\n'
        
l2.append(strv)
#l2.append(stri)
l2.append(stri2)
l2.append(strt)
l2.append('\t\t<transformation>1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1 </transformation>\n')
l2.append('\t</mesh>\n')
l2.append('</dftd-model>\n')

f2.writelines(l2)
f2.close()
print 'done!'
