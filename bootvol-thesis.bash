#!/bin/bash

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

export PATH=${PATH}:.
cells=10
draw=100
declare -ar groups=( as1-crypt as2-slump as3-undef )
declare -r w=0.5
declare -r boundaries="-x -${w} -X ${w} -y -${w} -Y ${w} -z -${w} -Z ${w}"

echo $boundaries

# FIX: remove this make
make s_bootstrap xyzdensity xyzvol_cmp



#
# Bootstrap each of the groups to produce Vmin (V3), Vint(V2), Vmax(V1) volumes
#
if [ 1 == 1 ]; then
    for group in "${groups[@]}"; do
	echo Processing $group
	s_bootstrap --in=${group}.s -f xyz  -n 3 --out ${group}.xyz. -p --draw ${draw}
	mv ${group}.xyz.1 ${group}-boot.xyz.vmax
	mv ${group}.xyz.2 ${group}-boot.xyz.vint
	mv ${group}.xyz.3 ${group}-boot.xyz.vmin
	echo Densifying
	args="  --bpv=16 -w ${cells} -t ${cells} -d ${cells}"
	xyzdensity --in=${group}-boot.xyz.vmax --out=${group}-vmax.vol -p 1 $args $boundaries
	xyzdensity --in=${group}-boot.xyz.vint --out=${group}-vint.vol -p 1 $args $boundaries
	xyzdensity --in=${group}-boot.xyz.vmin --out=${group}-vmin.vol -p 1 $args $boundaries

	scale="--xscale=0.5 --yscale=0.5 --zscale=0.5"
	volhdr_edit --in=${group}-vmax.vol --out=tmp $scale && /bin/mv tmp ${group}-vmax.vol
	volhdr_edit --in=${group}-vint.vol --out=tmp $scale && /bin/mv tmp ${group}-vint.vol
	volhdr_edit --in=${group}-vmin.vol --out=tmp $scale && /bin/mv tmp ${group}-vmin.vol
    #volinfo -r -i ${group}-vmax.vol
    done
fi

