AM_CPPFLAGS= -fPIC \
-I/home/imdb/supports/boost_1_53_0 \
-I/home/imdb/supports/boost_1_53_0/boost/serialization

LDADD = /home/imdb/supports/boost_1_53_0/stage/lib/libboost_serializaiton.a \
		/home/imdb/supports/boost_1_53_0/stage/lib/libboost_system.a

AM_LDFLAGS=-lc -lm -lrt -lboost_serialization

EXTRA_DIST = unordered_collections_load_imp.hpp\
			unordered_collections_save_imp.hpp