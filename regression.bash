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



######################################################################
# Make sure that s_bootstrap returns the same value if sigma is zero.

tail -1 as2-slump.s | awk '{print $1,$2,$3,$4,$5,$6,0.}' > $testno.s
s_bootstrap $testno.s -f xyz -n 1 --out $testno-boot.xyz -d 1 -p
s_eigs < $testno.s > $testno.eigs
eigs2xyz.py $testno.eigs > $testno.xyz
declare -a a=( `cat $testno.xyz` )
declare -a b=( `cat $testno-boot.xyz`  )

is_Equal  ${a[0]} ${b[0]} 0.001 || (echo "Test number $testno:    FAILED";exit 1)
is_Equal  ${a[1]} ${b[1]} 0.001 || (echo "Test number $testno:    FAILED";exit 1)
is_Equal  ${a[2]} ${b[2]} 0.001 || (echo "Test number $testno:    FAILED";exit 1)
is_Equal  ${a[3]} ${b[3]} 0.001 || (echo "Test number $testno:    FAILED";exit 1)
is_Equal  ${a[4]} ${b[4]} 0.001 || (echo "Test number $testno:    FAILED";exit 1)
is_Equal  ${a[5]} ${b[5]} 0.001 || (echo "Test number $testno:    FAILED";exit 1)
is_Equal  ${a[6]} ${b[6]} 0.001 || (echo "Test number $testno:    FAILED";exit 1)
is_Equal  ${a[7]} ${b[7]} 0.001 || (echo "Test number $testno:    FAILED";exit 1)
is_Equal  ${a[8]} ${b[8]} 0.001 || (echo "Test number $testno:    FAILED";exit 1)

testno=$[testno+1]

######################################################################
# make sure that vmin and vmax are getting written out correctly
# len(vmin) < len(vmax)
# for the 3 file output from s_bootstrap

head -1 as2-slump.s > $testno.s
s_bootstrap $testno.s -f xyz -n 3 --out $testno.xyz. -d 1 -p
declare -a vmin=( `cat $testno.xyz.3.vmin` )
declare -a vint=( `cat $testno.xyz.2.vint` )
declare -a vmax=( `cat $testno.xyz.1.vmax` )
vminlen=`echo "${vmin[0]}  ${vmin[1]}  ${vmin[2]}"  |  awk '{print sqrt($1*$1 + $2*$2 + $3*$3)}'`
vintlen=`echo "${vint[0]}  ${vint[1]}  ${vint[2]}"  |  awk '{print sqrt($1*$1 + $2*$2 + $3*$3)}'`
vmaxlen=`echo "${vmax[0]}  ${vmax[1]}  ${vmax[2]}"  |  awk '{print sqrt($1*$1 + $2*$2 + $3*$3)}'`

unset vmin vint vmax

t=`echo $vminlen $vintlen | awk '{if ($1>$2) print 0; else print 1}' `
if [ 0 == $t ]; then echo "Test number $testno:    FAILED 1";exit 1; fi
t=`echo $vminlen $vmaxlen | awk '{if ($1>$2) print 0; else print 1}' `
if [ 0 == $t ]; then echo "Test number $testno:    FAILED 2";exit 1; fi
t=`echo $vintlen $vmaxlen | awk '{if ($1>$2) print 0; else print 1}' `
if [ 0 == $t ]; then echo "Test number $testno:    FAILED 3";exit 1; fi

testno=$[testno+1]

# for the 1 file output from s_bootstrap
# Make sure that things all come out in the right order

head -1 as2-slump.s > $testno.s
s_bootstrap $testno.s -f xyz -n 1 --out $testno.xyz -d 1 -p
declare -a v=( `cat $testno.xyz` )
vminlen=`echo "${v[0]}  ${v[1]}  ${v[2]}"  |  awk '{print sqrt($1*$1 + $2*$2 + $3*$3)}'`
vintlen=`echo "${v[3]}  ${v[4]}  ${v[5]}"  |  awk '{print sqrt($1*$1 + $2*$2 + $3*$3)}'`
vmaxlen=`echo "${v[6]}  ${v[7]}  ${v[8]}"  |  awk '{print sqrt($1*$1 + $2*$2 + $3*$3)}'`

#echo min = $vminlen int = $vintlen max = $vmaxlen
unset v

t=`echo $vminlen $vintlen | awk '{if ($1>$2) print 0; else print 1}' `
if [ 0 == $t ]; then echo "Test number $testno:    FAILED 1";exit 1; fi
t=`echo $vminlen $vmaxlen | awk '{if ($1>$2) print 0; else print 1}' `
if [ 0 == $t ]; then echo "Test number $testno:    FAILED 2";exit 1; fi
t=`echo $vintlen $vmaxlen | awk '{if ($1>$2) print 0; else print 1}' `
if [ 0 == $t ]; then echo "Test number $testno:    FAILED 3";exit 1; fi

testno=$[testno+1]

