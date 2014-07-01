AM_CPPFLAGS= -fPIC\
-I/home/NewHadoop/hadoop/hadoop-1.0.3/src/c++/libhdfs\
-I/home/NewHadoop/java/jdk1.7.0_45/include\
-I/home/NewHadoop/java/jdk1.7.0_45/include/linux \
-I/home/imdb/tools/Theron-5.01.01/Include \
-I/home/imdb/supports/boost_1_53_0 \
-I/home/imdb/supports/boost_1_53_0/boost/serialization

AM_LDFLAGS=-lc -lm -lrt 

LDADD = ../Schema/libschema.a \
		../utility/libutility.a

noinst_LIBRARIES=libblock.a
libblock_a_SOURCES = \
	Block.cpp 							Block.h \
	BlockContainer.cpp 					BlockContainer.h \
	BlockFix.cpp 						BlockFix.h \
	BlockReadable.cpp 					BlockReadable.h \
	BlockReadableFix.cpp 				BlockReadableFix.h \
	BlockReadableFixBuffer.cpp 			BlockReadableFixBuffer.h \
	BlockStream.cpp 					BlockStream.h \
	MonitorableBuffer.cpp 				MonitorableBuffer.h \
	BlockStreamBuffer.cpp 				BlockStreamBuffer.h \
	BlockWritable.cpp 					BlockWritable.h \
	BlockWritableFix.cpp 				BlockWritableFix.h \
	BlockWritableFixBuffer.cpp 			BlockWritableFixBuffer.h \
	DynamicBlockBuffer.cpp 				DynamicBlockBuffer.h\
	PartitionedBlockBuffer.cpp 			PartitionedBlockBuffer.h \
	PartitionedBlockContainer.cpp 		PartitionedBlockContainer.h \
	ResultSet.cpp 						ResultSet.h \
	synch.h

#libblock_a_LIBADD = ../Schema/libschema.a ../utility/libutility.a
