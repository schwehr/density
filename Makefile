# $Revision$  $Author$  $Date$

# Copyright (C) 2004  Kurt Schwehr
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#
# FIX: assumes we are on Mac OSX 10.3 using fink with the following installed:
# 
#  simvoleon21, soqt21, gengetopt (>=2.12.1), gsl, doxygen
#
#  Suggested to install Lisa Tauxe's pmag (>=1.8).  Sorry, it is not in fink yet.

# If you do not have bash in /bin, go get your admin to install it!!!!
SHELL=/bin/bash

help:
	@echo
	@echo " USAGE:"
	@echo 
	@echo "  make targets     - Build with debugging enabled and then test"
	@echo "  make clean       - Clean up all the moose droppings"
	@echo "  make docs        - Generate doxygen docs"
	@echo "  make man         - Generate section 1 man pages"
	@echo "  make tar         - Build a distribution"
	@echo "  make check       - Search for all known code issuse (FIX tags)"
	@echo "  make info        - Display a number of internal make variables"
	@echo
	@echo "  Add 'OPTIMIZE=1' - Build with optimizations enabled and then test"



CXXFLAGS := -Wall -Wimplicit -pedantic -W -Wstrict-prototypes -Wredundant-decls
CXXFLAGS += -I/sw/include -L/sw/lib


# FIX: gcc can tell us the endian 
#  touch foo.h && cpp -dM foo.h
#
#define __BIG_ENDIAN__ 1
#define _BIG_ENDIAN 1

# This is a cheap hack to prebuild endian
ENDIAN_DUMMY:=${shell make -f Makefile.endian}
CXXFLAGS += -D${shell ./endian}

FFLAGS := -g -Wall

# Make is shut up about GSL using long double
CXXFLAGS += -Wno-long-double

ifdef OPTIMIZE
  CXXFLAGS += -O3 -funroll-loops -fexpensive-optimizations -DNDEBUG
  CXXFLAGS += -ffast-math -mtune=G4 -mcpu=G4 -mpowerpc

# Fast is specific for G4 and G5 cpus, here only for the G4
#  CXXFLAGS += -mcpu=7450 -fast
#  Programs crashing with -fast
else
  CXXFLAGS += -g3 -O0
  CXXFLAGS += -D_GLIBCXX_DEBUG
endif

CFLAGS := ${CXXFLAGS} -Wimplicit-int -Wimplicit-function-declaration -Wnested-externs

# These are programs that give --help for help2man
GENGETOPT_BINS := s_bootstrap
GENGETOPT_BINS += simpleview
GENGETOPT_BINS += xyzdensity
GENGETOPT_BINS += xyz_iv
GENGETOPT_BINS += xyzvol_cmp
GENGETOPT_BINS += vol2vol
GENGETOPT_BINS += volhdr_edit
GENGETOPT_BINS += volinfo
GENGETOPT_BINS += vol_iv
GENGETOPT_BINS += volmakecmap

# Those without gengetopt
SIMPLE_BINS := endian
SIMPLE_BINS += histogram
SIMPLE_BINS += is_equal
SIMPLE_BINS += makeCDF
#SIMPLE_BINS+= AMScrunch

BINS := ${GENGETOPT_BINS}
BINS += ${SIMPLE_BINS}


# TESTING TARGETS:
TEST_BINS += test_s_bootstrap
TEST_BINS += test_SiteSigma
TEST_BINS += test_VolHeader
TEST_BINS += test_Density
TEST_BINS += test_DensityFlagged
TEST_BINS += test_Cdf
TEST_BINS += test_Eigs

TARGETS := ${BINS} ${TEST_BINS}

targets-no-test: ${BINS}
targets: ${TARGETS} test



######################################################################
# GENGETOPT programs to build

%.c: %.ggo
	gengetopt --input=$< --file-name=${<:.ggo=} --unamed-opts

s_bootstrap: s_bootstrap.C SiteSigma.o Bootstrap.o s_bootstrap_cmd.o Eigs.o
	${CXX} -o $@ $^ ${CXXFLAGS} -Wno-long-double -lgsl -lgslcblas

simpleview: simpleview_cmd.o simpleview.C
	${CXX} -o $@ $^  -I/sw/include/qt ${CXXFLAGS} -lsimage -lCoin -lSoQt -lSimVoleon -lqt-mt -bind_at_load -Wno-long-long

xyzdensity: xyzdensity.C Density.o VolHeader.o xyzdensity_cmd.o
	${CXX} -o $@ $^ ${CXXFLAGS}

xyz_iv: xyz_iv_cmd.o xyz_iv.C
	${CXX} -o $@ $^ ${CXXFLAGS}

