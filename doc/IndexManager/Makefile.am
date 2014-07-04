AM_CPPFLAGS= -fPIC \
-I/home/imdb/supports/boost_1_53_0 \
-I/home/imdb/supports/boost_1_53_0/boost/serialization \
-I/home/imdb/supports/Theron-5.01.01/Include \
-I/home/NewHadoop/hadoop/hadoop-1.0.3/src/c++/libhdfs\
-I/home/NewHadoop/java/jdk1.7.0_45/include\
-I/home/NewHadoop/java/jdk1.7.0_45/include/linux

AM_LDFLAGS=-lc -lm -lrt -lboost_serialization

LDADD = ../storage/libstorage.a \
		../Schema/libschema.a \
		../BlockStreamIterator/libblockstreamiterator.a \
		../Block/libblock.a \
		../Catalog/libcatalog.a \
		../iterator/libiterator.a \
		../Scheduler/libscheduler.a \
		../LogicalQueryPlan/liblogicalqueryplan.a

noinst_LIBRARIES=libindexmanager.a
libindexmanager_a_SOURCES = \
	CSBIndexBuilding.cpp			CSBIndexBuilding.h \
	CSBPlusTree.cpp 				CSBPlusTree.h \
	IndexManager.cpp 				IndexManager.h \
	IndexScanIterator.cpp		 	IndexScanIterator.h \
	LogicalCSBIndexBuilding.cpp 	LogicalCSBIndexBuilding.h \
	LogicalIndexScan.cpp			LogicalIndexScan.h
