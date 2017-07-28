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
 * /Claims/loader/test/data_injector_test.h
 *
 *  Created on: Oct 30, 2015
 *      Author: yukai
 *		   Email: yukai2014@gmail.com
 *
 * Description:
 *
 */

#ifndef LOADER_TEST_DATA_INJECTOR_TEST_H_
#define LOADER_TEST_DATA_INJECTOR_TEST_H_

#include <gtest/gtest.h>
#include <iosfwd>
#include <string>
#include "../../loader/data_injector.h"
#include "../../catalog/catalog.h"
#include "../common/memory_handle.h"

using std::ofstream;
using claims::catalog::Catalog;

namespace claims {
namespace loader {

class DataInjectorTest : public ::testing::Test {
 public:
  DataInjectorTest() {}
  virtual ~DataInjectorTest() {}

 public:
  //  Client client_;
  std::string ip_;
  int port_;
};

TEST_F(DataInjectorTest, GetTuple1) {
  char* data[2] = {"sf|sfs||sfssfs|\n", "sfsf\fw|fsfegbf,s,nn|fwnelweh\n"};

  ofstream ofs("test_for_DI_GetTuple");
  for (int i = 0; i < 2; ++i) ofs << data[i];

  ifstream ifs("test_for_DI_GetTuple");
  string res;
  string ter("\n");
  int i = 0;
  while (DataInjector::GetTupleTerminatedBy(ifs, res, ter)) {
    EXPECT_STREQ(data[i++], res.c_str());
    res.clear();
  }
}

TEST_F(DataInjectorTest, GetTuple2) {
  char* data[2] = {"sf|sfs||sfssfs|\n", "sfsf\fw|fsfegbf,s,nn|fwnelweh\n|\n"};

  ofstream ofs("test_for_DI_GetTuple");
  for (int i = 0; i < 2; ++i) ofs << data[i];

  ifstream ifs("test_for_DI_GetTuple");
  string res;
  string ter = "|\n";
  int i = 0;
  while (DataInjector::GetTupleTerminatedBy(ifs, res, ter)) {
    EXPECT_STREQ(data[i++], res.c_str());
    res.clear();
  }
}
TEST_F(DataInjectorTest, GetTuple3) {
  char* data[3] = {"sf|sfs||sfssfs|\n", "sfsf\fw|fsfegbf,s,nn|fwnelweh\n|\n",
                   "|\n"};

  ofstream ofs("test_for_DI_GetTuple");
  for (int i = 0; i < 3; ++i) ofs << data[i];

  ifstream ifs("test_for_DI_GetTuple");
  string res;
  string ter = "|\n";
  int i = 0;
  while (DataInjector::GetTupleTerminatedBy(ifs, res, ter)) {
    EXPECT_STREQ(data[i++], res.c_str());
    res.clear();
  }
}

TEST_F(DataInjectorTest, GetTuple4) {
  char* data[4] = {"sf|sfs||sfssfs||||",
                   "sfsf\fw|fsfe|||gbf,s,nn|fwnelweh\n||||", "|\n||||", "sfsf"};

  ofstream ofs("test_for_DI_GetTuple");
  for (int i = 0; i < 4; ++i) ofs << data[i];

  ifstream ifs("test_for_DI_GetTuple");
  string res;
  string ter = "||||";
  int i = 0;
  while (DataInjector::GetTupleTerminatedBy(ifs, res, ter)) {
    EXPECT_STREQ(data[i++], res.c_str());
    res.clear();
  }
}
TEST_F(DataInjectorTest, PrepareForLoadFromHdfs){

}
/*
TEST_F(DataInjectorTest, GetTupleFromHdfs){
	char* data[5] = {
			"0|AFRICA|lar deposits. blithely final packages cajole. regular waters are final requests. regular accounts are according to |\n",
			"1|AMERICA|hs use ironic, even requests. s|\n",
			"2|ASIA|ges. thinly even pinto beans ca|\n",
			"3|EUROPE|ly final courts cajole furiously final excuse|\n",
			"4|MIDDLE EAST|uickly special accounts cajole carefully blithely close requests. carefully final asymptotes haggle furiousl|\n"
			};
	HdfsLoader* hdfsloader_ = new HdfsLoader();
	hdfsloader_->PrepareForLoadFromHdfs();
	//printf("open the hdfsloader_");
	string file_name = "/test/claims/region.tbl";
	hdfsloader_->CheckHdfsFile(file_name);
	hdfsloader_->OpenHdfsFile(file_name);
	//printf("open the file_name");
	int pos = 0;
	int read_num = 0;
	string row_separator_ = " ";
	int length = 64;
	string tuple_record = "";
	void * buffer_ = claims::common::Malloc(length + 1);
	int i = 0;
	while(DataInjector::GetTupleTerminatedByFromHdfs(buffer_,hdfsloader_, file_name, tuple_record,
					  row_separator_, pos, read_num, length) ){
		EXPECT_STREQ(data[i], data[i++]);
				//tuple_record.c_str());
		tuple_record.clear();
	}

}

*/



} /* namespace loader */
} /* namespace claims */

#endif  // LOADER_TEST_DATA_INJECTOR_TEST_H_
