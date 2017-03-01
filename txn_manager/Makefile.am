AM_CPPFLAGS= -fPIC -fpermissive\
-I${BOOST_HOME} \
-I${BOOST_HOME}/boost/serialization \
-I${HADOOP_HOME}/include\
-I${JAVA_HOME}/include\
-I${JAVA_HOME}/include/linux 

AM_LDFLAGS=-lc -lm -lrt  -ll -ly -lstdc++ -lxs -lcaf_core -lcaf_io -lboost_serialization

if OPT_TCMALLOC
AM_CPPFLAGS+=-fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free 
AM_LDFLAGS+=-ltcmalloc 
endif

LDADD = ../catalog/libcatalog.a \
		../catalog/stat/libstat.a \
		../loader/libloader.a \
		../storage/libstorage.a \
		../common/libcommon.a \
		../common/expression/libexpression.a \
		../common/Expression/libexpression.a \
		../common/Schema/libschema.a \
		../common/Block/libblock.a	\
		../common/type/libtypes.a \
		../common/types/ttmath/libttmath.a \
		../common/serialization/libserialization.a \
		../utility/libutility.a \
		../Executor/libexecutor.a \
		../sql_parser/ast_node/libast_node.a \
		../sql_parser/parser/libparser.a \
		../BufferManager/libbuffermanager.a \
		../node_manager/libnodemanager.a \
		${BOOST_HOME}/stage/lib/libboost_system.a \
		${BOOST_HOME}/stage/lib/libboost_system.so \
		${BOOST_HOME}/stage/lib/libboost_serialization.a \
		${BOOST_HOME}/stage/lib/libboost_serialization.so

noinst_LIBRARIES=libtxnmanager.a

libtxnmanager_a_SOURCES = \
	txn.hpp txn.cpp \
	txn_client.hpp txn_client.cpp \
	txn_server.hpp txn_server.cpp  \
	txn_log.hpp txn_log.cpp  

	

	
