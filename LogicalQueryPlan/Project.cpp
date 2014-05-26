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
	mappings_=getMapping();
	Dataflow ret;
	const Dataflow child_dataflow=child_->getDataflow();
	ret.property_.commnication_cost=child_dataflow.property_.commnication_cost;
	ret.property_.partitioner=child_dataflow.property_.partitioner;
	std::vector<Attribute> ret_attrs;

	Schema *input_=getSchema(child_dataflow.attribute_list_);
	for(unsigned i=0;i<exprArray_.size();i++){
		int seq=-1;
		for(unsigned j=0;j<exprArray_[i].size();j++){
			if(exprArray_[i][j].type==ExpressionItem::variable_type){
				seq++;
				exprArray_[i][j].return_type=input_->getcolumn(mappings_.getMapping()[i][seq]).type;
			}
		}
	}

	for(unsigned i=0;i<mappings_.getMapping().size();i++){
		cout<<"mappings_.getMappings().size(): "<<mappings_.getMapping().size()<<endl;
		if(exprArray_[i].size()>1){
			//在这里是要将新名字赋给dataflow中的attribute name的
			string alias=recovereyName(exprArray_[i]);
			column_type column=ExpressionCalculator::getOutputType_(exprArray_[i]);
			//currently,0 is to table_id TODO;
			Attribute attr_alais(0,i,alias,column.type,column.get_length());
			ret_attrs.push_back(attr_alais);
		}
		else{
			Attribute tmp=child_dataflow.attribute_list_[mappings_.getMapping()[i][0]];
			Attribute attr(0,i,tmp.getName(),tmp.attrType->type,tmp.attrType->get_length());
			ret_attrs.push_back(attr);
//			ret_attrs.push_back(child_dataflow.attribute_list_[mappings_.getMapping()[i][0]]);
		}
//		getchar();
	}
	ret.attribute_list_=ret_attrs;
	dataflow_=new Dataflow();
	*dataflow_=ret;
	return ret;
}

BlockStreamIteratorBase *LogicalProject::getIteratorTree(const unsigned& blocksize){
//	mappings_=getMapping();
	getDataflow();
	Dataflow child_dataflow=child_->getDataflow();
	BlockStreamIteratorBase *child=child_->getIteratorTree(blocksize);
	BlockStreamProjectIterator::State state;
	state.block_size_=blocksize;
	state.children_=child;
	state.v_ei_=exprArray_;
	state.input_=getSchema(child_dataflow.attribute_list_);
	state.map_=mappings_;
	state.output_=getOutputSchema();
	/*
	 * the output schema has the column type which has the data type and the data length
	 * like the column type string, _string and sizeof(_string)
	 * */
	return new BlockStreamProjectIterator(state);
}

