#!/bin/bash
./s_bootstrap -P as1-crypt.s 500 > as1-site-500a.s
./s_bootstrap -P as1-crypt.s 500 > as1-site-500b.s
s_eigs < as1-site-500a.s > a-site.eigs
s_eigs < as1-site-500b.s > b-site.eigs
cat ?-site.eigs > site.eigs
./eigs2xyz.py site.eigs  > site.xyz

echo "splot  'site.xyz' \\" > site.gnuplot
echo "      ,'site.xyz' using 4:5:6\\" >> site.gnuplot
echo "      ,'site.xyz' using 7:8:9"   >> site.gnuplot



./s_bootstrap -p as1-crypt.s 500 > as1-sample-500a.s
./s_bootstrap -p as1-crypt.s 500 > as1-sample-500b.s
s_eigs < as1-sample-500a.s > a-sample.eigs
s_eigs < as1-sample-500b.s > b-sample.eigs
cat ?-sample.eigs > sample.eigs
./eigs2xyz.py sample.eigs  > sample.xyz

echo "splot  'sample.xyz' \\" > sample.gnuplot
echo "      ,'sample.xyz' using 4:5:6\\" >> sample.gnuplot
echo "      ,'sample.xyz' using 7:8:9"   >> sample.gnuplot
