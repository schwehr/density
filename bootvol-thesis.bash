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
# visualization of it.



######################################################################
# Debugging
######################################################################

. debug.bash

######################################################################
# Local tuning
######################################################################


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


declare -ar ol_groups=( g1-fluidized g2-undeformed g3-sheared g4-little-def g5-intermediate )
#declare -ar ol_groups=( g2-undeformed )


if [ ! -e "rosenbaum-ams-stripped.dat" ]; then
    DebugEcho $ALWAY $LINENO "ERROR: Must have rosenbaum-ams-stripped.dat to continue.  Bye" 
    exit $EXIT_FAILURE
fi

if [ ! -e g1-fluidized.dat ]; then getgroups.bash; fi


if [ 1 == 1 ]; then
    for group in "${ol_groups[@]}"; do

	DebugEcho $TERSE $LINENO "Processing $group"
	# Should normed error be devided by 3?
	awk '{print $32/3,$33/3,$34/3,$35/3,$36/3,$37/3,$5,$2}' $group.dat > $group.s
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
	DebugEcho $TRACE $LINENO  "Densifying:  calling xyzdensity 4 times"

	args=" -p 1  --bpv=16 -w ${cells} -t ${cells} -d ${cells} --verbosity=$debugLevel"
	
 	xyzdensity ${group}-boot.xyz.vmax --out=${group}-vmax.vol $args $boundaries
 	xyzdensity ${group}-boot.xyz.vint --out=${group}-vint.vol $args $boundaries
 	xyzdensity ${group}-boot.xyz.vmin --out=${group}-vmin.vol $args $boundaries
 	xyzdensity --out=${group}-all.vol -p 1 -b 8 -w ${cells} -t ${cells} -d ${cells} $boundaries \
 	    ${group}-boot.xyz.vmax \
 	    ${group}-boot.xyz.vint \
 	    ${group}-boot.xyz.vmin 

	if [ ! -e current.cmap ]; then 
	    if [ ! -e rgba.cpt ]; then
		DebugEcho $TRACE $LINENO "ERROR:  missing rgba.cpt"
	    fi
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

	ivargs=" -c ALPHA_BLENDING --numslicescontrol=ALL -p NONE -C current.cmap -v $debugLevel"
	vol_iv -o ${group}-vmax-8.iv ${group}-vmax-8.vol $ivargs
	vol_iv -o ${group}-vint-8.iv ${group}-vint-8.vol $ivargs
	vol_iv -o ${group}-vmin-8.iv ${group}-vmin-8.vol $ivargs
    done
fi

######################################################################

echo
echo Done with $0

