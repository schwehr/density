#!/bin/bash
# --verbose  print as lines are read

# $Revision$  $Author$  $Date$

##############################################################################
#     Copyright (C) 2004  Kurt Schwehr
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
###############################################################################

# The goal of this script is to take 1st the ardath data sets and test
# out the bootstrapping blobby density function and create a
# visualization of it.



######################################################################
# Debugging
######################################################################

. debug.bash

######################################################################
# Local tuning
######################################################################

die()
{
    declare -ir line=$1
    echo "ERROR: Command failed at line $line"
    exit $EXIT_FAILURE
}

export PATH=${PATH}:.:..
declare -ir cells=100
if [ -z "$DENSITY_DRAW" ]; then
    declare -ir draw=25000
else
    declare -ir draw=$DENSITY_DRAW
fi
DebugEcho $TRACE $LINENO "draw = $draw"


declare -ar groups=( as1-crypt as2-slump as3-undef )
declare -r w=0.5
declare -r boundaries="-x -${w} -X ${w} -y -${w} -Y ${w} -z -${w} -Z ${w}"

# Make all volumes be a unit cube
declare -r scale="--xscale=0.5 --yscale=0.5 --zscale=0.5"

DebugEcho $TERSE $LINENO "Cells = $cells   Draw = $draw "
DebugEcho $VERBOSE $LINENO "boundaries are $boundaries"

if [ -e Makefile ]; then
    make targets-no-test
    #make s_bootstrap xyzdensity xyzvol_cmp volhdr_edit vol_iv volmakecmap vol2vol
fi

#
# Bootstrap each of the groups to produce Vmin (V3), Vint(V2), Vmax(V1) volumes
#
if [ 0 == 1 ]; then
    for group in "${groups[@]}"; do

	DebugEcho $TERSE $LINENO "Processing $group"

	if [ ! -e ${group}-boot.xyz.vmax ]; then 
	    if [ ! -e ${group}.s ]; then
		echo "ERROR: ${group}.s is missing.  Goodbye."
		exit $EXIT_FAILURE
	    fi
	    s_bootstrap ${group}.s -f xyz  -n 3 --out ${group}.xyz. -p --draw ${draw} -v $debugLevel
	    mv ${group}.xyz.1.vmax ${group}-boot.xyz.vmax
	    mv ${group}.xyz.2.vint ${group}-boot.xyz.vint
	    mv ${group}.xyz.3.vmin ${group}-boot.xyz.vmin
	else
	    DebugEcho $TERSE $LINENO "Using existing bootstrapped xyz's"
	fi

	DebugEcho $TRACE $LINENO  Densifying:  calling xyzdensity 4 times

	args=" -p 1  --bpv=16 -w ${cells} -t ${cells} -d ${cells} --verbosity=$debugLevel"
	
 	xyzdensity ${group}-boot.xyz.vmax --out=${group}-vmax.vol $args $boundaries
 	xyzdensity ${group}-boot.xyz.vint --out=${group}-vint.vol $args $boundaries
 	xyzdensity ${group}-boot.xyz.vmin --out=${group}-vmin.vol $args $boundaries
 	xyzdensity --out=${group}-all.vol -p 1 -b 8 -w ${cells} -t ${cells} -d ${cells} $boundaries \
 	    ${group}-boot.xyz.vmax \
 	    ${group}-boot.xyz.vint \
 	    ${group}-boot.xyz.vmin 
	
	if [ ! -e current.cmap ]; then 
	    volmakecmap --cpt=rgba.cpt -o current.cmap --zero=0
	fi

	volhdr_edit ${group}-all.vol --out=tmp.vol $scale && /bin/mv tmp.vol ${group}-all.vol
	# --box=2.0
	vol_iv -c ALPHA_BLENDING --numslicescontrol=ALL -p NONE -C current.cmap -o ${group}-all.iv ${group}-all.vol

	volhdr_edit ${group}-vmax.vol --out=tmp.vol $scale && /bin/mv tmp.vol ${group}-vmax.vol
	volhdr_edit ${group}-vint.vol --out=tmp.vol $scale && /bin/mv tmp.vol ${group}-vint.vol
	volhdr_edit ${group}-vmin.vol --out=tmp.vol $scale && /bin/mv tmp.vol ${group}-vmin.vol

	#
	# Make each component viewable... scale them to min max
	#
	vol2vol_args="  -p 0 --bpv=8 -j 0.5 -k 0.5 -l 0.5 -v $debugLevel"
	vol2vol -o ${group}-vmax-8.vol ${group}-vmax.vol $vol2vol_args
	vol2vol -o ${group}-vint-8.vol ${group}-vint.vol $vol2vol_args
	vol2vol -o ${group}-vmin-8.vol ${group}-vmin.vol $vol2vol_args

	# -b=2
	ivargs=" -c ALPHA_BLENDING --numslicescontrol=ALL -p NONE -C current.cmap -v $debugLevel"
	vol_iv -o ${group}-vmax-8.iv ${group}-vmax-8.vol $ivargs
	vol_iv -o ${group}-vint-8.iv ${group}-vint-8.vol $ivargs
	vol_iv -o ${group}-vmin-8.iv ${group}-vmin-8.vol $ivargs

    done
