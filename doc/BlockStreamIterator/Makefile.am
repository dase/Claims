AM_CPPFLAGS= -fPIC \
-I/home/imdb/supports/boost_1_53_0 \
-I/home/imdb/supports/boost_1_53_0/boost/serialization \
-I/home/imdb/tools/Theron-5.01.01/Include \
-I/home/NewHadoop/hadoop/hadoop-1.0.3/src/c++/libhdfs\
-I/home/NewHadoop/java/jdk1.7.0_45/include\
-I/home/NewHadoop/java/jdk1.7.0_45/include/linux

#此文件需要包含boost序列化库
#本模块最好依赖的是这些库：executor schema block utility common storage
#其中iterator可以合并进来
AM_LDFLAGS=-lc -lm -lrt -lboost_serialization

LDADD = ../Executor/libexecutor.a \
		../Schema/libschema.a \
		../Block/libblock.a \
		../utility/libutility.a \
		../common/libcommon.a \
		../iterator/libiterator.a \
		ParallelBlockStreamIterator/libparallelblockstreamiterator.a \
		/home/NewHadoop/hadoop/hadoop-1.0.3/c++/Linux-amd64-64/lib/libhdfs.a\
		/home/NewHadoop/java/jdk1.7.0_45/jre/lib/amd64/server/libjvm.so\
		/home/NewHadoop/hadoop/hadoop-1.0.3/c++/Linux-amd64-64/lib/libhdfs.so

noinst_LIBRARIES=libblockstreamiterator.a
libblockstreamiterator_a_SOURCES = \
	BlockStreamExchangeBase.cpp 				BlockStreamExchangeBase.h\
	BlockStreamExchangeLowerBase.cpp 			BlockStreamExchangeLowerBase.h\
	BlockStreamFilter.cpp 						BlockStreamFilter.h\
	BlockStreamIteratorBase.cpp 				BlockStreamIteratorBase.h\
	BlockStreamPrint.cpp 						BlockStreamPrint.h\
	BlockStreamRandomMemAccess.cpp				BlockStreamRandomMemAccess.h\
	BlockStreamResultCollector.cpp 				BlockStreamResultCollector.h\
	BlockStreamSingleColumnScan.cpp 			BlockStreamSingleColumnScan.h\
	ExpandableBlockStreamIteratorBase.cpp 		ExpandableBlockStreamIteratorBase.h
			
SUBDIRS = ParallelBlockStreamIterator
DIST_SUBDIRS = ParallelBlockStreamIterator 							