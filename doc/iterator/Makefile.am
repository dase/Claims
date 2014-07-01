AM_CPPFLAGS= -fPIC \
-I/home/imdb/supports/boost_1_53_0 \
-I/home/imdb/supports/boost_1_53_0/boost/serialization \
-I/home/NewHadoop/hadoop/hadoop-1.0.3/src/c++/libhdfs\
-I/home/NewHadoop/java/jdk1.7.0_45/include\
-I/home/NewHadoop/java/jdk1.7.0_45/include/linux\
-I/home/imdb/tools/Theron-5.01.01/Include

#此模块添加boost序列化库
AM_LDFLAGS=-lc -lm -lrt -lboost_serializaiton

LDADD = ../Block/libblock.a \
		../Schema/libschema.a \
		../Executor/libexecutor.a \
		../Serialization/libserialization.a

noinst_LIBRARIES=libiterator.a
libiterator_a_SOURCES = \
	AggregationIterator.cpp 						AggregationIterator.h\
	CombinedIterator.cpp 							CombinedIterator.h\
	ExchangeBase.cpp 								ExchangeBase.h\
	ExchangeIterator.cpp							ExchangeIterator.h\
	ExchangeIteratorEager.cpp 						ExchangeIteratorEager.h\
	ExchangeIteratorEagerLower.cpp 					ExchangeIteratorEagerLower.h\
	ExchangeIteratorLower.cpp 						ExchangeIteratorLower.h\
	ExchangeIteratorLowerWithWideDependency.cpp 	ExchangeIteratorLowerWithWideDependency.h\
	ExchangeIteratorWithWideDependency.cpp 			ExchangeIteratorWithWideDependency.h\
	FilterIterator.cpp 								FilterIterator.h\
	JoinIterator.cpp 								JoinIterator.h\
	PrintIterator.cpp 								PrintIterator.h\
	RandomDiskAccessIterator.cpp 					RandomDiskAccessIterator.h\
	RandomMemAccessIterator.cpp 					RandomMemAccessIterator.h\
	RowScanIterator.cpp 							RowScanIterator.h\
	SequencialDiskAccessIterator.cpp 				SequencialDiskAccessIterator.h\
	SingleColumnScanIterator.cpp 					SingleColumnScanIterator.h\
	SingleColumnScanIteratorFix.cpp 				SingleColumnScanIteratorFix.h				