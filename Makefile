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
GENGETOPT_BINS += xyzdensity
GENGETOPT_BINS += xyzvol_cmp
GENGETOPT_BINS += volinfo
GENGETOPT_BINS += vol2vol
GENGETOPT_BINS += volhdr_edit

BINS := ${GENGETOPT_BINS}
BINS += makeCDF
BINS += histogram
BINS += endian
BINS += simpleview
#BINS+= AMScrunch

# TESTING TARGETS:
TEST_BINS += test_s_bootstrap
TEST_BINS += test_SiteSigma
TEST_BINS += test_VolHeader
TEST_BINS += test_Density
TEST_BINS += test_DensityFlagged
TEST_BINS += test_Cdf
TEST_BINS += test_Eigs

TARGETS:=${BINS} ${TEST_BINS}

targets: ${TARGETS} test


xyzdensity_cmd.c: xyzdensity_cmd.h
xyzdensity_cmd.h: xyzdensity_cmd.ggo
	gengetopt --input=$< --file-name=${<:.ggo=}
xyzdensity_cmd.o: xyzdensity_cmd.c xyzdensity_cmd.ggo
	${CXX} -c $< ${CXXFLAGS}

xyzdensity: xyzdensity.C Density.o VolHeader.o xyzdensity_cmd.o
	${CXX} -o $@ $^ ${CXXFLAGS}

volhdr_edit_cmd.c: volhdr_edit_cmd.h
volhdr_edit_cmd.h: volhdr_edit_cmd.ggo
	gengetopt --input=$< --file-name=${<:.ggo=}
volhdr_edit_cmd.o: volhdr_edit_cmd.c volhdr_edit_cmd.ggo
	${CXX} -c $< ${CXXFLAGS}

volhdr_edit: volhdr_edit.C VolHeader.o volhdr_edit_cmd.o
	${CXX} -o $@ $^ ${CXXFLAGS}



xyzvol_cmp_cmd.c: xyzvol_cmp_cmd.h
xyzvol_cmp_cmd.h: xyzvol_cmp_cmd.ggo
	gengetopt --input=$< --file-name=${<:.ggo=}
xyzvol_cmp_cmd.o: xyzvol_cmp_cmd.c xyzvol_cmp_cmd.ggo
	${CXX} -c $< ${CXXFLAGS}
xyzvol_cmp: xyzvol_cmp.C Density.o VolHeader.o xyzvol_cmp_cmd.o
	${CXX} -o $@ $^ ${CXXFLAGS}

#Eigs.H
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

s_bootstrap_cmd.c: s_bootstrap_cmd.h
s_bootstrap_cmd.h: s_bootstrap_cmd.ggo
	gengetopt --input=$< --file-name=${<:.ggo=}
s_bootstrap_cmd.o: s_bootstrap_cmd.c s_bootstrap_cmd.ggo
	${CXX} -c $< ${CXXFLAGS}

s_bootstrap: s_bootstrap.C SiteSigma.o Bootstrap.o s_bootstrap_cmd.o Eigs.o
	${CXX} -o $@ $^ ${CXXFLAGS} -Wno-long-double -lgsl -lgslcblas

endian: endian.C
	${CXX} -g -Wall $< -o $@

Density.o: endian

simpleview: simpleview.C
	${CXX} -o $@ $<  -I/sw/include/qt ${CXXFLAGS} -lsimage -lCoin -lSoQt -lSimVoleon -lqt-mt

volinfo_cmd.c: volinfo_cmd.h
volinfo_cmd.h: volinfo_cmd.ggo
	gengetopt --input=$< --file-name=${<:.ggo=}
volinfo_cmd.o: volinfo_cmd.c volinfo_cmd.ggo
	${CXX} -c $< ${CXXFLAGS}

volinfo: volinfo.C VolHeader.o volinfo_cmd.o Density.o
	${CXX} -o $@ $^  ${CXXFLAGS}


vol2vol_cmd.c: vol2vol_cmd.h
vol2vol_cmd.h: vol2vol_cmd.ggo
	gengetopt --input=$< --file-name=${<:.ggo=}
vol2vol_cmd.o: vol2vol_cmd.c vol2vol_cmd.ggo
	${CXX} -c $< ${CXXFLAGS}

vol2vol: vol2vol.C VolHeader.o vol2vol_cmd.o Density.o
	${CXX} -o $@ $^  ${CXXFLAGS}


test: ${TEST_BINS}
	@for file in ${TEST_BINS}; do \
		echo ;\
		echo $$file ;\
		echo ;\
		export TESTING=yes ;\
		./$$file ;\
		done
	@echo 
	@echo SUCCESS!!
	@echo All tests passed in "${shell pwd}"

docs:
	doxygen

# to view a man page:
# groff -Tascii -man xyzdensity.1 | less
man: ${GENGETOPT_BINS}
	mkdir -p doc/man/man1
	for file in ${GENGETOPT_BINS}; do echo Processing $$file;help2man -N ./$$file --opt-include $$file.help2man > doc/man/man1/$$file.1; done



VERSION := ${shell cat VERSION}
NAME := density
TARNAME := ${NAME}-${VERSION}
tar: ${GENGETOPT_BINS}
	rm -rf ${TARNAME}
	mkdir ${TARNAME}
	@echo
	cp *.{C,H,ggo,c,h,help2man,bash} ${TARNAME}/
	@echo
	cp AUTHOR Makefile Doxyfile LICENSE.LGPL README.txt VERSION demos.bash good.iv ${TARNAME}/
	@echo
	tar cf ${TARNAME}.tar ${TARNAME}
	bzip2 -9 ${TARNAME}.tar
	rm -rf ${TARNAME}


coffee:
	@echo Get your own.

# Credit to Dan Goldman my programming partner for compilers many years ago
check:
	@echo
	@echo "**************************************"
	@echo "* Known issues currently in the code *"
	@echo "**************************************"
	@echo
	@grep -n FIX *.{C,H,ggo,help2man} Makefile | grep -v grep

clean:
	rm -f blah* foo* *~ ${TARGETS} *.o *.xyz *.eigs *.cdf [0-9]x[0-9]*test?.vol
	rm -f *_cmd.[ch]
	rm -f .*~
	rm -f as*.xyz* as*.vol
	rm -f ${BINS}

real-clean: clean
	rm -rf doc


############################################################
# Dependencies - FIX: do a real depend with gcc/g++
############################################################

xyzdensity: debug.H
VolHeader.o: VolHeader.C VolHeader.H