fi

# Now we need to make a compatability matrix.

# convert them all to xyz files
if [ 0 == 1 ]; then
    for group in "${groups[@]}"; do
	s_eigs < $group.s > $group.eigs
	eigs2xyz.py $group.eigs > $group.xyz
	awk '{print $1,$2,$3}' $group.xyz > $group-vmin.xyz
	awk '{print $4,$5,$6}' $group.xyz > $group-vint.xyz
	awk '{print $7,$8,$9}' $group.xyz > $group-vmax.xyz
    done

    for eig_type in vmin vint vmax; do
	echo $eig_type comparing to vol
	for file in as?-?????-${eig_type}.vol; do 
	    xyzvol_cmp -r -v $debugLevel -d $file as?-?????-$eig_type.xyz -o ${eig_type}-${file}.cmp
	done
    done
fi


######################################################################
# Owens Lake data
######################################################################


DebugEcho $TERSE $LINENO "##############"
DebugEcho $TERSE $LINENO "# Owens Lake #"
DebugEcho $TERSE $LINENO "##############"


#declare -ar ol_groups=( g1-fluidized g2-undeformed g3-sheared g4-little-def g5-intermediate )
declare -ar ol_groups=( g2-undeformed )


if [ ! -e "rosenbaum-ams-stripped.dat" ]; then
    DebugEcho $ALWAY $LINENO "ERROR: Must have rosenbaum-ams-stripped.dat to continue.  Bye" 
    exit $EXIT_FAILURE
fi

if [ ! -e g1-fluidized.dat ]; then getgroups-ol92.bash; fi


