AM_CPPFLAGS= -fPIC\
-I/home/NewHadoop/hadoop/hadoop-1.0.3/src/c++/libhdfs\
-I/home/NewHadoop/java/jdk1.7.0_45/include\
-I/home/NewHadoop/java/jdk1.7.0_45/include/linux \
-I/home/imdb/tools/Theron-5.01.01/Include \
-I/home/imdb/supports/boost_1_53_0 \
-I/home/imdb/supports/boost_1_53_0/boost/serialization

AM_LDFLAGS=-lc -lm -lrt -lboost_system


LDADD = \
		../../Serialization/libserialization.a \
	    ../../BlockIterator/libblockiterator.a \
		../../Test/TestSuit/libtestsuite.a \
		../../LogicalQueryPlan/liblogicalqueryplan.a \
		../../Catalog/stat/libstat.a \
		../../storage/libstorage.a \
		../../Executor/libexecutor.a \
		../../Catalog/libcatalog.a \
	    ../../BufferManager/libbuffermanager.a \
		../../Client/libclient.a \
		../../Daemon/libdaemon.a \
		../../IndexManager/libindexmanager.a \
		../../Loader/libloader.a \
		../../Parsetree/libparsetree.a \
		../../PerformanceMonitor/libperformancemonitor.a \
		../../Resource/libresouce.a \
		../../Scheduler/libscheduler.a \
		../../try/libtry.a \
		../../BlockStreamIterator/ParallelBlockStreamIterator/libparallelblockstreamiterator.a \
		../../BlockStreamIterator/libblockstreamiterator.a \
		../../iterator/libiterator.a \
		../../common/libcommon.a \
		../../Schema/libschema.a \
	    ../../types/libtypes.a \
		../../Block/libblock.a \
		../../utility/libutility.a \
		/home/imdb/supports/boost_1_53_0/stage/lib/libboost_serialization.a \
		/home/imdb/supports/boost_1_53_0/stage/lib/libboost_serialization.so \
		/home/imdb/supports/boost_1_53_0/stage/lib/libboost_system.a \
		/home/imdb/supports/boost_1_53_0/stage/lib/libboost_system.so \
		/home/NewHadoop/hadoop/hadoop-1.0.3/c++/Linux-amd64-64/lib/libhdfs.so \
		/home/NewHadoop/hadoop/hadoop-1.0.3/c++/Linux-amd64-64/lib/libhdfs.a \
		/home/NewHadoop/java/jdk1.7.0_45/jre/lib/amd64/server/libjvm.so\
		/home/imdb/tools/Theron-5.01.01/Lib/libtherond.a

noinst_LIBRARIES=libtestsuite.a

libtestsuite_a_SOURCES = \
	ExpanderFrameTest.h \
	hash_table_test.h \
	in_segment_scalability_test.cpp \
	tpc_h_test.cpp
