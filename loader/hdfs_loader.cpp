/*
 * Copyright [2012-2017] DaSE@ECNU
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
 * /CLAIMS/stmt_handler/load_exec.cpp
 *
 *  Created on: Feb 22, 2017
 *      Author: hurry.huang
 *       Email: hurry.huang@infosys.com
 *
 * Description:
 *    this file is the function body of class LoadExec.
 */

//#include <string>
#include "hdfs_loader.h"
#include "../common/error_define.h"
#include "../common/file_handle/hdfs_connector.h"
#include <string>
using namespace std;
using namespace claims::common;

namespace claims{
namespace loader{

	HdfsLoader::HdfsLoader(){

	}
	HdfsLoader::~HdfsLoader(){
		if(NULL == file_){
			LOG(INFO) <<"hdfs file for load from hdfs has been closed."<<endl;
		}
		assert(NULL != fs_ && "failed to connect hdfs");
		if (0 != hdfsCloseFile(fs_, file_)){
			PLOG(ERROR) << "failed to close hdfs file for load from hdfs."<<endl;
			return ;
		}
		file_ = NULL;
		LOG(INFO) << "The hdfs for load from hdfs has been closed."<<endl;
	}
	RetCode HdfsLoader::CheckHdfsFile(string file_name){
		int ret = rSuccess;
		hdfsFileInfo* hdfsfile = hdfsGetPathInfo(fs_,file_name.c_str());
		if(NULL == hdfsfile){
			ret = rFailure;
		}
		return ret;

	}

	RetCode HdfsLoader::PrepareForLoadFromHdfs(){
		int ret = rSuccess;
		fs_ = claims::common::HdfsConnector::Instance();
		if (NULL == fs_)
			ret = rFailure;
		return ret;
	}

	RetCode HdfsLoader::CloseHdfsFile(){
		  hdfsCloseFile(fs_, file_);

	}

	int HdfsLoader::GetCharFromBuffer(void*& buffer, int & pos, int & read_num, const int & length,
													int & total_read_num){
		if(pos == read_num){
			read_num = hdfsRead(fs_,file_,static_cast<char*>(buffer),length);
			pos = 0;
			if(read_num == 0){
				pos = 0;

				return -1;

			}
		}
		total_read_num++;
		return *((char*)buffer + pos++);
		//return ' ';

	}

	RetCode HdfsLoader::OpenHdfsFile(string & file_name){
		int ret = rSuccess;
		file_ = hdfsOpenFile(fs_, file_name.c_str(),O_RDONLY,0,0,0);
		if (!file_) {
			ret = rFailure;
	  }
		return ret;



	}



}
}
