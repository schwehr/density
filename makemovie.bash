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
FailIfNotThere $LINENO g1-fluidized-all.iv

# 0..99 frames
declare -ir maxFrames=100
#declare -ir maxFrames=5


#
# Make all the frames
#
declare -ir size=200
declare -r render_args="axes.iv     -p 0.05  -w sample.wpt -L -W $size -H $size -v $debugLevel"

declare -ar ol_groups=( g1-fluidized g2-undeformed g3-sheared g4-little-def g5-intermediate )

echo ${ol_groups[@]}

if [ 1 == 1 ]; then 
    for group in "${ol_groups[@]}"; do
	DebugEcho $TERSE $LINENO "Rendering group $group"

	FailIfNotThere $LINENO ${group}-all.iv
	cmd="render -b ${group}-all- ${group}-all.iv $render_args"
	DebugEcho $VERBOSE $LINENO "$cmd"
	$cmd || die $LINENO

	FailIfNotThere $LINENO ${group}-vmax-8.iv
        render -b ${group}-vmax- ${group}-vmax-8.iv $render_args $group-vmax.xyz.iv || die $LINENO

	FailIfNotThere $LINENO ${group}-vint-8.iv
        render -b ${group}-vint- ${group}-vint-8.iv $render_args $group-vint.xyz.iv || die $LINENO

	FailIfNotThere $LINENO ${group}-vmin-8.iv
        render -b ${group}-vmin- ${group}-vmin-8.iv $render_args $group-vmin.xyz.iv || die $LINENO
    done
fi

if [ 1 == 1 ]; then 
    DebugEcho $TERSE $LINENO "Brightening all images"
    for file in g[1-5]-*.png; do
	convert -modulate 210 $file tmp-$file || die $LINENO
	/bin/mv -f tmp-$file $file
    done
fi

#
# Label the all 3 component images using ImageMagic
# http://www-106.ibm.com/developerworks/library/l-graf/?ca=dnt-428
#
if [ 1 == 1 ]; then 
    if [ 1 == 1 ]; then 
	DebugEcho $TERSE $LINENO "Label the 'all' images"
	for group in "${ol_groups[@]}"; do
	    DebugEcho $TERSE $LINENO "  Group: $group"
	    for file in $group-all-*.png; do
		convert -font helvetica -fill white -pointsize 12 -draw "text 10,15 $group" $file tmp-$file
		/bin/mv -f tmp-$file $file
	    done
	done
    fi
    DebugEcho $TERSE $LINENO "Labeling the component images - vmin"
    for file in g?-*-vmin-*.png; do
	convert -font helvetica -fill white -pointsize 12 -draw "text 10,15 Vmin" $file tmp-$file
	/bin/mv -f tmp-$file $file
    done
    DebugEcho $TERSE $LINENO "Labeling the component images - vint"
    for file in g?-*-vint-*.png; do
	convert -font helvetica -fill white -pointsize 12 -draw "text 10,15 Vint" $file tmp-$file
	/bin/mv -f tmp-$file $file
    done
    DebugEcho $TERSE $LINENO "Labeling the component images - vmax"
    for file in g?-*-vmax-*.png; do
	convert -font helvetica -fill white -pointsize 12 -draw "text 10,15 Vmax" $file tmp-$file
	/bin/mv -f tmp-$file $file
    done
fi

#
# Group all rendered frames for one step into one big movie frame
# 

if [ 1 == 1 ]; then 
    for group in "${ol_groups[@]}"; do
	declare -i fileNum=0
	DebugEcho $TERSE $LINENO "Assemble frames for group: $group"
	while [ $maxFrames != $fileNum ]; do
	    f=`printf "%04d" $fileNum`
	    echo $f
	    pngtopnm ${group}-all-${f}.png > all.pnm
	    pngtopnm ${group}-vmax-${f}.png > vmax.pnm
	    pngtopnm ${group}-vint-${f}.png > vint.pnm
	    pngtopnm ${group}-vmin-${f}.png > vmin.pnm

	    pnmcat -lr  all.pnm vmax.pnm > l
	    pnmcat -lr vint.pnm vmin.pnm > r
	    pnmcat -lr l r | pnmtopng > ${group}-$f.png

	    fileNum=$[fileNum+1]
	    rm -f all.pnm vmax.pnm vint.pnm vmin.pnm l r
	done
    done
fi


if [ 1 == 1 ]; then 
    declare -i fileNum=0
    while [ $maxFrames != $fileNum ]; do
	f=`printf "%04d" $fileNum`
	DebugEcho $TERSE $LINENO "Assemble composites to a superframe for $f"
	for group in "${ol_groups[@]}"; do
	    pngtopnm ${group}-${f}.png > ${group}.pnm  || die $LINENO
	done
	pnmcat -tb g1-fluidized.pnm g2-undeformed.pnm > t1  || die $LINENO
	pnmcat -tb g3-sheared.pnm   g4-little-def.pnm > t2  || die $LINENO
	pnmcat -tb t1 t2 > t3 || die $LINENO
	pnmcat -tb t3 g5-intermediate.pnm | pnmtopng > ol-$f.png || die $LINENO
	rm -f t{1,2,3}  g1-fluidized.pnm g2-undeformed.pnm g3-sheared.pnm  g4-little-def.pnm g5-intermediate.pnm
	fileNum=$[fileNum+1]
    done
fi


