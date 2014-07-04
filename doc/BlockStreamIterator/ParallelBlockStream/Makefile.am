AM_CPPFLAGS= -fPIC \
-I/home/imdb/supports/boost_1_53_0 \
-I/home/imdb/supports/boost_1_53_0/boost/serialization \
-I/home/imdb/supports/Theron-5.01.01/Include \
-I/home/NewHadoop/hadoop/hadoop-1.0.3/src/c++/libhdfs\
-I/home/NewHadoop/java/jdk1.7.0_45/include\
-I/home/NewHadoop/java/jdk1.7.0_45/include/linux

#此文件需要包含boost序列化库
AM_LDFLAGS=-lc -lm -lrt -lboost_serialization -lpthread

#本模块最好依赖的是这些库：executor schema block utility common storage
#其中iterator可以合并进来,还需要包含blockiterator的库
LDADD = ../../Executor/libexecutor.a \
		../../Schema/libschema.a \
		../../Block/libblock.a \
		../../utility/libutility.a \
		../../common/libcommon.a \
		../../iterator/libiterator.a \
		../../storage/libstorage.a \
		/home/NewHadoop/hadoop/hadoop-1.0.3/c++/Linux-amd64-64/lib/libhdfs.a\
		/home/NewHadoop/java/jdk1.7.0_45/jre/lib/amd64/server/libjvm.so\
		/home/imdb/supports/boost_1_53_0/stage/lib/libboost_serialization.a \
		/home/imdb/supports/boost_1_53_0/stage/lib/libboost_serialization.so

noinst_LIBRARIES=libparallelblockstreamiterator.a
libparallelblockstreamiterator_a_SOURCES = \
	BlockStreamAggregationIterator.cpp 								BlockStreamAggregationIterator.h\
	BlockStreamCombinedIterator.cpp 								BlockStreamCombinedIterator.h\
	BlockStreamExchangeBroadcast.cpp 								BlockStreamExchangeBroadcast.h\
	BlockStreamExchangeLowerBroadcast.cpp 							BlockStreamExchangeLowerBroadcast.h\
	BlockStreamExpander.cpp 										BlockStreamExpander.h\
	BlockStreamInIterator.cpp 										BlockStreamInIterator.h\
	BlockStreamJoinIterator.cpp 									BlockStreamJoinIterator.h\
	BlockStreamProjectIterator.cpp 									BlockStreamProjectIterator.h\
	BlockStreamSortIterator.cpp 									BlockStreamSortIterator.h\
	BlockStreamTopN.cpp 											BlockStreamTopN.h\
	ExpandableBlockStreamBroadcastExchange.cpp 						ExpandableBlockStreamBroadcastExchange.h\
	ExpandableBlockStreamBroadcastExchangeLower.cpp 				ExpandableBlockStreamBroadcastExchangeLower.h\
	ExpandableBlockStreamBuffer.cpp 								ExpandableBlockStreamBuffer.h\
	ExpandableBlockStreamExchange.cpp 								ExpandableBlockStreamExchange.h\
	ExpandableBlockStreamExchangeEpoll.cpp 							ExpandableBlockStreamExchangeEpoll.h\
	ExpandableBlockStreamExchangeLower.cpp 							ExpandableBlockStreamExchangeLower.h\
	ExpandableBlockStreamExchangeLowerEfficient.cpp 				ExpandableBlockStreamExchangeLowerEfficient.h\
	ExpandableBlockStreamExchangeLowerMaterialized.cpp 				ExpandableBlockStreamExchangeLowerMaterialized.h\
	ExpandableBlockStreamExchangeMaterialized.cpp 					ExpandableBlockStreamExchangeMaterialized.h\
	ExpandableBlockStreamFilter.cpp 								ExpandableBlockStreamFilter.h\
	ExpandableBlockStreamHdfsScan.cpp 								ExpandableBlockStreamHdfsScan.h\
	ExpandableBlockStreamProjectionScan.cpp 						ExpandableBlockStreamProjectionScan.h\
	ExpandableBlockStreamRandomDiskAccess.cpp 						ExpandableBlockStreamRandomDiskAccess.h\
	ExpandableBlockStreamRandomMemAccess.cpp 						ExpandableBlockStreamRandomMemAccess.h\
	ExpandableBlockStreamSingleColumnScan.cpp 						ExpandableBlockStreamSingleColumnScan.h\
	ExpandableBlockStreamSingleColumnScanDisk.cpp 					ExpandableBlockStreamSingleColumnScanDisk.h 
