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

#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.

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

FailIfNotThere()
{
    declare -ir line=$1
    declare -r filename="$2"
    
    if [ ! -e $filename ]; then
	echo "makemovie2.bash:($line): ERROR $filename does not exist.  Goodbye."
	exit $EXIT_FAILURE
    fi
}

#
# Make sure we have a sane environment
#
FailIfNotThere $LINENO sample.wpt
#if [ ! -e sample.wpt ]; then
#    echo "ERROR: need to run get a copy of the waypoint flight path - sample.wpt"
#    exit $EXIT_FAILURE
#fi

FailIfNotThere $LINENO g1-fluidized-all.iv
#if [ ! -e as1-crypt-all.iv ]; then
#    echo "ERROR: need to run bootvol-thesis.bash.  as1-crypt-all.iv does not exist"
#    exit $EXIT_FAILURE
#fi

# 0..99 frames
declare -ir maxFrames=100

# image size
declare -i s=200



#
# Make all the frames
#
declare -ir size=200
declare -r render_args="axes.iv     -p 0.05  -w sample.wpt -L -W $size -H $size -v 6"


declare -ar ol_groups=( g1-fluidized g2-undeformed g3-sheared g4-little-def g5-intermediate )

for group in "${ol_groups[@]}"; do
    echo #
    #declare iv=k$group.iv
    FailIfNotThere $LINENO $iv
    echo render -b ${group}-all $iv $render_args
done

echo "early exit" && exit $EXIT_SUCCESS

#convert -modulate 210 -size 350x350 -resize 350x350 tmp.pnm small3/$file

while [ $maxFrames != $fileNum ]; do
    f=`printf "%04d" $fileNum`
    echo $f

    if [ 1 == 1 ]; then 
	pngtopnm ${b1}${f}.png > ${b1}${f}.pnm
	pngtopnm ${b2}${f}.png > ${b2}${f}.pnm
	pngtopnm ${b3}${f}.png > ${b3}${f}.pnm
	pngtopnm ${b4}${f}.png > ${b4}${f}.pnm
    else
	pngtopnm ${b1}${f}.png | pnmscale -xysize $s $s > ${b1}${f}.pnm
	pngtopnm ${b2}${f}.png | pnmscale -xysize $s $s > ${b2}${f}.pnm
	pngtopnm ${b3}${f}.png | pnmscale -xysize $s $s > ${b3}${f}.pnm
	pngtopnm ${b4}${f}.png | pnmscale -xysize $s $s > ${b4}${f}.pnm
    fi

    # FIX: can this be done without writing tmp files?
    pnmcat -lr ${b1}${f}.pnm ${b2}${f}.pnm > left2.pnm
    pnmcat -lr ${b3}${f}.pnm ${b4}${f}.pnm > right2.pnm
    pnmcat -lr left2.pnm right2.pnm | pnmtopng > ${outbase}${f}.png

    fileNum=$[fileNum+1]

    rm -f ${b1}${f}.pnm ${b2}${f}.pnm ${b3}${f}.pnm ${b4}${f}.pnm left2.pnm right2.pnm

done
