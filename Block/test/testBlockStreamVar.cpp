#include "../BlockStream.h"
#include "../../Schema/SchemaVar.h"
#include "../../rename.h"
#include <string>
using namespace std;

int main88(){
	int i1=1;
	int i2=2;
    char* str="string1";

	int filed;
	filed=FileOpen("/home/casa/storage/file/file_64M",O_WRONLY|O_CREAT);

	std::vector<column_type> vc;
	vc.push_back(column_type(t_int));
	vc.push_back(column_type(t_double));
	vc.push_back(column_type(t_string));
	Schema *sch=new SchemaVar(vc);

	int offset=0;
	while(true){
	if(offset>=1024)
		break;
	BlockStreamBase *bs=new BlockStreamVar(64*1024,sch);
//	bs->printSchema();


	for(unsigned i=0;i<1000000;i++){
	void * ins=bs->allocateTuple(8+2*4+4);

	if(ins==0){
		break;
	}else{
	void *pstr=str;

	void *tuple1=malloc(8+3*4);
	int *tuple=(int*)tuple1;
//	cout<<"error"<<endl;
	*tuple=8;
	*(tuple+1)=i1;
	*(tuple+2)=i2;
	void *tuple_=tuple1+3*4;
	memcpy(tuple_,str,8);
//	cout<<"error"<<endl;
	if(bs->insert(ins,tuple1,8+3*4))
//		cout<<"it's true!!!"<<endl;
	bs->serialize(*bs);
//	bs->printSchema();
	}
	}
	lseek(filed,64*1024*offset,SEEK_SET);
	write(filed,bs->getBlock(),1024*64);
	offset++;
	}
//	bs->toDisk(bs);

	return true;
}
