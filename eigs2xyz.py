#!/usr/bin/env python

"""Takes s_eigs output and converts it to xyz with y north, x east, z up"""

# Kurt Schwehr, Sept 2004
# http://schwehr.org/xcore

from math import *
import os, os.path, string, sys, math
import re # Regular expressions
if (2 != len(sys.argv)):
    print "crap"
    sys.exit(1)

if (1 != os.access(sys.argv[1],os.F_OK)):
    print "ERROR: output file exists.  do not want to over write!!"
    print "  ", sys.argv[1]
    sys.exit(1)

infile = open(sys.argv[1],"r");


def printXyzFromEigs(len, dec, inc):
    incRad = inc * pi/180
    decRad = dec * pi/180
    y = len * cos(incRad) * cos(decRad)
    x = len * cos(incRad) * sin(decRad)
    z = -sin(incRad)*len  # inclination 90 is straigh down
    v = [ x, y, z]
    #v[0] = x
    #v[1] = y
    #v[2] = z
    return v

#def checkXyzFromEigs():
    #print "FIX - implement checkXyzFromEigs: ok"

#checkXyzFromEigs()

for line in infile.xreadlines():
    s = line.split()
    vMin = printXyzFromEigs(float(s[0]),float(s[1]),float(s[2]))
    vInt = printXyzFromEigs(float(s[3]),float(s[4]),float(s[5]))
    vMax = printXyzFromEigs(float(s[6]),float(s[7]),float(s[8]))
    print vMin[0],vMin[1],vMin[2], vInt[0],vInt[1],vInt[2], vMax[0],vMax[1],vMax[2]

    
