AM_CPPFLAGS= -fPIC \
-I/home/imdb/supports/boost_1_53_0

AM_LDFLAGS=-lc -lm -lrt -lboost_serialization

LDADD = ../Catalog/libcatalog.a \
		../Daemon/libdaemon.a \
		../Block/libblock.a \
		../Schema/libschema.a \
		/home/imdb/supports/boost_1_53_0/stage/lib/libboost_serialization.a \
		/home/imdb/supports/boost_1_53_0/stage/lib/libboost_serialization.so			

noinst_LIBRARIES=libclient.a
libclient_a_SOURCES = \
	ClaimsServer.cpp		ClaimsServer.h \
	Client.cpp 				Client.h \
	ClientResponse.cpp 		ClientResponse.h
