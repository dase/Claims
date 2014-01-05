/*
 * SchemaVar.cpp
 *
 *  Created on: 2013-12-25
 *      Author: casa
 */

#include "SchemaVar.h"

SchemaVar::SchemaVar(std::vector<column_type> columns):Schema(columns) {
	attributes_=columns.size();
	for(unsigned i=0;i<columns.size();i++){
		if(columns[i].type==t_string)
			var_attributes_++;
	}
}

SchemaVar::~SchemaVar() {

}
