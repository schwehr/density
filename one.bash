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


export PATH=${PATH}:.
cells=100
draw=100000
declare -r w=0.5
declare -r boundaries="-x -${w} -X ${w} -y -${w} -Y ${w} -z -${w} -Z ${w}"
declare -r debug_level=2


make targets-no-test

head -1 as2-slump.s > one.s

echo Processing one.s
s_bootstrap one.s -f xyz  -n 3 --out one.xyz. -p --draw ${draw}
mv one.xyz.1 one-boot.xyz.vmax
mv one.xyz.2 one-boot.xyz.vint
mv one.xyz.3 one-boot.xyz.vmin
echo Densifying
args="  --bpv=16 -w ${cells} -t ${cells} -d ${cells} --verbosity=$debug_level"
xyzdensity one-boot.xyz.vmax --out=one-vmax.vol -p 1 $args $boundaries
xyzdensity one-boot.xyz.vint --out=one-vint.vol -p 1 $args $boundaries
xyzdensity one-boot.xyz.vmin --out=one-vmin.vol -p 1 $args $boundaries

xyzdensity --out=one-all.vol -p 1 -b 8 -w ${cells} -t ${cells} -d ${cells} $boundaries \
    one-boot.xyz.vmax \
    one-boot.xyz.vint \
    one-boot.xyz.vmin 

vol_iv -b=2 -c ALPHA_BLENDING --numslicescontrol=ALL -p NONE -C kurt.cmap -o one-all.iv one-all-1.0.vol

scale="--xscale=0.5 --yscale=0.5 --zscale=0.5"
volhdr_edit one-vmax.vol --out=tmp $scale && /bin/mv tmp one-vmax.vol
volhdr_edit one-vint.vol --out=tmp $scale && /bin/mv tmp one-vint.vol
volhdr_edit one-vmin.vol --out=tmp $scale && /bin/mv tmp one-vmin.vol