xyzvol_cmp: xyzvol_cmp.C Density.o VolHeader.o xyzvol_cmp_cmd.o
	${CXX} -o $@ $^ ${CXXFLAGS}

vol2vol: vol2vol.C VolHeader.o vol2vol_cmd.o Density.o
	${CXX} -o $@ $^  ${CXXFLAGS}

volinfo: volinfo.C VolHeader.o volinfo_cmd.o Density.o
	${CXX} -o $@ $^  ${CXXFLAGS}

volhdr_edit: volhdr_edit.C VolHeader.o volhdr_edit_cmd.o
	${CXX} -o $@ $^ ${CXXFLAGS}

vol_iv: vol_iv.C vol_iv_cmd.o
	${CXX} -o $@ $^ ${CXXFLAGS}

volmakecmap: volmakecmap.C volmakecmap_cmd.o
	${CXX} -o $@ $^ ${CXXFLAGS} -lCoin

######################################################################
# Regular commands sans GENGETOPT

# Do not need to be here unless they need some extra flags

######################################################################
# Test Programs

test_Eigs: Eigs.C 
	${CXX} -o $@ $< -Wno-long-double -DREGRESSION_TEST ${CXXFLAGS}  -lgsl -lgslcblas

test_Cdf: Cdf.C Cdf.H
	${CXX} -o $@ $< -DREGRESSION_TEST ${CXXFLAGS}

test_VolHeader: VolHeader.C VolHeader.H
	${CXX} -o $@ $< -DREGRESSION_TEST ${CXXFLAGS}

test_Density: Density.C Density.H VolHeader.o
	${CXX} -o $@ $< -DREGRESSION_TEST ${CXXFLAGS} VolHeader.o

test_DensityFlagged: DensityFlagged.C DensityFlagged.H Density.H Density.o VolHeader.o
	${CXX} -o $@ $< -DREGRESSION_TEST ${CXXFLAGS} Density.o VolHeader.o

test_SiteSigma: SiteSigma.C SiteSigma.H
	${CXX} -o $@ $< -DREGRESSION_TEST ${CXXFLAGS} 

test_s_bootstrap: s_bootstrap.C SiteSigma.o Bootstrap.o
	${CXX} -o $@ $< -DREGRESSION_TEST ${CXXFLAGS} SiteSigma.o Bootstrap.o -lgsl -lgslcblas


######################################################################
# Weird tweaks

simpleview.help2man: simpleview simpleview.help2man.in
	/bin/cp simpleview.help2man.in simpleview.help2man
	@echo Generating file format text
	./simpleview -l >> simpleview.help2man
	@echo FIX: need to cleanup the tables following two tables >> simpleview.help2man
	@echo Generating keyboard shortcut text
	./simpleview -k >> simpleview.help2man

######################################################################
# Worker Bees

# TARGETS includes TEST_BINS
test: ${TARGETS}
	@for file in ${TEST_BINS}; do \
		echo ;\
		echo $$file ;\
		echo ;\
		export TESTING=yes ;\
		./$$file ;\
		done
	@echo 
	./regression.bash
	@echo 
	@echo SUCCESS!!
	@echo All tests passed in "${shell pwd}"

docs:
	doxygen

# to view a man page:
# groff -Tascii -man xyzdensity.1 | less
# Arg... this fails with the fink groff installed.  Gets weird escape sequences
man: ${GENGETOPT_BINS} simpleview.help2man
	mkdir -p doc/man/man1
	for file in ${GENGETOPT_BINS}; do echo Processing $$file;help2man -N ./$$file --opt-include $$file.help2man > doc/man/man1/$$file.1; done

man2html: man
	cd doc/man/man1 && for file in *.1; do groff -Tascii -man $$file | man2html > $${file%%.1}.html; done