Schema *LogicalProject::getOutputSchema(){
	Schema *schema=getSchema(dataflow_->attribute_list_);
	return schema;
//	//must scan the expression and get the output schema
//	vector<column_type> column_list;
//	Dataflow child_dataflow=child_->getDataflow();
//	Schema *input_=getSchema(child_dataflow.attribute_list_);
//	for(unsigned i=0;i<exprArray_.size();i++){
////		int seq=-1;
////		for(unsigned j=0;j<exprArray_[i].size();j++){
////			if(exprArray_[i][j].type==ExpressionItem::variable_type){
////				seq++;
////				exprArray_[i][j].return_type=input_->getcolumn(mappings_.getMapping()[i][seq]).type;
////			}
////		}
//		/*
//		//如果是获得输出的类型，就用原来的算一遍
//		data_type rt_type_per_expression=ExpressionCalculator::getOutputType(exprArray_[i]);
//		//只是试试：-》
//		column_type column_schema=*dataflow_.attribute_list_[mappings_.getMapping()[i][0]].attrType;
//
//		//TODO:设计一个功能，这个功能在输入的expressions选出schema，schema中是column_type
////		column_type column_schema(rt_type_per_expression);*/
//
//		column_type column_schema=ExpressionCalculator::getOutputType_(exprArray_[i]);
////		cout<<"column_type len: "<<column_schema.get_length()<<endl;
//		column_list.push_back(column_schema);
//	}
//	Schema *rt_schema=new SchemaFix(column_list);
//	return rt_schema;
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

//string LogicalProject::recovereyName(Expression &ei) {
//	string ret;
//	/* 将后缀表达式变成中缀表达式 */
//	stringstream ss;
//	Expression exp1,exp2;
//	Exp_op current;
//	for(unsigned i=0;i<ei.size();i++){
//		if(ei[i].type==ExpressionItem::const_type ||ei[i].type==ExpressionItem::variable_type){
//			if(i==0){
//				exp1.push_back(ei[i]);
//				current.expr=exp1;
//				current.op=0;
//				continue;
//			}
//			else{
//				exp2.push_back(ei[i]);
//				continue;
//			}
//		}
//		if(ei[i].type==ExpressionItem::operator_type){
//			if(ei[i].getOperatorName()=="+"||ei[i].getOperatorName()=="-"){
//				Expression tmp;
//				for(unsigned k=0;k<current.expr.size();k++){
//					tmp.push_back(current.expr[k]);
//				}
//				tmp.push_back(ei[i]);
//				for(unsigned l=0;l<exp2.size();l++){
//					tmp.push_back(exp2[l]);
//				}
//				exp2.clear();
//				current.expr=tmp;
//				current.op=1;//+和-的优先级为0
//			}
//			if(ei[i].getOperatorName()=="*"||ei[i].getOperatorName()=="/"){
//				Expression tmp1;
//				if(current.op==2){
//					for(unsigned m=0;m<current.expr.size();m++){
//						tmp1.push_back(current.expr[m]);
//					}
//					tmp1.push_back(ei[i]);
//					for(unsigned n=0;n<exp2.size();n++){
//						tmp1.push_back(exp2[n]);
//					}
//					exp2.clear();
//					current.expr=tmp1;
//					current.op=2;
//				}
//				if(current.op==1){
//					//加括号
//					ExpressionItem black_left,black_right;
//					black_left.item_name='(';
//					black_right.item_name=')';
//					tmp1.push_back(black_left);
//					for(unsigned m=0;m<current.expr.size();m++){
//						tmp1.push_back(current.expr[m]);
//					}
//					tmp1.push_back(black_right);
//					tmp1.push_back(ei[i]);
//					for(unsigned n=0;n<exp2.size();n++){
//						tmp1.push_back(exp2[n]);
//					}
//					exp2.clear();
//					current.expr=tmp1;
//					current.op=2;
//				}
//			}
//		}
//	}
//	cout<<"current.expr.size(): "<<current.expr.size()<<endl;
//	for(unsigned i=0;i<current.expr.size();i++){
//		ss<<current.expr[i].item_name<<' ';
//	}
//	cout<<"recoveryName: "<<ss.str().c_str()<<endl;
//	getchar();
//	return ret;
//}

string LogicalProject::recovereyName(Expression ei) {
	string ret;
	/* 将后缀表达式变成中缀表达式 */
	stringstream ss;
	Exp_op_tmp exop_ini;
	for(unsigned i=0;i<ei.size();i++){
		Exp_op ini;
		ini.expr.push_back(ei[i]);
		ini.op=0;
		exop_ini.push_back(ini);
	}

	Exp_op_tmp current;
	while(exop_ini.size()>1){
		cout<<"in the while loop！"<<endl;
	for(unsigned i=0;i<exop_ini.size();i++){
		if(exop_ini[i].op==2){
			continue;
		}
		if(exop_ini[i].op==1){
			continue;
		}
		if(exop_ini[i].op==0){
			if(exop_ini[i].expr[0].type==ExpressionItem::operator_type){
				if(exop_ini[i].expr[0].getOperatorName()=="+"||exop_ini[i].expr[0].getOperatorName()=="-"){
					//将前两个i-1和i-2加起来
					Exp_op op;
					for(unsigned m=0;m<exop_ini[i-2].expr.size();m++){
						op.expr.push_back(exop_ini[i-2].expr[m]);
					}
					op.expr.push_back(exop_ini[i].expr[0]);
					for(unsigned n=0;n<exop_ini[i-1].expr.size();n++){
						op.expr.push_back(exop_ini[i-1].expr[n]);
					}
					op.op=1;
					for(unsigned j=0;j<exop_ini.size();j++){
						if((j>=i-2)&&(j<=i)){
							if(j==i){
								current.push_back(op);
								continue;
							}
							else
								continue;
						}
						else
							current.push_back(exop_ini[j]);
					}
					exop_ini=current;
					current.clear();
					break;
				}
				else{
					//将前两个相乘，看是否有括号
					if(exop_ini[i-1].op==1||exop_ini[i-2].op==1){
						Exp_op op;
						ExpressionItem bl;
						ExpressionItem br;
						bl.item_name="(";
						br.item_name=")";
						if(exop_ini[i-2].op==1){
							op.expr.push_back(bl);
							for(unsigned m=0;m<exop_ini[i-2].expr.size();m++){
								op.expr.push_back(exop_ini[i-2].expr[m]);
							}
							op.expr.push_back(br);
						}
						else{
							for(unsigned m=0;m<exop_ini[i-2].expr.size();m++){
								op.expr.push_back(exop_ini[i-2].expr[m]);
							}
						}
						op.expr.push_back(exop_ini[i].expr[0]);
						if(exop_ini[i-1].op==1){
							op.expr.push_back(bl);
							for(unsigned n=0;n<exop_ini[i-1].expr.size();n++){
								op.expr.push_back(exop_ini[i-1].expr[n]);
							}
							op.expr.push_back(br);
						}
						else{
							for(unsigned n=0;n<exop_ini[i-1].expr.size();n++){
								op.expr.push_back(exop_ini[i-1].expr[n]);
							}
						}
						op.op=2;
						for(unsigned j=0;j<exop_ini.size();j++){
							if((j>=i-2)&&(j<=i)){
								if(j==i){
									current.push_back(op);
									continue;
								}
								else
									continue;
							}
							else
								current.push_back(exop_ini[j]);
						}
						exop_ini=current;
						current.clear();
						break;
					}
					else{
						Exp_op op;
						for(unsigned m=0;m<exop_ini[i-2].expr.size();m++){
							op.expr.push_back(exop_ini[i-2].expr[m]);
						}
						op.expr.push_back(exop_ini[i].expr[0]);
						for(unsigned n=0;n<exop_ini[i-1].expr.size();n++){
							op.expr.push_back(exop_ini[i-1].expr[n]);
						}
						op.op=2;
						for(unsigned j=0;j<exop_ini.size();j++){
							if((j>=i-2)&&(j<=i)){
								if(j==i){
									current.push_back(op);
									continue;
								}
								else
									continue;
							}
							else
								current.push_back(exop_ini[j]);
						}
						exop_ini=current;
						current.clear();
						break;
					}
				}
			}
			else{
				continue;
			}
		}
	}
	}

	cout<<"current.expr.size(): "<<exop_ini[0].expr.size()<<endl;
	for(unsigned i=0;i<exop_ini[0].expr.size();i++){
		ss<<exop_ini[0].expr[i].item_name;
	}
	cout<<"recoveryName: "<<ss.str().c_str()<<endl;
	ret=ss.str();
	return ret;
}
