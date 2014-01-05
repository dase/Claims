#include "../SchemaVar.h"
#include "../../ids.h"
#include "../../Block/BlockStream.h"
#include "../../BlockStreamIterator/ParallelBlockStreamIterator/ExpandableBlockStreamProjectionScan.h"
#include "../../BlockStreamIterator/ParallelBlockStreamIterator/ExpandableBlockStreamFilter.h"
#include <iostream>
using namespace std;
//
//int main(){
//	cout<<"in the main!!!"<<endl;
//	std::vector<column_type> vc;
//	vc.push_back(column_type(t_int));
//	vc.push_back(column_type(t_double));
//	vc.push_back(column_type(t_string));
//	Schema *sch=new SchemaVar(vc);
//
//	cout<<"in the main!!!"<<endl;
//	ProjectionID *pid=new ProjectionID(1,1);
//	ExpandableBlockStreamProjectionScan::State ebsps_state(*pid,sch,1024*64);
//	BlockStreamIteratorBase *ebsps=new ExpandableBlockStreamProjectionScan(ebsps_state);
//
//	int f=100;
//	FilterIterator::AttributeComparator filter1(column_type(t_int),Comparator::L,1,&f);
//	std::vector<FilterIterator::AttributeComparator> ComparatorList;
//	ComparatorList.push_back(filter1);
//
//	cout<<"in the main!!!"<<endl;
//	ExpandableBlockStreamFilter::State ebsf_state(sch,ebsps,ComparatorList,1024*64);
//	BlockStreamIteratorBase *ebsf=new ExpandableBlockStreamFilter(ebsf_state);
//
//	cout<<"in the main!!!"<<endl;
//	BlockStreamBase *bsb=new BlockStreamVar(64*1024,sch);
//	int choice=0;
//	cout<<"input: "<<endl;
//	scanf("%d",&choice);
//	while(choice==1){
//		unsigned tuple_count=0;
//		ebsf->open();
////		ebsf->next(bsb);
//		while(ebsf->next(bsb)){
//			BlockStreamBase::BlockStreamTraverseIterator *it=bsb->createIterator();
//			void* tuple;
//			while(tuple=it->nextTuple()){
//				tuple_count++;
//			}
//			bsb->setEmpty();
//		}
//		ebsf->close();
//		cout<<"input: "<<endl;
//		scanf("%d",&choice);
//	}
//
//	return 0;
//}
#include <vector>
#include <iostream>
#include "../../Environment.h"
#include "../../LogicalQueryPlan/Scan.h"
#include "../../LogicalQueryPlan/LogicalQueryPlanRoot.h"
#include "../../LogicalQueryPlan/EqualJoin.h"
#include "../../Catalog/ProjectionBinding.h"
#include "../../LogicalQueryPlan/Filter.h"
#include "../../LogicalQueryPlan/Aggregation.h"
using namespace std;
int main(){
	int master;
//	cout<<"Master(0) or Slave(others)"<<endl;
//	cin>>master;
	printf("Master(0) or Slave(others)??\n");
	scanf("%d",&master);
	if(master!=0){
		Environment::getInstance(false);
	}
	else{

		Environment::getInstance(true);

		ResourceManagerMaster *rmms=Environment::getInstance()->getResourceManagerMaster();
		Catalog* catalog=Environment::getInstance()->getCatalog();

		TableDescriptor* table_1=new TableDescriptor("1",Environment::getInstance()->getCatalog()->allocate_unique_table_id());
		table_1->addAttribute("1",data_type(t_int));  				//0
		table_1->addAttribute("2",data_type(t_int));
		table_1->addAttribute("3",data_type(t_string));

		vector<ColumnOffset> cj_proj0_index;
		cj_proj0_index.push_back(0);
		cj_proj0_index.push_back(1);
		cj_proj0_index.push_back(2);
//		const int partition_key_index_1=2;
		table_1->createHashPartitionedProjection(cj_proj0_index,"1",1);	//G0

		catalog->add_table(table_1);

		for(unsigned i=0;i<table_1->getProjectoin(0)->getPartitioner()->getNumberOfPartitions();i++){

			catalog->getTable(0)->getProjectoin(0)->getPartitioner()->RegisterPartition(i,1);
		}

//		printf("ready(?)\n");
//		int input;
//		scanf("%d",&input);

		LogicalOperator* scan1=new LogicalScan(table_1->getProjectoin(0));

		Filter::Condition filter_con;
		char *tmp2="string1";
		filter_con.add(table_1->getAttribute(2),FilterIterator::AttributeComparator::EQ,tmp2);
		LogicalOperator* filter1=new Filter(filter_con,scan1);

		const NodeID collector_node_id=0;
		LogicalOperator* root=new LogicalQueryPlanRoot(collector_node_id,filter1,LogicalQueryPlanRoot::PERFORMANCE);
		root->getDataflow();
		BlockStreamIteratorBase* executable_query_plan=root->getIteratorTree(1024*64-sizeof(unsigned));
//		BlockStreamIteratorBase* executable_query_plan=root->getIteratorTree(1024-sizeof(unsigned));
		cout<<"after get iterator tree!!"<<endl;
		int c=1;
		while(c==1){
			IteratorExecutorMaster::getInstance()->ExecuteBlockStreamIteratorsOnSite(executable_query_plan,"10.11.1.199");

//						executable_query_plan->open();
//			while(executable_query_plan->next(0));
//			executable_query_plan->close();
//
//			cout<<"Terminal(0) or continue(others)?"<<endl<<flush;
//			cin>>c;
			printf("Terminate(0) or continue(others)?\n");
//			sleep()
			scanf("%d",&c);
//			sleep(10);
//			getchar();
//			cout<<"<<<<<<<<<<<<<<<<<<<<<You input ="<<c+1<<endl<<flush;
			printf("you print=%d\n",c);
//			sleep(5);
		}
	}
	cout<<"Waiting~~~!~"<<endl;
	while(true){
		sleep(1);
	}

}
