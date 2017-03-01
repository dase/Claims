AM_CPPFLAGS=-fPIC  -DTHERON_XS\
-I${HADOOP_HOME}/include\
-I${JAVA_HOME}/include\
-I${JAVA_HOME}/include/linux \
-I${BOOST_HOME} \
-I${BOOST_HOME}/boost/serialization \
-I${GTEST_HOME}/include \
-I${CAF_HOME}/libcaf_io	\
-I${CAF_HOME}/libcaf_core
#-L/usr/local/lib \
#-I/usr/local/include
AM_LDFLAGS=  -lc -lm -lrt -lpthread -lactivemq-cpp -lprofiler\
	-lconfig++ -lxs -lnuma -lreadline -lhistory -lz -ltinfo -Wl,--no-as-needed -ldl -rdynamic -lglog

if OPT_TCMALLOC
AM_CPPFLAGS+=-fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free
AM_LDFLAGS+=-ltcmalloc 
endif

LDADD =  \
	Test/libtest.a \
	Test/common/libcommon.a \
	Test/utility/libutility.a \
	node_manager/libnodemanager.a \
	common/serialization/libserialization.a \
	Client/libclient.a \
	Daemon/libdaemon.a \
	Executor/libexecutor.a \
	Executor/Test/libtest.a \
	IndexManager/libindexmanager.a \
	IndexManager/Test/libtest.a \
    stmt_handler/libstmthandler.a \
    sql_parser/Test/libparser_test.a	\
    sql_parser/parser/libparser.a \
    sql_parser/ast_node/libast_node.a \
    exec_tracker/libexec_tracker.a	\
	logical_operator/liblogicalqueryplan.a \
	physical_operator/libphysicalqueryplan.a \
	storage/libstorage.a \
	Resource/libresouce.a \
	catalog/libcatalog.a \
	loader/libloader.a \
	loader/test/libtest.a \
    codegen/libcodegen.a\
	common/libcommon.a \
	common/Block/libblock.a \
	common/Expression/libexpression.a \
	common/expression/libexpression.a \
	common/file_handle/libfilehandle.a \
	common/file_handle/test/libtest.a \
	common/log/liblog.a \
	common/Schema/libschema.a \
	common/Schema/Test/libtest.a \
	common/types/libtypes.a \
	common/types/Test/libtest.a \
	common/types/ttmath/libttmath.a \
	utility/libutility.a \
	txn_manager/libtxnmanager.a \
	${CAF_HOME}/build/lib/libcaf_core.so \
	${CAF_HOME}/build/lib/libcaf_io.so \
	${BOOST_HOME}/stage/lib/libboost_serialization.a \
	${BOOST_HOME}/stage/lib/libboost_serialization.so \
	${BOOST_HOME}/stage/lib/libboost_system.so \
	${BOOST_HOME}/stage/lib/libboost_date_time.so \
	${HADOOP_HOME}/lib/native/libhdfs.so\
	${JAVA_HOME}/jre/lib/amd64/server/libjvm.so\
	${GTEST_HOME}/libgtest.a \
	${GLOG_HOME}/.libs/libglog.so.0 \
	-lLLVMJIT -lLLVMX86Disassembler -lLLVMX86AsmParser -lLLVMX86CodeGen -lLLVMSelectionDAG -lLLVMAsmPrinter \
	-lLLVMMCParser -lLLVMCodeGen -lLLVMScalarOpts -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis \
	-lLLVMX86Desc -lLLVMX86Info -lLLVMX86AsmPrinter -lLLVMX86Utils -lLLVMMCJIT -lLLVMTarget -lLLVMRuntimeDyld \
	-lLLVMExecutionEngine -lLLVMMC -lLLVMObject -lLLVMBitReader -lLLVMCore -lLLVMSupport -lLLVMMCParser 

LDADD += catalog/stat/libstat.a 

if OPT_DEBUG
LDADD +=	${THERON_HOME}/Lib/libtherond.a 
else
LDADD +=	${THERON_HOME}/Lib/libtheron.a 
endif
 
include_HEADERS = Config.h \
				Debug.h \
				Environment.h \
				IDsGenerator.h \
				configure.h  \
				txn_manager/txn.hpp \
				txn_manager/txn_server.hpp \
				txn_manager/txn_client.hpp
				
bin_PROGRAMS = claimsserver \
				client \
				test  \
				txntest
				
client_SOURCES = Client.cpp \
					Environment.cpp \
					Config.cpp \
					IDsGenerator.cpp 

claimsserver_SOURCES = Config.cpp	\
				Environment.cpp \
				IDsGenerator.cpp \
				Server.cpp

test_SOURCES = Test/gtest_main.cpp\
				Environment.cpp \
				IDsGenerator.cpp \
				Config.cpp
				
txntest_SOURCES = txn_manager/txn_client_test.cpp
				

SUBDIRS=  catalog Client common Daemon Executor IndexManager\
loader physical_operator logical_operator  Resource \
storage Test utility codegen sql_parser stmt_handler txn_manager node_manager exec_tracker

DIST_SUBDIRS =  catalog Client common Daemon Executor IndexManager\
loader physical_operator logical_operator  Resource \
storage Test utility codegen sql_parser stmt_handler txn_manager node_manager exec_tracker

