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




CXXFLAGS := -Wall -Wimplicit -pedantic -W -Wstrict-prototypes -Wredundant-decls
CXXFLAGS += -I/sw/include -L/sw/lib

# This is a cheap hack to prebuild endian
ENDIAN_DUMMY:=${shell make -f Makefile.endian}
CXXFLAGS += -D${shell ./endian}

FFLAGS := -g -Wall

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

BINS := makeCDF histogram s_bootstrap xyzdensity endian
BINS += volinfo
BINS += simpleview
#BINS+= AMScrunch

# TESTING TARGETS:
TEST_BINS += test_s_bootstrap
TEST_BINS += test_SiteSigma
TEST_BINS += test_VolHeader
TEST_BINS += test_Density
TEST_BINS += test_DensityFlagged
TEST_BINS += test_Cdf

TARGETS:=${BINS} ${TEST_BINS}

targets: ${TARGETS} test


xyzdensity_cmd.c: xyzdensity_cmd.h
xyzdensity_cmd.h: xyzdensity_cmd.ggo
	gengetopt --input=$< --file-name=${<:.ggo=}
xyzdensity_cmd.o: xyzdensity_cmd.c xyzdensity_cmd.ggo
	${CXX} -c $< ${CXXFLAGS}

xyzdensity: xyzdensity.C Density.o VolHeader.o xyzdensity_cmd.o
	${CXX} -o $@ $^ ${CXXFLAGS}

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

s_bootstrap: s_bootstrap.C SiteSigma.o Bootstrap.o
	${CXX} -o $@ $<  ${CXXFLAGS} SiteSigma.o Bootstrap.o -lgsl -lgslcblas

endian: endian.C
	${CXX} -g -Wall $< -o $@

clean:
	rm -f blah* foo* *~ ${TARGETS} *.o *.xyz *.eigs *.cdf [0-9]x[0-9]*test?.vol

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

