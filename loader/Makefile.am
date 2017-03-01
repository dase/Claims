
AM_CPPFLAGS= -fPIC -fpermissive \
-I${BOOST_HOME} \
-I${BOOST_HOME}/boost/serialization \
-I${HADOOP_HOME}/include\
-I${JAVA_HOME}/include\
-I${JAVA_HOME}/include/linux \
-I${THERON_HOME}/Include \
-I${CAF_HOME}/libcaf_io	\
-I${CAF_HOME}/libcaf_core \
-I/usr/local/include/activemq-cpp-3.9.3 \
-I/usr/local/apr/include/apr-1

AM_LDFLAGS=-lc -lm -lrt -lcaf_core -lcaf_io -lxs -lboost_serialization

if OPT_TCMALLOC
AM_CPPFLAGS+=-fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free
AM_LDFLAGS+=-ltcmalloc 
endif

#LDADD = 	../catalog/libcatalog.a \
#		../common/libcommon.a \
#		../common/Block/libblock.a \
#		../common/file_handle/libfilehandle.a \
#		../common/Schema/libschema.a 

noinst_LIBRARIES=libloader.a
libloader_a_SOURCES = \
	data_ingestion.cpp 		data_ingestion.h \
	file_connector.h \
	master_loader.cpp 		master_loader.h \
	single_file_connector.cpp 		single_file_connector.h \
	single_thread_single_file_connector.cpp 		single_thread_single_file_connector.h \
	slave_loader.cpp 		slave_loader.h \
	table_file_connector.cpp 		table_file_connector.h \
	validity.cpp 		validity.h \
	load_packet.h		load_packet.cpp \
	AMQ_consumer.h		AMQ_consumer.cpp

SUBDIRS = test
DIST_SUBDIRS = test
