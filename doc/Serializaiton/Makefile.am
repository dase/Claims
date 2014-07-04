AM_CPPFLAGS= -fPIC \
-I/home/NewHadoop/hadoop/hadoop-1.0.3/src/c++/libhdfs\
-I/home/NewHadoop/java/jdk1.7.0_45/include\
-I/home/NewHadoop/java/jdk1.7.0_45/include/linux \
-I/home/imdb/supports/Theron-5.01.01/Include \
-I/home/imdb/supports/boost_1_53_0 \
-I/home/imdb/supports/boost_1_53_0/boost/serialization

AM_LDFLAGS=-lc -lm -lrt -lboost_serializaiton -lhdfs -ljvm

LDADD = ../Block/libblock.a \
		../Schema/libschema.a \
		../iterator/libiterator.a \
		../BlockStreamIterator/libblockstreamiterator.a \
		../BlockStreamIterator/ParallelBlockStreamIterator/libparallelblockstreamiterator.a \
		../PerformanceMonitor/libperformancemonitor.a \
		../IndexManager/libindexmanager.a

noinst_LIBRARIES=libserialization.a
libserialization_a_SOURCES = \
	RegisterDerivedClass.cpp 		RegisterDerivedClass.h \
	Serialization_column_type.h 						   \
	Serialization_Schema.h

#libserialization_a_LIBADD = ../Schema/libschema.a \
#						../iterator/libiterator.a \
#						../BlockStreamIterator/libblockstreamiterator.a \
#						../PerformanceMonitor/libperformancemonitor.a \
#						../IndexManager/libindexmanager.a