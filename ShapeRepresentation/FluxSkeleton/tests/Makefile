
include ../Common

CPP = $(CXX) $(CPPFLAGS)


tests := testPoint testVector testLineSeg testCircSeg testDiscreteSegCurve testDiscreteShape testDistanceTransform testSimpleShapeMaker testDivergenceMap testDivergenceSkeletonMaker

sources := $(wildcard *.cpp)

.PHONY: clean .cvsignore

all: $(tests)
include $(sources:.cpp=.d)

%.d: %.cpp
	$(CC) -MM $(CPPFLAGS) $< | sed 's/$*.o/& $@/g' | sed 's/$*.o/& $*/g' > $@

.cvsignore: 
	echo \*.d \*.o \*~ \*.a $(tests) > $@

read_ppm.o: read_ppm.c read_ppm.h
	$(CC) $(CPPFLAGS) -c read_ppm.c

testDivergenceSkeletonMaker: testDivergenceSkeletonMaker.cpp read_ppm.o
	$(CPP) -o $@ -lm -lppm -lpgm -lpbm read_ppm.o  \
		testDivergenceSkeletonMaker.cpp ../lib/libsg.a 

testDivergenceMap: testDivergenceMap.cpp read_ppm.o
	$(CPP) -o $@ -lm -lppm -lpgm -lpbm read_ppm.o  \
		testDivergenceMap.cpp ../lib/libsg.a 

testSimpleShapeMaker: testSimpleShapeMaker.cpp read_ppm.o
	$(CPP) -o $@ -lm -lppm -lpgm -lpbm read_ppm.o  testSimpleShapeMaker.cpp ../lib/libsg.a 

testDistanceTransform: ../lib/libsg.a  testDistanceTransform.cpp 
	$(CPP) testDistanceTransform.cpp ../lib/libsg.a -o $@

testDiscreteShape: ../lib/libsg.a  testDiscreteShape.cpp 
	$(CPP) testDiscreteShape.cpp ../lib/libsg.a -o $@

testPSDrawable:  testPSDrawable.cpp
	$(CPP) testPSDrawable.cpp -o $@

testDiscreteSegCurve:  ../lib/libsg.a  testDiscreteSegCurve.cpp  
	$(CPP) testDiscreteSegCurve.cpp ../lib/libsg.a -o $@

testCircSeg:  ../lib/libsg.a testCircSeg.cpp
	$(CPP) testCircSeg.cpp ../lib/libsg.a -o $@

testLineSeg:  ../lib/libsg.a  testLineSeg.cpp
	$(CPP) testLineSeg.cpp ../lib/libsg.a  -o $@

testPoint:  testPoint.cpp
	$(CPP) testPoint.cpp -o $@

testVector:  testVector.cpp
	$(CPP) testVector.cpp -o $@

clean:
	@echo "Cleaning tests directory..."
	@$(RM) *.o  *~ $(tests)
