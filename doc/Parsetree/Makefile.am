AM_CPPFLAGS= -fPIC \
-I/home/NewHadoop/hadoop/hadoop-1.0.3/src/c++/libhdfs\
-I/home/NewHadoop/java/jdk1.7.0_45/include\
-I/home/NewHadoop/java/jdk1.7.0_45/include/linux \
-I/home/imdb/supports/Theron-5.01.01/Include \
-I/home/imdb/supports/boost_1_53_0 \
-I/home/imdb/supports/boost_1_53_0/boost/serialization

AM_LDFLAGS=-lc -lm -lrt -lboost_system -lboost_serialization -ll -ly -lstdc++

LDADD = ../Catalog/libcatalog.a \
		../LogicalQueryPlan/liblogicalqueryplan.a \
		../utility/libutility.a \
		../Loader/libloader.a \
		../iterator/libiterator.a \
		../BlockStreamIterator/libblockstreamiterator.a	

noinst_LIBRARIES=libparsetree.a

libparsetree_a_SOURCES = \
	ExecuteLogicalQueryPlan.cpp  		ExecuteLogicalQueryPlan.h \
	function.cpp \
	lex.yy.cpppp \
	parsetree2logicalplan.cpp \
	runparsetree.cpp \
	semantic_analysis.cpp \
	sql_node_struct.h \
	sql.tab.cpp                 		sql.tab.cpp\
	wc2tb.cpp
