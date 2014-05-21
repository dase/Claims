/*
 * Project.cpp
 *
 *  Created on: 2014-2-21
 *      Author: casa
 */

#include "Project.h"

LogicalProject::LogicalProject(LogicalOperator *child, std::vector<std::vector<ExpressionItem> > &exprArray)
:child_(child),exprArray_(exprArray){
	initialize_arithmetic_type_promotion_matrix();
	initialize_type_cast_functions();
	setOperatortype(l_project);
}

LogicalProject::~LogicalProject(){
	dataflow_->~Dataflow();
	if(child_>0){
		child_->~LogicalOperator();
	}
}

Dataflow LogicalProject::getDataflow(){
	Dataflow ret;
	const Dataflow child_dataflow=child_->getDataflow();
	ret.property_.commnication_cost=child_dataflow.property_.commnication_cost;
	ret.property_.partitioner=child_dataflow.property_.partitioner;
	std::vector<Attribute> ret_attrs;
	for(unsigned i=0;i<mappings_.getMapping().size();i++){
		ret_attrs.push_back(child_dataflow.attribute_list_[mappings_.getMapping()[i][0]]);
	}
	ret.attribute_list_=ret_attrs;
	dataflow_=new Dataflow();
	*dataflow_=ret;
	return ret;
}

BlockStreamIteratorBase *LogicalProject::getIteratorTree(const unsigned& blocksize){
	mappings_=getMapping();
	getDataflow();
	Dataflow child_dataflow=child_->getDataflow();
	BlockStreamIteratorBase *child=child_->getIteratorTree(blocksize);
	BlockStreamProjectIterator::State state;
	state.block_size_=blocksize;
	state.children_=child;
	state.v_ei_=exprArray_;
	state.input_=getSchema(child_dataflow.attribute_list_);
	state.map_=mappings_;
	/*
	 * the output schema has the column type which has the data type and the data length
	 * like the column type string, _string and sizeof(_string)
	 * */
	state.output_=getOutputSchema();
	return new BlockStreamProjectIterator(state);
}

Schema *LogicalProject::getOutputSchema(){
	//must scan the expression and get the output schema
	vector<column_type> column_list;
	Dataflow child_dataflow=child_->getDataflow();
	Schema *input_=getSchema(child_dataflow.attribute_list_);
	for(unsigned i=0;i<exprArray_.size();i++){
		for(unsigned j=0;j<exprArray_[i].size();j++){
			if(exprArray_[i][j].type==ExpressionItem::variable_type){
				exprArray_[i][j].return_type=input_->getcolumn(mappings_.getMapping()[i][j]).type;
			}
		}
		/*
		//如果是获得输出的类型，就用原来的算一遍
		data_type rt_type_per_expression=ExpressionCalculator::getOutputType(exprArray_[i]);
		//只是试试：-》
		column_type column_schema=*dataflow_.attribute_list_[mappings_.getMapping()[i][0]].attrType;

		//TODO:设计一个功能，这个功能在输入的expressions选出schema，schema中是column_type
//		column_type column_schema(rt_type_per_expression);*/

		column_type column_schema=ExpressionCalculator::getOutputType_(exprArray_[i]);
//		cout<<"column_type len: "<<column_schema.get_length()<<endl;
		column_list.push_back(column_schema);
	}
	Schema *rt_schema=new SchemaFix(column_list);
	return rt_schema;
}

Mapping LogicalProject::getMapping(){
	Mapping mappings;
	for(unsigned i=0;i<exprArray_.size();i++){
		ExpressionMapping em;
		for(unsigned j=0;j<exprArray_[i].size();j++){
			if(exprArray_[i][j].type==ExpressionItem::variable_type)
				em.push_back(getColumnSeq(exprArray_[i][j]));
		}
		mappings.atomicPushExpressionMapping(em);
	}
	return mappings;
}

int LogicalProject::getColumnSeq(ExpressionItem &ei){
	int rt;
	/*every time invoke a getColumnSeq, you need to new a catalog--@li: it seams that you actually get
	 * the reference to the catalog rather than creating one.
	*/
	Dataflow child_dataflow=child_->getDataflow();
	for(unsigned i=0;i<child_dataflow.attribute_list_.size();i++){
		TableDescriptor *table=Catalog::getInstance()->getTable(child_dataflow.attribute_list_[i].table_id_);
		string tablename=table->getTableName();
		if((tablename.compare(ei.content.var.table_name)==0)&&(child_dataflow.attribute_list_[i].attrName.compare(ei.content.var.column_name)==0)){
			return i;
		}
	}
	printf("Variable ExpressItem fails to match any attribute in the dataflow!\n");
	assert(false);
}
