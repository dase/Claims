AM_CPPFLAGS= -fPIC \
-I/home/imdb/supports/boost_1_53_0 \
-I/home/imdb/supports/boost_1_53_0/boost/serialization

AM_LDFLAGS=-lc -lm -lrt -lboost_serializaiton

LDADD = ../Catalog/libcatalog.a \
		/home/imdb/supports/boost_1_53_0/stage/lib/libboost_serializaiton.a \
		/home/imdb/supports/boost_1_53_0/stage/lib/libboost_system.a

noinst_LIBRARIES=libschema.a
libschema_a_SOURCES = \
	LogicalProjection.cpp 			LogicalProjection.h\
	Schema.cpp 						Schema.h \
	SchemaFix.cpp 					SchemaFix.h \
	SchemaVar.cpp 					SchemaVar.h \
	TupleConvertor.cpp 				TupleConvertor.h

#libschema_a_LIBADD = ../Catalog/libcatalog.a 