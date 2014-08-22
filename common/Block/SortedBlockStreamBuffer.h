/*
 * SortedBlockStreamBuffer.h
 *
 *  Created on: 2014-7-28
 *      Author: casa
 */

#ifndef SORTEDBLOCKSTREAMBUFFER_H_
#define SORTEDBLOCKSTREAMBUFFER_H_
#include "../../common/Block/BlockStreamBuffer.h"
#include "../../common/Block/BlockStream.h"
#include "../../common/Schema/Schema.h"

#include <vector>
using namespace std;

struct TupleNode{
	void *tuple;
	TupleNode *next;
};

class SortedBlockStreamBuffer {
public:
	SortedBlockStreamBuffer(unsigned, Schema *, unsigned, vector<unsigned>);
	virtual ~SortedBlockStreamBuffer();

	void init();

	TupleNode *mergeTwoList(TupleNode *, TupleNode *);
	TupleNode *catAll(TupleNode *, TupleNode *);//for debuging

	void sortAllTuples();
	void insertIntoBuffer(BlockStreamBuffer *&);
	void insertIntoSortedLists(BlockStreamBase *, int);

	bool CompareTwoTuple(TupleNode *, TupleNode *);

private:
	vector<TupleNode *> root_;
	vector<TupleNode *> cursor_;
	TupleNode *sorted_root_;
	Schema *schema_;
	unsigned nPartitions_;
	unsigned blockSize_;
	vector<unsigned> orderbyKey_;
};

#endif /* SORTEDBLOCKSTREAMBUFFER_H_ */
