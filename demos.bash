#!/bin/bash

# $Revision$  $Author$  $Date$

#     Copyright (C) 2004  Kurt Schwehr

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

(cd .. && make s_bootstrap xyzdensity )

# ./s_bootstrap -P as1-crypt.s 500 > as1-site-500a.s
# ./s_bootstrap -P as1-crypt.s 500 > as1-site-500b.s
# s_eigs < as1-site-500a.s > a-site.eigs
# s_eigs < as1-site-500b.s > b-site.eigs
# cat ?-site.eigs > site.eigs
# ./eigs2xyz.py site.eigs  > site.xyz

# echo "splot  'site.xyz' \\" > site.gnuplot
# echo "      ,'site.xyz' using 4:5:6\\" >> site.gnuplot
# echo "      ,'site.xyz' using 7:8:9"   >> site.gnuplot


# ./s_bootstrap -p as1-crypt.s 500 > as1-sample-500a.s
# ./s_bootstrap -p as1-crypt.s 500 > as1-sample-500b.s
# s_eigs < as1-sample-500a.s > a-sample.eigs
# s_eigs < as1-sample-500b.s > b-sample.eigs
# cat ?-sample.eigs > sample.eigs
# ./eigs2xyz.py sample.eigs  > sample.xyz

# echo "splot  'sample.xyz' \\" > sample.gnuplot
# echo "      ,'sample.xyz' using 4:5:6\\" >> sample.gnuplot
# echo "      ,'sample.xyz' using 7:8:9"   >> sample.gnuplot

######################################################################
#  SMALLER -- 100 samples
######################################################################


# ./s_bootstrap -P as1-crypt.s 100 > as1-site-100a.s
# s_eigs < as1-site-100a.s > a-site-100.eigs
# cat a-site-100.eigs > site-100.eigs
# ./eigs2xyz.py site-100.eigs  > as1-crypt-site-100.xyz

# echo "splot  'as1-crypt-site-100.xyz' \\" > as1-crypt-site-100.gnuplot
# echo "      ,'as1-crypt-site-100.xyz' using 4:5:6\\" >> as1-crypt-site-100.gnuplot
# echo "      ,'as1-crypt-site-100.xyz' using 7:8:9"   >> as1-crypt-site-100.gnuplot


# ./s_bootstrap -p as1-crypt.s 100 > as1-sample-100a.s
# s_eigs < as1-sample-100a.s > a-sample-100.eigs
# cat a-sample-100.eigs > sample-100.eigs
# ./eigs2xyz.py sample-100.eigs  > as1-crypt-sample-100.xyz

# echo "splot  'as1-crypt-sample-100.xyz' \\" > as1-crypt-sample-100.gnuplot
# echo "      ,'as1-crypt-sample-100.xyz' using 4:5:6\\" >> as1-crypt-sample-100.gnuplot
# echo "      ,'as1-crypt-sample-100.xyz' using 7:8:9"   >> as1-crypt-sample-100.gnuplot


############################## AS2 slump

# ./s_bootstrap -P as2-slump.s 100 > as1-site-100a.s
# s_eigs < as1-site-100a.s > a-site-100.eigs
# cat a-site-100.eigs > site-100.eigs
# ./eigs2xyz.py site-100.eigs  > as2-slump-site-100.xyz

# echo "splot  'as2-slump-site-100.xyz' \\" > as2-slump-site-100.gnuplot
# echo "      ,'as2-slump-site-100.xyz' using 4:5:6\\" >> as2-slump-site-100.gnuplot
# echo "      ,'as2-slump-site-100.xyz' using 7:8:9"   >> as2-slump-site-100.gnuplot


# ./s_bootstrap -p as2-slump.s 100 > as1-sample-100a.s
# s_eigs < as1-sample-100a.s > a-sample-100.eigs
# cat a-sample-100.eigs > sample-100.eigs
# ./eigs2xyz.py sample-100.eigs  > as2-slump-sample-100.xyz

# echo "splot  'as2-slump-sample-100.xyz' \\" > as2-slump-sample-100.gnuplot
# echo "      ,'as2-slump-sample-100.xyz' using 4:5:6\\" >> as2-slump-sample-100.gnuplot
# echo "      ,'as2-slump-sample-100.xyz' using 7:8:9"   >> as2-slump-sample-100.gnuplot

######################################## AS3 - undef

# ./s_bootstrap -P as3-undef.s 100 > as1-site-100a.s
# s_eigs < as1-site-100a.s > a-site-100.eigs
# cat a-site-100.eigs > site-100.eigs
# ./eigs2xyz.py site-100.eigs  > as3-undef-site-100.xyz

# echo "splot  'as3-undef-site-100.xyz' \\" > as3-undef-site-100.gnuplot
# echo "      ,'as3-undef-site-100.xyz' using 4:5:6\\" >> as3-undef-site-100.gnuplot
# echo "      ,'as3-undef-site-100.xyz' using 7:8:9"   >> as3-undef-site-100.gnuplot


../s_bootstrap -p ../as3-undef.s 500 > as1-sample-500a.s
s_eigs < as1-sample-500a.s > a-sample-500.eigs
cat a-sample-500.eigs > sample-500.eigs
../eigs2xyz.py sample-500.eigs  > as3-undef-sample-500.xyz


awk '{print $1,$2,$3}' as3-undef-sample-500.xyz > as3-v3t.xyz
awk '{print $4,$5,$6}' as3-undef-sample-500.xyz > as3-v2t.xyz
awk '{print $7,$8,$9}' as3-undef-sample-500.xyz > as3-v1t.xyz

for file in *.xyz; do
    echo 
    echo xyz to vol: $file

    cmd="../xyzdensity -i $file -o ${file%%.xyz}.vol \
	--xmin=-0.5 --xmax=0.5 \
	--ymin=-0.5 --ymax=0.5 \
	--zmin=-0.5 --zmax=0.5 \
	--width=40 --tall=40 --depth=40 \
	--pack=0 --bpv=8"
    echo $cmd
    $cmd
    #cmd=" ../good.iv"
    #echo $cmd
    perl -p -e "s,density.vol,${file%%.xyz}.vol,g" ../good.iv > ${file%%.xyz}.iv
done




echo "splot  'as3-undef-sample-500.xyz' \\" > as3-undef-sample-500.gnuplot
echo "      ,'as3-undef-sample-500.xyz' using 4:5:6\\" >> as3-undef-sample-500.gnuplot
echo "      ,'as3-undef-sample-500.xyz' using 7:8:9"   >> as3-undef-sample-500.gnuplot
echo "pause -1 \"Hit return to continue\""   >> as3-undef-sample-500.gnuplot

#gnuplot as3-undef-sample-500.gnuplot
