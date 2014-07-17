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

int SortmergejoinSuite(){
	/*
	 * select sum(a+1)+count(a),b
	 * from T
	 * group by b
	 *
	 * notation: p a p s
	 * */
	unsigned long long int start=curtick();
	TableDescriptor* table=Environment::getInstance()->getCatalog()->getTable("LINEITEM");
	//===========================scan===========================
	LogicalOperator* scan=new LogicalScan(table->getProjectoin(0));

	LogicalOperator* project1=new LogicalProject(scan,expr_list1);

	LogicalOperator* root=new LogicalQueryPlanRoot(0,project1,LogicalQueryPlanRoot::PERFORMANCE);

	cout<<"performance is ok!"<<endl;
	BlockStreamIteratorBase* physical_iterator_tree=root->getIteratorTree(64*1024);
//	physical_iterator_tree->print();
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

	/////////////////////////////// LINEITEM TABLE //////////////////////////////////
	TableDescriptor* table_1=new TableDescriptor("LINEITEM",0);
	table_1->addAttribute("row_id", data_type(t_u_long));
	table_1->addAttribute("L_ORDERKEY",data_type(t_u_long));  				//0
	table_1->addAttribute("L_PARTKEY",data_type(t_u_long));
	table_1->addAttribute("L_SUPPKEY",data_type(t_u_long));
	table_1->addAttribute("L_LINENUMBER",data_type(t_u_long));
	table_1->addAttribute("L_QUANTITY",data_type(t_decimal));
	table_1->addAttribute("L_EXTENDEDPRICE",data_type(t_decimal));
	table_1->addAttribute("L_DISCOUNT",data_type(t_decimal));
	table_1->addAttribute("L_TEX",data_type(t_decimal));
	table_1->addAttribute("L_RETURNFLAG",data_type(t_string),1);
	table_1->addAttribute("L_LINESTATUS",data_type(t_string),1);
	table_1->addAttribute("L_SHIPDATE",data_type(t_date));
	table_1->addAttribute("L_COMMITDATE",data_type(t_date));
	table_1->addAttribute("L_RECEIPTDATE",data_type(t_date));
	table_1->addAttribute("L_SHIPINSTRUCT",data_type(t_string),25);
	table_1->addAttribute("L_SHIPMODE",data_type(t_string),10);
	table_1->addAttribute("L_COMMENT",data_type(t_string),44);

	table_1->createHashPartitionedProjectionOnAllAttribute(table_1->getAttribute(1).getName(),1);

	catalog->add_table(table_1);

	for(unsigned i=0;i<table_1->getProjectoin(0)->getPartitioner()->getNumberOfPartitions();i++){

		catalog->getTable(0)->getProjectoin(0)->getPartitioner()->RegisterPartition(i,1);
	}
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
