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

# The goal of this script is to take 1st the ardath data sets and test
# out the bootstrapping blobby density function and create a
# visualization of it.  Hopefully later, I will get to processing the
# Owens Lake data.



######################################################################
# Debugging
######################################################################

. debug.bash

######################################################################
# Local tuning
######################################################################

# simpleview -i 0.1 -p 0.05 as2-slump-all.iv ../axes.iv -w main.wpt -n -b as2-slump-all -W 750 -H 750
# simpleview -i 0.1 -p 0.05 as2-slump-vmax-8.iv ../axes.iv -w main.wpt -n -b as2-slump-vmax -W 750 -H 750 -v 9
# simpleview -i 0.1 -p 0.05 as2-slump-vint-8.iv ../axes.iv -w main.wpt -n -b as2-slump-vint -W 750 -H 750 -v 9

#export PATH=${PATH}:.:..:../..

if [ -z "$6" ]; then
    echo "ERROR must specify the prefix for all 4 frames and the max frame number"
    echo Had too few args...
    echo
    echo "USAGE: $0 basename1 basename2 basename3 basename4 outbasename maxFileNum+1"
    exit $EXIT_FAILURE
fi
if [ ! -z "$7" ]; then
    echo "ERROR must specify the prefix for all 4 frames and the max frame number"
    echo Had too many args
    echo
    echo "USAGE: $0 basename1 basename2 basename3 basename4 outbasename maxFileNum+1"
    exit $EXIT_FAILURE
fi

# Base names of the png frames  1==left most  2==right most
declare -r b1=$1
declare -r b2=$2
declare -r b3=$3
declare -r b4=$4
declare -r outbase=$5
declare -i max=$6

# Skip 0000 because it currently is fubar
declare -i fileNum=1

# image size
declare -i s=300

while [ $max != $fileNum ]; do
    f=`printf "%04d" $fileNum`
    echo $f

    pngtopnm ${b1}${f}.png | pnmscale -xysize $s $s > ${b1}${f}.pnm
    pngtopnm ${b2}${f}.png | pnmscale -xysize $s $s > ${b2}${f}.pnm
    pngtopnm ${b3}${f}.png | pnmscale -xysize $s $s > ${b3}${f}.pnm
    pngtopnm ${b4}${f}.png | pnmscale -xysize $s $s > ${b4}${f}.pnm

    # FIX: can this be done without writing tmp files?
    pnmcat -lr ${b1}${f}.pnm ${b2}${f}.pnm > left2.pnm
    pnmcat -lr ${b3}${f}.pnm ${b4}${f}.pnm > right2.pnm
    pnmcat -lr left2.pnm right2.pnm | pnmtopng > ${outbase}${f}.png

    fileNum=$[fileNum+1]

    rm -f ${b1}${f}.pnm ${b2}${f}.pnm ${b3}${f}.pnm ${b4}${f}.pnm left2.pnm right2.pnm

done


