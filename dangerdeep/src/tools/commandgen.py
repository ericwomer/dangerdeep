#!/usr/bin/python

# generate C++ code to send commands
# call:
# class-name command-name [type variable-name]*

msgclass = 'message' # os_message
cmdqname = 'command_queue'

import sys

if len(sys.argv) != 2:
    print 'call ', sys.argv[0], ' definitions-file'
    print '\nSyntax of definitions file:\n'
    print '1st line is class name'
    print 'lines 2-n have 1-x space separated strings,'
    print ' first string is command name, then in pairs parameter type and name.\n'
    sys.exit()

f = open(sys.argv[1])
fl = f.readlines()
f.close()

classname = fl[0][:-1]

header = []
header2 = []
header3 = []
cpp = []
cpp2 = []

def makeitso(myline, header, header2, header3, cpp, cpp2):
    mystrings = myline.split()
    cmdname = mystrings[0]
    params = []
    for i in range(0, (len(mystrings)-1)/2):
        params += [(mystrings[1+2*i], mystrings[1+2*i+1])]
    parstr = ''
    parstr2 = ''
    parstr3 = ''
    for i in params:
        tp = i[0]
        if tp == 'std::string':
            tp = 'const ' + tp + '&'
        parstr += ', ' + tp + ' ' + i[1]
        parstr2 += ', ' + tp + ' ' + i[1] + '_'
        parstr3 += ', ' + i[1]
    header += ['\tbool ' + cmdname + '(' + parstr[2:] + ');']
    header2 += ['\tvoid exec_' + cmdname + '(' + parstr[2:] + ');']
    header3 += ['\tclass command_' + cmdname + ' : public ' + msgclass]
    header3 += ['\t{']
    header3 += ['\t\t' + classname + '& my_' + classname + ';']
    for i in params:
        header3 += ['\t\t' + i[0] + ' ' + i[1] + ';']
    header3 += ['\t\tvoid eval() const { my_' + classname + '.exec_' + cmdname + '(' + parstr3[2:] + '); }']
    header3 += ['\t public:']
    cs = '\t\tcommand_' + cmdname + '(' + classname
    cs += '& my_' + classname + '_' + parstr2 + ') : my_' + classname + '(my_' + classname + '_)'
    for i in params:
        cs += ', ' + i[1] + '(' + i[1] + '_)'
    cs += ' {}'
    header3 += [cs]
    header3 += ['\t};']
    header3 += ['']

    cpp += ['bool ' + classname + '::' + cmdname + '(' + parstr[2:] + ')']
    cpp += ['{']
    cpp += ['\t' + cmdqname + '.send(std::auto_ptr<' + msgclass + '>(new command_' + cmdname + '(*this' + parstr3 + ')));']
    cpp += ['}']
    cpp += ['']
    #cpp += ['']
    #cpp += ['']

    cpp2 += ['void ' + classname + '::exec_' + cmdname + '(' + parstr[2:] + ')']
    cpp2 += ['{']
    cpp2 += ['\t//code...']
    cpp2 += ['}']
    cpp += ['']
    #cpp += ['']
    #cpp += ['']




for i in fl[1:]:
    makeitso(i[:-1], header, header2, header3, cpp, cpp2)

print '***HEADER***\n\n'
for i in header:
    print i
for i in header2:
    print i
for i in header3:
    print i

print '\n\n***CODE***\n'
for i in cpp:
    print i
for i in cpp2:
    print i
