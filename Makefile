CXXFLAGS := -Wall -Wimplicit -pedantic -W -Wstrict-prototypes -Wredundant-decls

ifdef OPTIMIZE
  CXXFLAGS += -O3 -funroll-loops -fexpensive-optimizations -DNDEBUG
else
  CXXFLAGS += -g
endif

CFLAGS := ${CXXFLAGS} -Wimplicit-int -Wimplicit-function-declaration -Wnested-externs

TARGETS:= test_SiteSigma
targets: ${TARGETS}


test_SiteSigma: SiteSigma.C SiteSigma.H
	${CXX} -o $@ $< -DREGRESSION_TEST ${CXXFLAGS}  ${TEST_SCANLINES_OBJ}

clean:
	rm -f blah* foo* *~ ${TARGETS} *.o
