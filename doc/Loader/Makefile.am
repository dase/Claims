AM_CPPFLAGS= -fPIC\
-I/home/imdb/supports/boost_1_53_0 \
-I/home/imdb/supports/boost_1_53_0/boost/serialization \
-I/home/NewHadoop/hadoop/hadoop-1.0.3/src/c++/libhdfs\
-I/home/NewHadoop/java/jdk1.7.0_45/include\
-I/home/NewHadoop/java/jdk1.7.0_45/include/linux \
-I/home/imdb/tools/Theron-5.01.01/Include

AM_LDFLAGS=-lc -lm -lrt 

LDADD = ../Block/libblock.a \
		../Catalog/libcatalog.a \
		../Schema/libschema.a

noinst_LIBRARIES=libloader.a
libloader_a_SOURCES = \
Hdfsloader.cpp Hdfsloader.h \
Hdfsconnector.cpp Hdfsconnector.h