AM_CPPFLAGS= -fPIC\
-I/home/imdb/supports/boost_1_53_0 \
-I/home/imdb/supports/boost_1_53_0/boost/serialization

#<boost/unordered_map.hpp>应该加什么编译选项
AM_LDFLAGS=-lc -lm -lrt lboost_serialization

LDADD = ../../Block/libblock.a \
		../../LogicalQueryPlan/liblogicalqueryplan.a \
		../Catalog/libcatalog.a \
		/home/imdb/supports/boost_1_53_0/stage/lib/libboost_serializaiton.a \
		/home/imdb/supports/boost_1_53_0/stage/lib/libboost_system.a

noinst_LIBRARIES=libstat.a

libstat_a_SOURCES = \
	Analyzer.cpp 				Analyzer.h\
	AttributeStatistics.cpp 	AttributeStatistics.h\
	Estimation.cpp 				Estimation.h\
	StatManager.cpp 			StatManager.h\
	Statistic.cpp 				Statistic.h\
	TableStatistic.cpp 			TableStatistic.h 

#libstat_a_LIBADD = ../../Block/libblock.a \
#				../../LogicalQueryPlan/liblogicalqueryplan.a \
#				../Catalog/libcatalog.a