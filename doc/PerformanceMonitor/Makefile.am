AM_CPPFLAGS= -fPIC

AM_LDFLAGS=-lc -lm -lrt -lboost_serialization

LDADD = ../Schema/libschema.a \
		../BlockStreamIterator/libblockstreamiterator.a

noinst_LIBRARIES=libperformancemonitor.a
libperformancemonitor_a_SOURCES = \
	BlockStreamPerformanceMonitorTop.cpp 	BlockStreamPerformanceMonitorTop.h \
	BlockStreamPerformanceTest.cpp 			BlockStreamPerformanceTest.h \
	PerformanceIteratorTop.cpp 				PerformanceIteratorTop.h

#libperformancemonitor_a_LIBADD = \
#					../Schema/libschema.a \
#					../BlockStreamIterator/libblockstreamiterator.a
					