install-web: install-web-doxy install-web-man2html
install-web-doxy: docs
	scp doc/html/* kds:www/software/density/html
install-web-man2html: man2html
	scp doc/man/man1/*.html kds:www/software/density/man


# Need these so we can make sure the *_cmd.[ch] files exist so do not need gengetopt
GGOS:=${wildcard *.ggo}
GEN_CFILES := ${GGOS:.ggo=.c}
GEN_HFILES := ${GGOS:.ggo=.h}


VERSION := ${shell cat VERSION}
NAME := density
TARNAME := ${NAME}-${VERSION}
tar: ${GEN_CFILES} ${GENGETOPT_BINS} test
	rm -rf ${TARNAME} ${TARNAME}.tar ${TARNAME}.tar.bz2
	mkdir ${TARNAME}
	@echo
	cp *.{C,H,ggo,c,h,help2man,help2man.in,bash} ${TARNAME}/
	@echo
	rm -f HEADER.html HEADER-${VERSION}.hmtl
	wget http://schwehr.org/software/density/HEADER.html
	mv HEADER.html ${TARNAME}/HEADER.html
	@echo
	cp AUTHOR ChangeLog Doxyfile INSTALL LICENSE.LGPL ${TARNAME}/
	cp Makefile Makefile.endian README.txt TODO VERSION ${TARNAME}/
	@echo Copying example data for one.bash and bootvolume-thesis.bash
	cp as1-crypt.s as2-slump.s as3-undef.s ${TARNAME}/
	@echo
	tar cf ${TARNAME}.tar ${TARNAME}
	bzip2 -9 ${TARNAME}.tar
	rm -rf ${TARNAME}

coffee:
	@echo Go make your own!

check:
	@echo
	@echo "**************************************"
	@echo "* Known issues currently in the code *"
	@echo "**************************************"
	@echo
	@grep -n FIX *.{C,H,ggo,help2man} Makefile | grep -v grep

clean: clean-runs
	rm -rf ${TARGETS} *~ *.o
	rm -f *_cmd.[ch]
	rm -f .*~

# Stuff that running make test or a bash script leaves behind
clean-runs:
	rm -rf blah* foo* *.xyz *.eigs *.cdf [0-9]x[0-9]*test?.vol
	rm -f .*~
	rm -f as*.xyz* as*.vol
	rm -f [0-9].{vol,s,xyz}
	rm -f [0-9][0-9].{vol,s,xyz*}
	rm -f one.xyz* one-* one.cmap one.s
	rm -f current.cmap as[0-9]-*all-1.0.iv as?-?????-????-8.iv vol.iv
	rm -f *.vol.cmp
	rm -f test3.vol*

real-clean: clean
	rm -rf doc
	rm -f .gdb_history

# FIX: make optimize print on or off
info:
	@echo " NAME            -  " ${NAME}
	@echo " VERSION         -  " ${VERSION}
	@echo " OPTIMIZE        -  " ${OPTIMIZE}
	@echo " CXXFLAGS        -  " ${CXXFLAGS}
	@echo " CFLAGS          -  " ${CFLAGS}
	@echo " TEST_BINS       -  " ${TEST_BINS}
	@echo " SIMPLE_BINS     -  " ${SIMPLE_BINS}
	@echo " GENGETOPT GGOs  -  " ${GGOS:.ggo=}
	@echo " GENGETOPT_BINS  -  " ${GENGETOPT_BINS}


# When you need to "RTFS/RTFM"... get the latest
CVSROOT_SIM:= ":pserver:cvs@cvs.coin3d.org:/export/cvsroot"
sim-cvs-login:
	-mkdir tmp
	cd tmp && export CVSROOT=${CVSROOT_SIM} && cvs login
get-cvs-coin:
	-mkdir tmp
	cd tmp && export CVSROOT=${CVSROOT_SIM} && cvs get Coin
get-cvs-soguiexamples:
	-mkdir tmp
	cd tmp && export CVSROOT=${CVSROOT_SIM} && cvs -z3 get SoGuiExamples
get-cvs-soqt:
	-mkdir tmp
	cd tmp && export CVSROOT=${CVSROOT_SIM} && cvs -z3 get SoQt
get-cvs-dime:
	-mkdir tmp
	cd tmp && export CVSROOT=${CVSROOT_SIM} && cvs -z3 get dime
get-cvs-smallchange:
	-mkdir tmp
	cd tmp && export CVSROOT=${CVSROOT_SIM} && cvs -z3 get SmallChange
get-cvs-voleon:
	-mkdir tmp
	cd tmp && export CVSROOT=${CVSROOT_SIM} && cvs -z3 get SIMVoleon
get-cvs-soxt:
	-mkdir tmp
	cd tmp && export CVSROOT=${CVSROOT_SIM} && cvs -z3 get SoXt

# Arbitrary color for programs
install-acoc:
	@if [ ! -e ~/.acoc.conf ]; then echo "Installing...";/bin/cp .acoc.conf ~/; \
	else echo "Already there.  Not copying."; fi
install-acoc-force:
	@echo Copying acoc.conf into your home directory.  WILL OVERWRITE
	/bin/cp .acoc.conf ~/



############################################################
# Dependencies - FIX: do a real depend with gcc/g++
############################################################

# Thses endian ones are special
Density.o: endian
VolHeader.o: endian

xyzdensity: debug.H
VolHeader.o: VolHeader.C VolHeader.H
