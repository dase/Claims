/*
 * TestMain.cpp
 *
 *  Created on: Nov 19, 2013
 *      Author: wangli
 */

#include "../LogicalQueryPlan/Test/testGenerateIteratorTree.cpp"
#include "../BlockStreamIterator/ParallelBlockStreamIterator/Test/projectionScan.cpp"
#include "../Executor/Test/Executor.cpp"
//#include "../Block/test/testBlockStreamVar.cpp"
//#include "hashtable_test.cpp"
int main1(){
//	return testVar();
	return testGenerateIteratorTree();
//	return testProjectionScan();
//	return testIteratorExecutor();
//	return hashtable_test();
}
