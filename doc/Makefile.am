#现在写的动态链接库是两个地方都需要写include的文件，这里要想办法将系统中的hadoop_home指定
AM_CPPFLAGS= -fPIC\
-I/home/NewHadoop/hadoop/hadoop-1.0.3/src/c++/libhdfs\
-I/home/NewHadoop/java/jdk1.7.0_45/include\
-I/home/NewHadoop/java/jdk1.7.0_45/include/linux \
-I/home/imdb/tools/Theron-5.01.01/Include \
-I/home/imdb/supports/boost_1_53_0 \
-I/home/imdb/supports/boost_1_53_0/boost/serialization

AM_LDFLAGS= -lc -lm -lrt -lpthread -lboost_serialization -lboost_date_time -lboost_system -lconfig++ -lxs


LDADD =  \
	Test/TestSuit/libtestsuite.a \
	LogicalQueryPlan/liblogicalqueryplan.a \
	Serialization/libserialization.a \
	Catalog/stat/libstat.a \
	storage/libstorage.a \
	Executor/libexecutor.a \
	Catalog/libcatalog.a \
	Client/libclient.a \
	Daemon/libdaemon.a \
	IndexManager/libindexmanager.a \
	Loader/libloader.a \
	Parsetree/libparsetree.a \
	PerformanceMonitor/libperformancemonitor.a \
	Resource/libresouce.a \
	Scheduler/libscheduler.a \
	try/libtry.a \
	BlockStreamIterator/ParallelBlockStreamIterator/libparallelblockstreamiterator.a \
	BlockStreamIterator/libblockstreamiterator.a \
	iterator/libiterator.a \
	BufferManager/libbuffermanager.a \
	BlockIterator/libblockiterator.a \
	utility/libutility.a \
	common/libcommon.a \
	Schema/libschema.a \
	types/libtypes.a \
	Block/libblock.a \
	/home/imdb/supports/boost_1_53_0/stage/lib/libboost_serialization.a \
	/home/imdb/supports/boost_1_53_0/stage/lib/libboost_serialization.so \
	/home/imdb/supports/boost_1_53_0/stage/lib/libboost_system.a \
	/home/imdb/supports/boost_1_53_0/stage/lib/libboost_system.so \
	/home/NewHadoop/hadoop/hadoop-1.0.3/c++/Linux-amd64-64/lib/libhdfs.so \
	/home/NewHadoop/hadoop/hadoop-1.0.3/c++/Linux-amd64-64/lib/libhdfs.a \
	/home/NewHadoop/java/jdk1.7.0_45/jre/lib/amd64/server/libjvm.so\
	/home/imdb/tools/Theron-5.01.01/Lib/libtherond.a

include_HEADERS = A.h \
			AdaptiveEndPoint.h \
			atomics.h \
			Column_to_be_removed.h \
			ColumnAppenderBase.h \
			Comparator.h \
			Config.h \
			configure.h \
			data_type.h \
			Debug.h \
			Environment.h \
			foreach.hpp \
			hash.h \
			hashtable.h \
			HeapColumn.h \
			ids.h \
			IDsGenerator.h \
			iterator.hpp \
			lock.h \
			Logging.h \
			Message.h \
			PortManager.h \
			rdtsc.h \
			rename.h \
			TableManager.h \
			TableManagerMaster.h \
			TableManagerSlave.h \
			TablePartition.h \
			TimeOutReceiver.h \
			Timer.h
				
bin_PROGRAMS =  server

server_SOURCES = Server.cpp \
			AdaptiveEndPoint.cpp \
			Column_to_be_removed.cpp \
			Comparator.cpp \
			Config.cpp \
			Environment.cpp \
			IDsGenerator.cpp \
			Logging.cpp \
			Message.cpp \
			PortManager.cpp \
			TableManager.cpp \
			TableManagerMaster.cpp \
			TableManagerSlave.cpp \
			TablePartition.cpp \
			TimeOutReceiver.cpp \
			boost.cpp \
			example.cpp \
			hash.cpp \
			hashtable.cpp \
			ids.cpp \
			test.cpp 

#生成可执行文件是用LDADD，但是生成.a文件是用LIBADD，
#后面.o是需要加入的，有没有优雅点的方法呢？但如果只是.h文件呢？
#如果有多层依赖的话，要在第一层的makefile中写上第三层.a的依赖，但是这个地方是可以优化的感觉。
#server_LDADD = Block/libblock.a \
#	BlockStreamIterator/libblockstreamiterator.a \
#	BlockStreamIterator/ParallelBlockStreamIterator/libparallelblockstreamiterator.a \
#	BufferManager/libbuffermanager.a \
#	Catalog/libcatalog.a \
#	Catalog/stat/libstat.a \
#	Client/libclient.a \
#	common/libcommon.a \
#	Daemon/libdaemon.a \
#	Executor/libexecutor.a \
#	IndexManager/libindexmanager.a \
#	iterator/libiterator.a \
#	Loader/libloader.a \
#	LogicalQueryPlan/liblogicalqueryplan.a \
#	Parsetree/libparsetree.a \
#	PerformanceMonitor/libperformancemonitor.a \
#	Resource/libresouce.a \
#	Scheduler/libscheduler.a \
#	Schema/libschema.a \
#	Serialization/libserialization.a \
#	storage/libstorage.a \
#	Test/TestSuit/libtestsuite.a \
#	try/libtry.a \
#	types/libtypes.a \
#	utility/libutility.a \
#	/home/imdb/supports/boost_1_53_0/stage/lib/libboost_serialization.a \
#	/home/imdb/supports/boost_1_53_0/stage/lib/libboost_serialization.so \
#	/home/imdb/supports/boost_1_53_0/stage/lib/libboost_system.a \
#	/home/imdb/supports/boost_1_53_0/stage/lib/libboost_system.so \
#	/home/NewHadoop/hadoop/hadoop-1.0.3/c++/Linux-amd64-64/lib/libhdfs.so \
#	/home/NewHadoop/hadoop/hadoop-1.0.3/c++/Linux-amd64-64/lib/libhdfs.a \
#	/home/NewHadoop/java/jdk1.7.0_45/jre/lib/amd64/server/libjvm.so\
#	/home/imdb/tools/Theron-5.01.01/Lib/libtherond.a
	

SUBDIRS=Block BlockIterator BlockStreamIterator BufferManager Catalog Client common Daemon Executor IndexManager\
iterator Loader LogicalQueryPlan Parsetree PerformanceMonitor Resource Scheduler Schema Serialization\
storage Test ThirdParty try types utility

DIST_SUBDIRS = Block BlockIterator BlockStreamIterator BufferManager Catalog Client common Daemon Executor IndexManager\
iterator Loader LogicalQueryPlan Parsetree PerformanceMonitor Resource Scheduler Schema Serialization\
storage Test ThirdParty try types utility
