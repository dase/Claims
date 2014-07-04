AM_CPPFLAGS= -fPIC \
-I/home/imdb/supports/Theron-5.01.01/Include \
-I/home/imdb/supports/boost_1_53_0 \
-I/home/imdb/supports/boost_1_53_0/boost/serialization \
-I/home/NewHadoop/hadoop/hadoop-1.0.3/src/c++/libhdfs\
-I/home/NewHadoop/java/jdk1.7.0_45/include\
-I/home/NewHadoop/java/jdk1.7.0_45/include/linux

AM_LDFLAGS=-lc -lm -lrt -lboost_serialization

LDADD = ../parsetree/libparsetree.a \
		../Block/libblock.a \
		../utility/libutility.a	\
		LogicalQueryPlan/liblogicalplan.a

noinst_LIBRARIES=libdaemon.a
libdaemon_a_SOURCES = \
	Daemon.cpp 				Daemon.h \
	Executing.cpp 			Executing.h
