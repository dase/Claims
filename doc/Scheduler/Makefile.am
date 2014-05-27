AM_CPPFLAGS= -fPIC

AM_LDFLAGS=-lc -lm -lrt

LDADD = ../Catalog/libcatalog.a \
		../LogicalQueryPlan/liblogicalqueryplan.a \
		../Schema/libschema.a \
		../BlockStreamIterator/libblockstreamiterator.a

noinst_LIBRARIES=libscheduler.a
libscheduler_a_SOURCES = \
	Dataflow.cpp 			Dataflow.h\
	ExecuteTask.cpp 		ExecuteTask.h
#	OperateNode.cpp 		OperateNode.h\
#	Scheduler.cpp 			Scheduler.h

#libscheduler_a_LIBADD = \
#		../Catalog/libcatalog.a \
#		../LogicalQueryPlan/liblogicalqueryplan.a \
#		../Schema/libschema.a \
#		../BlockStreamIterator/libblockstreamiterator.a