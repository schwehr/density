

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
#TARGETS+= AMScrunch

# TESTING TARGETS:
TARGETS+= test_s_bootstrap test_SiteSigma
TARGETS+= test_Density
TARGETS+= test_DensityFlagged

targets: ${TARGETS}

xyzdensity: xyzdensity.C Density.o
	${CXX} -o $@ $< ${CXXFLAGS} Density.o

test_Density: Density.C Density.H
	${CXX} -o $@ $< -DREGRESSION_TEST ${CXXFLAGS} 

test_DensityFlagged: DensityFlagged.C DensityFlagged.H Density.H Density.o
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
