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

# Test out all the functionality with just one point to make sure
# everything lines up.

######################################################################
# Debugging
######################################################################

. debug.bash

######################################################################
# Local tuning
######################################################################

export PATH=${PATH}:.
cells=50
if [ -z "$DENSITY_DRAW" ]; then
    # Keep a low number so debugging goes quickly
    declare -ir draw=25000
else
    declare -ir draw=$DENSITY_DRAW
fi
DebugEcho $TRACE $LINENO "draw = $draw"
declare -r w=0.5
declare -r boundaries="-x -${w} -X ${w} -y -${w} -Y ${w} -z -${w} -Z ${w}"
#declare -r debugLevel=2

if [ -e Makefile ]; then
    make targets-no-test
fi

head -1 as2-slump.s > one.s

echo Processing one.s
if [ ! -e one-boot.xyz ]; then 
    s_bootstrap one.s -f xyz  -n 1 --out one-boot.xyz -p --draw ${draw}
    s_bootstrap one.s -f xyz  -n 3 --out one.xyz. -p --draw ${draw}
    mv one.xyz.1.vmax one-boot.xyz.vmax
    mv one.xyz.2.vint one-boot.xyz.vint
    mv one.xyz.3.vmin one-boot.xyz.vmin
fi

echo Densifying
args="  --bpv=16 -w ${cells} -t ${cells} -d ${cells} --verbosity=$debugLevel"

if [ ! -e one-all.vol ]; then
    xyzdensity one-boot.xyz.vmax --out=one-vmax.vol -p 1 $args $boundaries
    xyzdensity one-boot.xyz.vint --out=one-vint.vol -p 1 $args $boundaries
    xyzdensity one-boot.xyz.vmin --out=one-vmin.vol -p 1 $args $boundaries

    xyzdensity --out=one-all.vol -p 1 -b 8 -w ${cells} -t ${cells} -d ${cells} $boundaries \
	one-boot.xyz.vmax \
	one-boot.xyz.vint \
	one-boot.xyz.vmin 
fi

if [ ! -e one.cmap ]; then
    volmakecmap --cpt=rgba.cpt -o one.cmap --zero=0
fi

# Make the bounding box at 1.05 to be just outside of the volume
# --box=1.05
vol_iv -c ALPHA_BLENDING --numslicescontrol=ALL -p NONE -C one.cmap -o one-all.iv one-all.vol

scale="--xscale=0.5 --yscale=0.5 --zscale=0.5"
volhdr_edit one-vmax.vol --out=tmp.vol $scale && /bin/mv tmp.vol one-vmax.vol
volhdr_edit one-vint.vol --out=tmp.vol $scale && /bin/mv tmp.vol one-vint.vol
volhdr_edit one-vmin.vol --out=tmp.vol $scale && /bin/mv tmp.vol one-vmin.vol
volhdr_edit one-all.vol  --out=tmp.vol $scale && /bin/mv tmp.vol one-all.vol


s_eigs < one.s > one.eigs
eigs2xyz.py one.eigs > one.xyz
awk '{print $1,$2,$3}' one.xyz > one.xyz.vmin
awk '{print $4,$5,$6}' one.xyz > one.xyz.vint
awk '{print $7,$8,$9}' one.xyz > one.xyz.vmax

xyz_iv --box=0.005 -p --color="1 0 0" --out=one.xyz.vmin.iv -v 3 one.xyz.vmin
xyz_iv --box=0.005 -p --color="1 1 0" --out=one.xyz.vint.iv -v 3 one.xyz.vint
xyz_iv --box=0.005 -p --color="0 0 1" --out=one.xyz.vmax.iv -v 3 one.xyz.vmax

#debugLevel=1
echo "TYPE  VOL_FILE     XYZ_FILE     x         y           z     counts FracTotal FracCDF"
echo "------------------------------------------------------------------------------------"
echo -n "VMIN: " && xyzvol_cmp -v $debugLevel -d one-vmin.vol one.xyz.vmin --out=-
echo -n "VINT: " && xyzvol_cmp -v $debugLevel -d one-vint.vol one.xyz.vint --out=-
echo -n "VMAX: " && xyzvol_cmp -v $debugLevel -d one-vmax.vol one.xyz.vmax --out=-
