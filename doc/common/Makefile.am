AM_CPPFLAGS= -fPIC \
-I/home/imdb/supports/boost_1_53_0

AM_LDFLAGS=-lc -lm -lrt -lboost_serialization

LDADD = ../utility/libutility.a \
		/home/imdb/supports/boost_1_53_0/stage/lib/libboost_serialization.a \
		/home/imdb/supports/boost_1_53_0/stage/lib/libboost_serialization.so

noinst_LIBRARIES=libcommon.a
libcommon_a_SOURCES = \
	Expression_item.cpp 								   \
	ExpressionCalculator.cpp    	ExpressionCalculator.h \
	ExpressionItem.cpp 				ExpressionItem.h       \
	functions.h 										   \
	Mapping.cpp 					Mapping.h              \
	TypeCast.cpp 					TypeCast.h             \
	TypePromotionMap.cpp 			TypePromotionMap.h

#libcommon_a_LIBADD = ../utility/libutility.a \
#/home/imdb/supports/boost_1_53_0/stage/lib/libboost_serialization.a \
#/home/imdb/supports/boost_1_53_0/stage/lib/libboost_serialization.so			