#!/bin/bash

make s_bootstrap

rm -rf as3*.eigs as3*.xyz

./s_bootstrap -p as3-undef.s  500 | s_eigs > as3-01.eigs
./s_bootstrap -p as3-undef.s  500 | s_eigs > as3-02.eigs
./s_bootstrap -p as3-undef.s  500 | s_eigs > as3-03.eigs
./s_bootstrap -p as3-undef.s  500 | s_eigs > as3-04.eigs
./s_bootstrap -p as3-undef.s  500 | s_eigs > as3-05.eigs
./s_bootstrap -p as3-undef.s  500 | s_eigs > as3-06.eigs
./s_bootstrap -p as3-undef.s  500 | s_eigs > as3-07.eigs
./s_bootstrap -p as3-undef.s  500 | s_eigs > as3-08.eigs
./s_bootstrap -p as3-undef.s  500 | s_eigs > as3-09.eigs
./s_bootstrap -p as3-undef.s  500 | s_eigs > as3-10.eigs

rm -f as3.xyz
touch as3.xyz
for file in as3*.eigs; do
    cat $file >> as3.eigs
done
ls -l as3.eigs
wc -l as3.eigs
./eigs2xyz.py as3.eigs > as3.xyz

# v3t == V3 eigenvector scaled by Tau3
awk '{print $1,$2,$3}' as3.xyz > as3-v3t.xyz
awk '{print $4,$5,$6}' as3.xyz > as3-v2t.xyz
awk '{print $7,$8,$9}' as3.xyz > as3-v1t.xyz

rm -f as3-??.eigs
rm -f as3.xyz


########################################

rm -rf as2*.eigs as2*.xyz

./s_bootstrap -p as2-slump.s  500 | s_eigs > as2-01.eigs
./s_bootstrap -p as2-slump.s  500 | s_eigs > as2-02.eigs
./s_bootstrap -p as2-slump.s  500 | s_eigs > as2-03.eigs
./s_bootstrap -p as2-slump.s  500 | s_eigs > as2-04.eigs
./s_bootstrap -p as2-slump.s  500 | s_eigs > as2-05.eigs
./s_bootstrap -p as2-slump.s  500 | s_eigs > as2-06.eigs
./s_bootstrap -p as2-slump.s  500 | s_eigs > as2-07.eigs
./s_bootstrap -p as2-slump.s  500 | s_eigs > as2-08.eigs
./s_bootstrap -p as2-slump.s  500 | s_eigs > as2-09.eigs
./s_bootstrap -p as2-slump.s  500 | s_eigs > as2-10.eigs

rm -f as2.xyz
touch as2.xyz
for file in as2*.eigs; do
    cat $file >> as2.eigs
done
ls -l as2.eigs
wc -l as2.eigs
./eigs2xyz.py as2.eigs > as2.xyz

# v3t == V3 eigenvector scaled by Tau3
awk '{print $1,$2,$3}' as2.xyz > as2-v3t.xyz
awk '{print $4,$5,$6}' as2.xyz > as2-v2t.xyz
awk '{print $7,$8,$9}' as2.xyz > as2-v1t.xyz

rm -f as2-??.eigs
rm -f as2.xyz

