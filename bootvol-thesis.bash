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

export PATH=${PATH}:.:..
cells=100
draw=1000000
declare -ar groups=( as1-crypt as2-slump as3-undef )
declare -r w=0.5
declare -r boundaries="-x -${w} -X ${w} -y -${w} -Y ${w} -z -${w} -Z ${w}"
declare -r debug_level=2

echo $boundaries

# FIX: if exists Makefile, do this
make targets-no-test
#make s_bootstrap xyzdensity xyzvol_cmp volhdr_edit vol_iv volmakecmap vol2vol

#
# Bootstrap each of the groups to produce Vmin (V3), Vint(V2), Vmax(V1) volumes
#
if [ 1 == 1 ]; then
    for group in "${groups[@]}"; do

	echo Processing $group
	s_bootstrap ${group}.s -f xyz  -n 3 --out ${group}.xyz. -p --draw ${draw}
	mv ${group}.xyz.1.vmax ${group}-boot.xyz.vmax
	mv ${group}.xyz.2.vint ${group}-boot.xyz.vint
	mv ${group}.xyz.3.vmin ${group}-boot.xyz.vmin

	echo Densifying
	args="  --bpv=16 -w ${cells} -t ${cells} -d ${cells} --verbosity=$debug_level"
	xyzdensity ${group}-boot.xyz.vmax --out=${group}-vmax.vol -p 1 $args $boundaries
	echo -n 2
	xyzdensity ${group}-boot.xyz.vint --out=${group}-vint.vol -p 1 $args $boundaries
	echo -n 3
	xyzdensity ${group}-boot.xyz.vmin --out=${group}-vmin.vol -p 1 $args $boundaries

	echo -n 4
	xyzdensity --out=${group}-all-1.0.vol -p 1 -b 8 -w ${cells} -t ${cells} -d ${cells} $boundaries \
	    ${group}-boot.xyz.vmax \
	    ${group}-boot.xyz.vint \
	    ${group}-boot.xyz.vmin 

	
	#volhdr_edit ${group}-all-1.0.vol --out=${group}-all-0.5.vol --xscale=0.5 --yscale=0.5 --zscale=0.5
	#volhdr_edit ${group}-all-1.0.vol --out=${group}-all-2.0.vol --xscale=2.0 --yscale=2.0 --zscale=2.0

	volmakecmap --cpt=rgba.cpt -o current.cmap --zero=0

	vol_iv -b=2 -c ALPHA_BLENDING --numslicescontrol=ALL -p NONE -C current.cmap -o ${group}-all-1.0.iv ${group}-all-1.0.vol

	scale="--xscale=0.5 --yscale=0.5 --zscale=0.5"
	volhdr_edit ${group}-vmax.vol --out=tmp $scale && /bin/mv tmp ${group}-vmax.vol
	volhdr_edit ${group}-vint.vol --out=tmp $scale && /bin/mv tmp ${group}-vint.vol
	volhdr_edit ${group}-vmin.vol --out=tmp $scale && /bin/mv tmp ${group}-vmin.vol

	#
	# Make each component viewable
	#

	vol2vol -o ${group}-vmax-8.vol ${group}-vmax.vol  -p 1 --bpv=8 -j 0.5 -k 0.5 -l 0.5
	vol2vol -o ${group}-vint-8.vol ${group}-vint.vol  -p 1 --bpv=8 -j 0.5 -k 0.5 -l 0.5
	vol2vol -o ${group}-vmin-8.vol ${group}-vmin.vol  -p 1 --bpv=8 -j 0.5 -k 0.5 -l 0.5

    done
fi

# Now we need to make a compatability matrix.

# convert them all to xyz files
for group in "${groups[@]}"; do
    s_eigs < $group.s > $group.eigs
    ./eigs2xyz.py $group.eigs > $group.xyz
    awk '{print $1,$2,$3}' $group.xyz > $group-vmin.xyz
    awk '{print $4,$5,$6}' $group.xyz > $group-vint.xyz
    awk '{print $7,$8,$9}' $group.xyz > $group-vmax.xyz
done

for eig_type in vmin vint vmax; do
    echo $eig_type comparing to vol
    for file in as?-?????-${eig_type}.vol; do 
	./xyzvol_cmp -v $debug_level -d $file as?-?????-$eig_type.xyz > ${eig_type}-${file}.cmp
    done
done

echo
echo Done with $0
