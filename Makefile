

CXXFLAGS := -Wall -Wimplicit -pedantic -W -Wstrict-prototypes -Wredundant-decls
CXXFLAGS += -I/sw/include -L/sw/lib

# This is a cheap hack to prebuild endian
ENDIAN_DUMMY:=${shell make -f Makefile.endian}
CXXFLAGS += -D${shell ./endian}

FFLAGS := -g -Wall

ifdef OPTIMIZE
  CXXFLAGS += -O3 -funroll-loops -fexpensive-optimizations -DNDEBUG
else
  CXXFLAGS += -g
endif

CFLAGS := ${CXXFLAGS} -Wimplicit-int -Wimplicit-function-declaration -Wnested-externs

TARGETS:= makeCDF histogram s_bootstrap xyzdensity endian
TAGGETS+= volinfo
#TARGETS+= AMScrunch

# TESTING TARGETS:
TARGETS+= test_s_bootstrap
TARGETS+= test_SiteSigma
TARGETS+= test_VolHeader
TARGETS+= test_Density
TARGETS+= test_DensityFlagged

targets: ${TARGETS}

xyzdensity: xyzdensity.C Density.o
	${CXX} -o $@ $< ${CXXFLAGS} Density.o

test_VolHeader: VolHeader.C VolHeader.H
	${CXX} -o $@ $< -DREGRESSION_TEST ${CXXFLAGS}

test_Density: Density.C Density.H VolHeader.o
	${CXX} -o $@ $< -DREGRESSION_TEST ${CXXFLAGS} VolHeader.o

test_DensityFlagged: DensityFlagged.C DensityFlagged.H Density.H Density.o VolHeader.o
	${CXX} -o $@ $< -DREGRESSION_TEST ${CXXFLAGS} Density.o

test_SiteSigma: SiteSigma.C SiteSigma.H
	${CXX} -o $@ $< -DREGRESSION_TEST ${CXXFLAGS} 

test_s_bootstrap: s_bootstrap.C SiteSigma.o Bootstrap.o
	${CXX} -o $@ $< -DREGRESSION_TEST ${CXXFLAGS} SiteSigma.o Bootstrap.o -lgsl -lgslcblas

s_bootstrap: s_bootstrap.C SiteSigma.o Bootstrap.o
	${CXX} -o $@ $<  ${CXXFLAGS} SiteSigma.o Bootstrap.o -lgsl -lgslcblas

endian: endian.C
	${CXX} -g -Wall $< -o $@

clean:
	rm -f blah* foo* *~ ${TARGETS} *.o

Density.o: endian

simpleview: simpleview.C
	${CXX} -o $@ $<  -I/sw/include/qt ${CXXFLAGS} -lsimage -lCoin -lSoQt -lSimVoleon -lqt-mt

volinfo: volinfo.C VolHeader.o
	${CXX} -o $@ $^  ${CXXFLAGS}