#!/usr/bin/env python

"""Split a file in two using the 1st column depth."""

#     Copyright (C) 2004  Kurt Schwehr


#     This program is free software; you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation; either version 2 of the License, or
#     (at your option) any later version.

#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.

#     You should have received a copy of the GNU General Public License
#     along with this program; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



import os, os.path, string, sys
import re # Regular expressions
if (5 != len(sys.argv)):
    print len(sys.argv)
    print sys.argv
    print '\n   ERROR:  invalid number of arguments'
    print "   USAGE: ",sys.argv[0], " depth in.dat out-low.dat out-high.dat"
    sys.exit(1)

i=1
depth=float(sys.argv[i]); i=i+1
inFileName=sys.argv[i]; i=i+1
outFileName1=sys.argv[i]; i=i+1
outFileName2=sys.argv[i]; i=i+1

if (1 == os.access(outFileName1,os.F_OK)):
    print "ERROR: output file exists.  do not want to over write!!"
    print "  ", outFileName1
    sys.exit(1)

if (1 == os.access(outFileName2,os.F_OK)):
    print "ERROR: output file exists.  do not want to over write!!"
    print "  ", outFileName2
    sys.exit(1)

infile = open(inFileName,"r");
outfile1 = open(outFileName1,"w");
outfile2 = open(outFileName2,"w");

for line in infile.xreadlines():
    s = line.split()
    d = float(s[0])
    if (d<depth):
        outfile1.write (line)
    else:
        outfile2.write (line)
            
