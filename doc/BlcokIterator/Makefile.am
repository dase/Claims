AM_CPPFLAGS= -fPIC \
-I/home/NewHadoop/hadoop/hadoop-1.0.3/src/c++/libhdfs\
-I/home/NewHadoop/java/jdk1.7.0_45/include\
-I/home/NewHadoop/java/jdk1.7.0_45/include/linux \
-I/home/imdb/tools/Theron-5.01.01/Include \
-I/home/imdb/supports/boost_1_53_0 \
-I/home/imdb/supports/boost_1_53_0/boost/serialization

AM_LDFLAGS=-lc -lm -lrt

LDADD = ../Block/libblock.a \
		../Schema/libschema.a \
		../iterator/libiterator.a

noinst_LIBRARIES=libblockiterator.a
libblockiterator_a_SOURCES =\
					BlockIteratorFix.cpp BlockIteratorFix.h \
					CopyBasedFilterIteratorBlockFix.cpp CopyBasedFilterIteratorBlockFix.h \
					SingleColumnScanBlockIterator.cpp SingleColumnScanBlockIterator.h

#libblockiterator_a_LIBADD = ../Block/libblock.a ../Schema/libschema.a ../iterator/libiterator.a
