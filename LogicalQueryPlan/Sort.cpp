/*
 * Sort.cpp
 *
 *  Created on: 2014-3-1
 *      Author: casa
 */

#include "Sort.h"
#include "../BlockStreamIterator/ParallelBlockStreamIterator/ExpandableBlockStreamExchangeMaterialized.h"
#include "../BlockStreamIterator/ParallelBlockStreamIterator/BlockStreamExpander.h"
#include "../Config.h"
LogicalSort::LogicalSort(LogicalOperator *child,std::vector<LogicalSort::OrderByAttr*>oba)
:child_(child),oba_(oba){

}

LogicalSort::~LogicalSort() {

}

Dataflow LogicalSort::getDataflow(){
	dataflow_=child_->getDataflow();
	Dataflow ret;
	ret.attribute_list_=dataflow_.attribute_list_;
	ret.property_.commnication_cost=dataflow_.property_.commnication_cost;

	ret.property_.partitioner=dataflow_.property_.partitioner;
	ret.property_.partitioner.setPartitionFunction(dataflow_.property_.partitioner.getPartitionFunction());
	ret.property_.partitioner.setPartitionKey(Attribute());

	NodeID location=0;
	unsigned long data_cardinality=0;
	PartitionOffset offset=0;
	DataflowPartition par(offset,data_cardinality,location);
	std::vector<DataflowPartition> partition_list;
	partition_list.push_back(par);
	ret.property_.partitioner.setPartitionList(partition_list);
	return ret;
}

BlockStreamIteratorBase *LogicalSort::getIteratorTree(const unsigned& blocksize){
	Dataflow rt=getDataflow();
	BlockStreamIteratorBase *child=child_->getIteratorTree(blocksize);
	BlockStreamSortIterator::State mapper_state;
	mapper_state.block_size_=blocksize;
	mapper_state.child_=child;
	for(unsigned i=0;i<oba_.size();i++){
//		state.orderbyKey_.push_back(getOrderByKey(oba_[i]->tbl_,oba_[i]->attr_));
		mapper_state.orderbyKey_.push_back(getOrderByKey(oba_[i]->ta_));
	}
	mapper_state.input_=getSchema(dataflow_.attribute_list_);
	BlockStreamIteratorBase *mapper_sort=new BlockStreamSortIterator(mapper_state);

	BlockStreamExpander::State expander_state;
	expander_state.block_count_in_buffer_=EXPANDER_BUFFER_SIZE;
	expander_state.block_size_=blocksize;
	expander_state.init_thread_count_=Config::initial_degree_of_parallelism;
	expander_state.child_=mapper_sort;
	expander_state.schema_=mapper_state.input_;//getSchema(dataflow_->attribute_list_);
	BlockStreamIteratorBase* expander_lower=new BlockStreamExpander(expander_state);

	ExpandableBlockStreamExchangeMaterialized::State exchange_state;
	exchange_state.block_size_=blocksize;
	exchange_state.child_=expander_lower;
	exchange_state.exchange_id_=1;
	exchange_state.orderbyKey_=mapper_state.orderbyKey_;
	exchange_state.schema_=getSchema(dataflow_.attribute_list_);
	vector<NodeID> lower_ip_list=getInvolvedNodeID(dataflow_.property_.partitioner);
	exchange_state.lower_ip_list_=convertNodeIDListToNodeIPList(lower_ip_list);//upper
	cout<<exchange_state.lower_ip_list_.size()<<":tail!<<<>>>"<<endl;
	/* todo: compute the upper_ip_list to do reduce side sort */
	vector<NodeID> upper_ip_list;
	upper_ip_list.push_back(0);
	exchange_state.upper_ip_list_=convertNodeIDListToNodeIPList(upper_ip_list);//lower
	cout<<exchange_state.upper_ip_list_[0]<<":tail!<<<>>>"<<upper_ip_list.size()<<endl;
	BlockStreamIteratorBase *exchange=new ExpandableBlockStreamExchangeMaterialized(exchange_state);

	BlockStreamSortIterator::State reducer_state;
	reducer_state.block_size_=blocksize;
	reducer_state.child_=exchange;
	for(unsigned i=0;i<oba_.size();i++){
		reducer_state.orderbyKey_.push_back(getOrderByKey(oba_[i]->ta_));
	}
	reducer_state.input_=getSchema(dataflow_.attribute_list_);
	BlockStreamIteratorBase *reducer_sort=new BlockStreamSortIterator(reducer_state);

	return exchange;
}

int LogicalSort::getOrderByKey(const char *tbe, const char *attr){
	for(unsigned i=0;i<dataflow_.attribute_list_.size();i++){
		TableDescriptor *table=Catalog::getInstance()->getTable(dataflow_.attribute_list_[i].table_id_);
		string tablename=table->getTableName();
		if((tablename.compare(tbe)==0)&&(dataflow_.attribute_list_[i].attrName.compare(attr)==0)){
			return i;
		}
	}
}

int LogicalSort::getOrderByKey(const char *ta){
	for(unsigned i=0;i<dataflow_.attribute_list_.size();i++){
		string _ta(ta);
		if(_ta.compare(dataflow_.attribute_list_[i].attrName)==0){
			return i;
		}
	}
}

void LogicalSort::printOrderByAttr()const
{
	cout<<"OrderByAttr:"<<endl;
	for(int i=0;i<oba_.size();i++ )
	{
		printf("                        %s\n",(const char *)oba_[i]->ta_);

	}
}

void LogicalSort::print(int level)const
{
	printOrderByAttr();
	child_->print(level+1);
}
