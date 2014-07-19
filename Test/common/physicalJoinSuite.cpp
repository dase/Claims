#include "../../Environment.h"
#include "../../Catalog/table.h"
#include "../../LogicalQueryPlan/LogicalQueryPlanRoot.h"
#include "../../LogicalQueryPlan/Aggregation.h"
#include "../../LogicalQueryPlan/Scan.h"
#include "../../LogicalQueryPlan/Filter.h"
#include "../../LogicalQueryPlan/Project.h"
#include "../../LogicalQueryPlan/EqualJoin.h"
#include "../../common/ExpressionItem.h"
#include "../../common/ExpressionCalculator.h"
#include "../../utility/rdtsc.h"
#include "../../LogicalQueryPlan/Sort.h"
#include "../../BlockStreamIterator/ParallelBlockStreamIterator/SortmergeJoin.h"

int SortmergejoinSuite(){
	/*
	 * select sum(a+1)+count(a),b
	 * from T
	 * group by b
	 *
	 * notation: p a p s
	 * */
	unsigned long long int start=curtick();
	TableDescriptor* table1=Environment::getInstance()->getCatalog()->getTable("timeseries");
	TableDescriptor* table2=Environment::getInstance()->getCatalog()->getTable("frienddistributionF");
	//===========================scan===========================
	LogicalOperator* scan1=new LogicalScan(table1->getProjectoin(0));
	LogicalOperator* scan2=new LogicalScan(table2->getProjectoin(0));

	vector<LogicalSort::OrderByAttr*> vo1;
	LogicalSort::OrderByAttr tmp1=LogicalSort::OrderByAttr("timeseries.AllTweet"); vo1.push_back(&tmp1);

	vector<LogicalSort::OrderByAttr*> vo2;
	LogicalSort::OrderByAttr tmp2=LogicalSort::OrderByAttr("frienddistributionF.EventId"); vo2.push_back(&tmp2);

	LogicalOperator* sort1=new LogicalSort(scan1,vo1);
	LogicalOperator* sort2=new LogicalSort(scan2,vo2);

	////////////////////////////////////////////////////sortmerge join
//	BlockStreamIteratorBase *left=sort1->getIteratorTree(64*1024);
//	BlockStreamIteratorBase *right=sort2->getIteratorTree(64*1024);
//
//	std::vector<column_type> column_list_left;
//	std::vector<column_type> column_list_right;
//	std::vector<column_type> column_list_out;
//	column_list_left.push_back(column_type(t_int));
//	column_list_right.push_back(column_type(t_int));
//	column_list_out.push_back(column_type(t_int));
//	Schema *input_schema_left=new SchemaFix(column_list_left);
//	Schema *input_schema_right=new SchemaFix(column_list_right);
//	Schema *output_schema=new SchemaFix(column_list_out);
//
//	std::vector<unsigned> joinIndex_left;
//	std::vector<unsigned> joinIndex_right;
//	std::vector<unsigned> payload_left;
//	std::vector<unsigned> payload_right;
//
//	joinIndex_left.push_back(1);
//	joinIndex_right.push_back(1);
//	payload_left.push_back(1);
//	payload_left.push_back(1);
//
//	SortmergeJoin::State sortmergejoin_state(left,right,input_schema_left,input_schema_right,output_schema,joinIndex_left,joinIndex_right,payload_left,payload_right);
//	SortmergeJoin *smj=new SortmergeJoin(sortmergejoin_state);
//
//	cout<<"performance is ok!"<<endl;
//	smj->open();
//	while(smj->next(0));
//	smj->close();
//	printf("Q1: execution time: %4.4f second.\n",getSecond(start));

	//===========================root===========================
	cout<<"performance is ok!"<<endl;
	LogicalOperator* root=new LogicalQueryPlanRoot(0,sort1,LogicalQueryPlanRoot::PRINT);

	BlockStreamIteratorBase* physical_iterator_tree=root->getIteratorTree(64*1024);
	physical_iterator_tree->open();
	while(physical_iterator_tree->next(0));
	physical_iterator_tree->close();
	printf("Q1: execution time: %4.4f second.\n",getSecond(start));
	return 0;
}

static void init_single_node(bool master=true){
	Environment::getInstance(master);
	printf("Press any key to continue!\n");
	int input;
	scanf("%d",&input);
	Catalog* catalog=Environment::getInstance()->getCatalog();
	catalog->restoreCatalog();
}

static int JoinSuite(){
	unsigned repeated_times=1;
	init_single_node();
	for(unsigned i=0;i<repeated_times;i++){
		SortmergejoinSuite();
		cout<<"***************************"<<endl;
	}
	Environment::getInstance()->~Environment();
	return 0;
}
