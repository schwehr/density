#!/bin/bash
# --verbose  print as lines are read

# $Revision$  $Author$  $Date$

##############################################################################
#     Copyright (C) 2004  Kurt Schwehr
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License as published by the Free Software Foundation; either
#     version 2.1 of the License, or (at your option) any later version.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
###############################################################################

# This should become a variety of tests that make sure that all the
# programs are working ok from the outside.  The code regression tests
# can not see the outside picture.


export PATH=${PATH}:.
cells=80
draw=10000
declare -r w=0.5
declare -r boundaries="-x -${w} -X ${w} -y -${w} -Y ${w} -z -${w} -Z ${w}"
declare -r debug_level=2

declare -i testno=1  # Increment this to track which test we are one.

make targets-no-test

######################################################################
# Make sure that we can get back the same value from the cell

#echo Starting test number: $testno
echo "0.01 0.01 0.01" > $testno.xyz
range="--xmin=0. --xmax=1. --ymin=0. --ymax=1. --zmin=0. --zmax=1."
xyzdensity --out=$testno.vol --pack=1 $range -b 8 -w 10 -t 10 -d 10 $testno.xyz
count=`xyzvol_cmp -d $testno.vol $range $testno.xyz | awk '{print $6}'`
if [ 1 != $count ]; then
    echo "  Test number $testno:    FAILED"
    exit 1
else 
    echo "  Test number $testno:    ok."
fi
testno=$[testno+1]


echo "0.99 0.01 0.01" > $testno.xyz
range="--xmin=0. --xmax=1. --ymin=0. --ymax=1. --zmin=0. --zmax=1."
xyzdensity --out=$testno.vol --pack=1 $range -b 8 -w 10 -t 10 -d 10 $testno.xyz
count=`xyzvol_cmp -d $testno.vol $range $testno.xyz | awk '{print $6}'`
if [ 1 != $count ]; then
    echo "  Test number $testno:    FAILED"
    exit 1
else 
    echo "  Test number $testno:    ok."
fi
testno=$[testno+1]


echo "0.01 0.01 0.99" > $testno.xyz
range="--xmin=0. --xmax=1. --ymin=0. --ymax=1. --zmin=0. --zmax=1."
xyzdensity --out=$testno.vol --pack=1 $range -b 8 -w 10 -t 10 -d 10 $testno.xyz
count=`xyzvol_cmp -d $testno.vol $range $testno.xyz | awk '{print $6}'`
if [ 1 != $count ]; then
    echo "  Test number $testno:    FAILED"
    exit 1
else 
    echo "  Test number $testno:    ok."
fi
testno=$[testno+1]


echo "0.01 0.01 0.99" > $testno.xyz
echo "0.01 0.99 0.99" > $testno-b.xyz
range="--xmin=0. --xmax=1. --ymin=0. --ymax=1. --zmin=0. --zmax=1."
xyzdensity --out=$testno.vol --pack=1 $range -b 8 -w 10 -t 10 -d 10 $testno.xyz
count=`xyzvol_cmp -d $testno.vol $range $testno-b.xyz | awk '{print $6}'`
if [ 0 != $count ]; then
    echo "  Test number $testno:    FAILED"
    exit 1
else 
    echo "  Test number $testno:    ok."
fi
testno=$[testno+1]


echo "0.01 0.99 0.99" > $testno.xyz
range="--xmin=0. --xmax=1. --ymin=0. --ymax=1. --zmin=0. --zmax=1."
xyzdensity --out=$testno.vol --pack=1 $range -b 8 -w 10 -t 10 -d 10 $testno.xyz
count=`xyzvol_cmp -d $testno.vol $range $testno.xyz | awk '{print $6}'`
if [ 1 != $count ]; then
    echo "  Test number $testno:    FAILED"
    exit 1
else 
    echo "  Test number $testno:    ok."
fi
testno=$[testno+1]



echo "0.99 0.01 0.99" > $testno.xyz
range="--xmin=0. --xmax=1. --ymin=0. --ymax=1. --zmin=0. --zmax=1."
xyzdensity --out=$testno.vol --pack=1 $range -b 8 -w 10 -t 10 -d 10 $testno.xyz
count=`xyzvol_cmp -d $testno.vol $range $testno.xyz | awk '{print $6}'`
if [ 1 != $count ]; then
    echo "  Test number $testno:    FAILED"
    exit 1
else 
    echo "  Test number $testno:    ok."
fi
testno=$[testno+1]


echo "0.99 0.99 0.99" > $testno.xyz
range="--xmin=0. --xmax=1. --ymin=0. --ymax=1. --zmin=0. --zmax=1."
xyzdensity --out=$testno.vol --pack=1 $range -b 8 -w 10 -t 10 -d 10 $testno.xyz
count=`xyzvol_cmp -d $testno.vol $range $testno.xyz | awk '{print $6}'`
if [ 1 != $count ]; then
    echo "  Test number $testno:    FAILED"
    exit 1
else 
    echo "  Test number $testno:    ok."
fi
testno=$[testno+1]



echo "0.5 0.5 0.5" > $testno.xyz
range="--xmin=0. --xmax=1. --ymin=0. --ymax=1. --zmin=0. --zmax=1."
xyzdensity --out=$testno.vol --pack=1 $range -b 8 -w 10 -t 10 -d 10 $testno.xyz
count=`xyzvol_cmp -d $testno.vol $range $testno.xyz | awk '{print $6}'`
if [ 1 != $count ]; then
    echo "  Test number $testno:    FAILED"
    exit 1
else 
    echo "  Test number $testno:    ok."
fi
testno=$[testno+1]



