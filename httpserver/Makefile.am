AM_CPPFLAGS= -fPIC -fpermissive \
-I${BOOST_HOME} \
-I${BOOST_HOME}/boost/asio \
-I${BOOST_HOME}/boost/serialization \
-I${THERON_HOME}/Include 


AM_LDFLAGS=-lc -lm -lrt -lboost_serialization -lxs -lboost_system -lboost_thread-mt

if OPT_TCMALLOC
AM_CPPFLAGS+=-fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free
AM_LDFLAGS+=-ltcmalloc 
endif

LDADD = 	../catalog/libcatalog.a \
		../Daemon/libdaemon.a \
		../common/Block/libblock.a \
		../common/Schema/libschema.a \
		../httpserver/json/libjson.a \
		${BOOST_HOME}/stage/lib/libboost_serialization.a \
		${BOOST_HOME}/stage/lib/libboost_serialization.so \
		${BOOST_HOME}/stage/lib/libboost_system.a \
		${BOOST_HOME}/stage/lib/libboost_system.so \
		${THERON_HOME}/Lib/libtherond.a		

noinst_LIBRARIES = libhttpserver.a

libhttpserver_a_SOURCES = \
	server.cpp			server.hpp\
	connection.cpp		connection.hpp\
	connection_manager.cpp	connection_manager.hpp\
	io_service_pool.cpp	io_service_pool.hpp\
	mime_types.cpp		mime_types.hpp\
	reply.cpp			reply.hpp\
	request_handler.cpp	request_handler.hpp\
	request_parser.cpp	request_parser.hpp\
	claimshttpserver.cpp	header.hpp		\
	request.hpp			claimshttpserver.hpp\
	json.cppq			ResultToJson.hpp

SUBDIRS = json
DIST_SUBDIRS = json

