/*
 * SchemaVar.h
 *
 *  Created on: 2013-12-25
 *      Author: casa
 */

#ifndef SCHEMAVAR_H_
#define SCHEMAVAR_H_
#include <assert.h>
#include <vector>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include "Schema.h"
#include "../data_type.h"

class SchemaVar:public Schema {
public:
	SchemaVar(std::vector<column_type> columns);

	SchemaVar(){};

	virtual ~SchemaVar();

	virtual unsigned getTupleMaxSize(){};
	inline virtual unsigned getTupleActualSize(void* tuple) const{
		unsigned ofs=0;
		unsigned column_off=0;
		for(unsigned i=0;i<columns.size();i++){
			if(columns[i].type!=t_string){
				ofs+=columns[i].get_length();
			}
			else{
				ofs+=*(int *)(tuple+column_off*4);
				column_off++;
			}
		}
		ofs+=var_attributes_*4;
		return ofs;
	};

	/*get the pointer of the tuple and get the value of the tuple offset*/
	virtual void getColumnValue(unsigned index,void* src, void* desc){

	};

	inline virtual void* getColumnAddess(unsigned index,const void* const & column_start) const __attribute__((always_inline)){
		unsigned ofs=0;
		unsigned column_off=0;
		void *ret;
		if(index>0){
			for(unsigned i=0;i<index;i++){
				if(columns[i].type!=t_string){
					ofs+=columns[i].get_length();
				}
				else{
					ofs+=*(int *)(column_start+column_off*4);
					column_off++;
				}
			}
			return (char *)column_start+ofs+var_attributes_*4;
		}
		else{
			return (char *)column_start+var_attributes_*4;
		}
	};

	inline virtual unsigned copyTuple(void* src, void* desc) const{
		memcpy(desc,src,getTupleActualSize(src));
	};

	virtual Schema::schema_type getSchemaType(){
		return Schema::varaible;
	};

private:
	unsigned var_attributes_;
	unsigned attributes_;
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive &ar, const unsigned int version){
		ar & boost::serialization::base_object<Schema>(*this);
	}
};

#endif /* SCHEMAVAR_H_ */
