/*
 * Copyright [2012-2015] DaSE@ECNU
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * /CLAIMS/stmt_handler/load_exec.h
 *
 *  Created on: feb 22, 2017
 *      Author: hurry_huang
 *	     Email: hurry.huang@infosys.com
 *
 * Description:
 *      this file contains one class about load data from hdfs.
 *
 */

#ifndef STMT_HANDLER_HDFS_LOAD_EXEC_H
#define STMT_HANDLER_HDFS_LOAD_EXEC_H

//#include "../stmt_handler/stmt_exec.h"
#include <hdfs.h>
#include <string>
#include "../common/error_define.h"
using std::string;

namespace claims{
namespace loader{
/**
 * @brief
 * @details
 */
class HdfsLoader{
public:
	HdfsLoader();
	virtual ~HdfsLoader();
	//RetCode Execute (ExecutedResult* exec_result);
	 RetCode CheckHdfsFile(string file_name);
	 RetCode PrepareForLoadFromHdfs();
	 int GetCharFromBuffer(void*& buffer, int & pos, int & read_num, const int & length,
			 	 	 	 	 	 	 	  int & total_read_num);
	 RetCode GetFromHdfs(const string& file_name,int & length);
	 RetCode OpenHdfsFile(string & file_name);
	 RetCode CloseHdfsFile();
private:
	//AstLoadTable* load_ast_;
	hdfsFS fs_;
	hdfsFile file_;
	//common::FileHandleImp* imp_;

};


}
}

#endif