if [ 1 == 1 ]; then
    for group in "${ol_groups[@]}"; do

	DebugEcho $TERSE $LINENO "Processing $group"
	# Should normed error be devided by 3?
	awk '{print $32/3,$33/3,$34/3,$35/3,$36/3,$37/3,$5,$2}' $group.dat > $group.s
	if [ ! -e ${group}-boot-vmax.xyz ]; then 
	    if [ ! -e ${group}.s ]; then
		echo "ERROR: ${group}.s is missing.  Goodbye."
		exit $EXIT_FAILURE
	    fi
	    s_bootstrap ${group}.s -f xyz  -n 3 --out ${group}.xyz. -p --draw ${draw} -v $debugLevel || die $LINENO
	    mv ${group}.xyz.1.vmax ${group}-boot-vmax.xyz
	    mv ${group}.xyz.2.vint ${group}-boot-vint.xyz
	    mv ${group}.xyz.3.vmin ${group}-boot-vmin.xyz
	else
	    DebugEcho $TERSE $LINENO "Using existing bootstrapped xyz's"
	fi

	DebugEcho $TRACE $LINENO  "Densifying:  calling xyzdensity 4 times"

	echo "Only doing 8 bpv"
	#args=" -p 1  --bpv=16 -w ${cells} -t ${cells} -d ${cells} --verbosity=$debugLevel"
	args=" -p 1  --bpv=8 -w ${cells} -t ${cells} -d ${cells} --verbosity=$debugLevel"
	if [ ! -e ${group}-vmax.vol ]; then
	    xyzdensity ${group}-boot-vmax.xyz --out=${group}-vmax-8.vol $args $boundaries || die $LINENO
	    xyzdensity ${group}-boot-vint.xyz --out=${group}-vint-8.vol $args $boundaries || die $LINENO
	    xyzdensity ${group}-boot-vmin.xyz --out=${group}-vmin-8.vol $args $boundaries || die $LINENO
	    xyzdensity --out=${group}-all.vol -p 1 -b 8 -w ${cells} -t ${cells} -d ${cells} $boundaries \
		${group}-boot-vmax.xyz \
		${group}-boot-vint.xyz \
		${group}-boot-vmin.xyz  || die $LINENO
	    volhdr_edit ${group}-all.vol --out=tmp.vol $scale && /bin/mv tmp.vol ${group}-all.vol || die $LINENO
	    volhdr_edit ${group}-vmax-8.vol --out=tmp.vol $scale && /bin/mv tmp.vol ${group}-vmax-8.vol || die $LINENO
	    volhdr_edit ${group}-vint-8.vol --out=tmp.vol $scale && /bin/mv tmp.vol ${group}-vint-8.vol || die $LINENO
	    volhdr_edit ${group}-vmin-8.vol --out=tmp.vol $scale && /bin/mv tmp.vol ${group}-vmin-8.vol || die $LINENO
	    #
            # Make each component viewable... scale them to min max
	    #
	    vol2vol_args="  -p 0 --bpv=8 -j 0.5 -k 0.5 -l 0.5 -v $debugLevel"
	    #vol2vol -o ${group}-vmax-8.vol ${group}-vmax.vol $vol2vol_args || die $LINENO
	    #vol2vol -o ${group}-vint-8.vol ${group}-vint.vol $vol2vol_args || die $LINENO
	    #vol2vol -o ${group}-vmin-8.vol ${group}-vmin.vol $vol2vol_args || die $LINENO
	else
	    DebugEcho $TERSE $LINENO "Using existing ${group}-vmax.vol"
	fi

	if [ ! -e current.cmap ]; then 
	    if [ ! -e rgba.cpt ]; then
		DebugEcho $TRACE $LINENO "ERROR:  missing rgba.cpt"
	    fi
	    volmakecmap --cpt=rgba.cpt -o current.cmap --zero=0 || die $LINENO
	fi

	# --box=2.0
	vol_iv -c ALPHA_BLENDING --numslicescontrol=ALL -p NONE -C current.cmap -o ${group}-all.iv ${group}-all.vol \
	|| die $LINENO

	ivargs=" -c ALPHA_BLENDING --numslicescontrol=ALL -p NONE -C current.cmap -v $debugLevel"
	vol_iv -o ${group}-vmax-8.iv ${group}-vmax-8.vol $ivargs || die $LINENO
	vol_iv -o ${group}-vint-8.iv ${group}-vint-8.vol $ivargs || die $LINENO
	vol_iv -o ${group}-vmin-8.iv ${group}-vmin-8.vol $ivargs || die $LINENO

	#
	# Plot all the points as boxes
	#
	s_eigs < $group.s > $group.eigs
	eigs2xyz.py $group.eigs > $group.xyz
	awk '{print $1,$2,$3}' $group.xyz > $group.xyz.vmin || die $LINENO
	awk '{print $4,$5,$6}' $group.xyz > $group.xyz.vint || die $LINENO
	awk '{print $7,$8,$9}' $group.xyz > $group.xyz.vmax || die $LINENO

	xyz_iv $box -p --color="1 0 0" --out=$group.xyz.vmin.iv -v 3 $group.xyz.vmin || die $LINENO
	xyz_iv $box -p --color="1 1 0" --out=$group.xyz.vint.iv -v 3 $group.xyz.vint || die $LINENO
	xyz_iv $box -p --color="0 0 1" --out=$group.xyz.vmax.iv -v 3 $group.xyz.vmax || die $LINENO
    done
fi

######################################################################

echo
echo Done with $0

