#!/bin/bash

make s_bootstrap

./s_bootstrap -P as3-undef.s  500 | s_eigs > as3-01.eigs
./s_bootstrap -P as3-undef.s  500 | s_eigs > as3-02.eigs
./s_bootstrap -P as3-undef.s  500 | s_eigs > as3-03.eigs
./s_bootstrap -P as3-undef.s  500 | s_eigs > as3-04.eigs
./s_bootstrap -P as3-undef.s  500 | s_eigs > as3-05.eigs
./s_bootstrap -P as3-undef.s  500 | s_eigs > as3-06.eigs
./s_bootstrap -P as3-undef.s  500 | s_eigs > as3-07.eigs
./s_bootstrap -P as3-undef.s  500 | s_eigs > as3-08.eigs
./s_bootstrap -P as3-undef.s  500 | s_eigs > as3-09.eigs
./s_bootstrap -P as3-undef.s  500 | s_eigs > as3-10.eigs

rm -f as3.xyz
touch as3.xyz
for file in as3*.eigs; do
    cat $file >> as3.xyz
done

# v3t == V3 eigenvector scaled by Tau3
awk '{print $1,$2,$3}' as3.xyz > as3-v3t.xyz
awk '{print $4,$5,$6}' as3.xyz > as3-v2t.xyz
awk '{print $7,$8,$9}' as3.xyz > as3-v1t.xyz

rm -f as3-??.eigs
rm -f as3.xyz

