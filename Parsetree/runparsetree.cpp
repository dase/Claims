/*
 * runparsetree.cpp
 *
 *  Created on: 2014年1月21日
 *      Author: imdb
 */
#ifndef __RUNPARSER__
#define __RUNPARSER__
#include <sys/time.h>
#include "sql_node_struct.h"
#include <algorithm>
#include <vector>
//#include "../Environment.h"
extern "C" int yylex();
extern "C" int yyparse();
//extern "C" int yyclearin();
//extern "C" int yyerrok();
extern Node * parsetreeroot;
extern char globalInputText[10000];
extern int globalReadOffset;
extern int errorNumber;		// 2014-3-6---声明errorNumber变量---by余楷
extern timeval start_time;	//2014-5-4---add---by Yu

extern vector<Node*> NodePointer;	// 2014-4-2---存放节点指针的数组改为vector---by Yu

static Node * getparsetreeroot()
{
	puts("SQL!!!!!");
	int charnum=0;
	globalReadOffset = 0;

	NodePointer.clear();	// 2014-3-7---初始化---by余楷	// 2014-4-2---存放节点指针的数组改为vector---by Yu
	errorNumber = 0;	// 2014-3-6---初始化SQL解析错误个数---by余楷
	parsetreeroot = NULL;	// 2014-3-4---增加初始化语句---by余楷
	memset(globalInputText, 0, sizeof(globalInputText));	// 2014-3-4---增加初始化语句---by余楷
	cout<<"globalInputText:"<<globalInputText;


	while(1)
	{
		char c=getchar();
		globalInputText[charnum++]=c;
		if(c==';')
		{
			globalInputText[charnum++]='\0';
//			getchar();
			break;
		}
	}

	// 2014-3-4---测试数据---by余楷
//	strcpy(globalInputText,
//			"select row_id from cj;\n"
//			"create table tt(num int not null, s int, ss int primary key, c int, vc varchar(15));\n"
//			"create projection on tt(num, s, c) partitioned on num;\n"
//			"select s from tt;\n"
//			"create projection on tt(ss, c) partitioned on ss;\n"
//			"select ss from tt;\n"
//			"create table t(num int, d double, f float);\n"
//			"create table tt(num int, d double, f float);\n"
//			"create table ttt(num int, d double, f float);\n"
//			"create projection on t(num, f, d) partitioned on num;\n"
//			"select f,d from t;\n"
//			);
	cout<<"globalInputText:"<<globalInputText<<endl;

//	yyclearin;
//	yychar = YYEMPTY;
//	yyerrok;
//	cleanSQLparse();

	gettimeofday(&start_time, NULL);//2014-5-4---add---by Yu

	if(!yyparse())	// 2014-3-4---注释冗余代码---by余楷
	{
		return parsetreeroot;
	}
	else
	{
		printf("SQL parse failed\n");
		return NULL;
	}
}

static Node * getparsetreeroot(const char *sql)
{
	puts("SQL!!!!!");
	int charnum=0;
	globalReadOffset = 0;

	NodePointer.clear();	// 2014-3-7---初始化---by余楷	// 2014-4-2---存放节点指针的数组改为vector---by Yu
	errorNumber = 0;	// 2014-3-6---初始化SQL解析错误个数---by余楷
	parsetreeroot = NULL;	// 2014-3-4---增加初始化语句---by余楷
	memset(globalInputText, 0, sizeof(globalInputText));	// 2014-3-4---增加初始化语句---by余楷
	cout<<"globalInputText:"<<globalInputText;

	// 2014-3-4---测试数据---by余楷
	strcpy(globalInputText,	sql);
	cout<<"globalInputText:"<<globalInputText<<endl;

	if(!yyparse())	// 2014-3-4---注释冗余代码---by余楷
	{
		return parsetreeroot;
	}
	else
	{
		printf("SQL parse failed\n");
		return NULL;
	}
}


#endif
