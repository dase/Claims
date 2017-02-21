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
 * /Claims/loader/Validity.cpp
 *
 *  Created on: Nov 17, 2015
 *      Author: yukai
 *		   Email: yukai2014@gmail.com
 *
 * Description:
 *
 */

#include "./validity.h"
#include <string>
#include "../catalog/table.h"

using claims::catalog::TableDescriptor;
using namespace claims::common;  // NOLINT
namespace claims {
namespace loader {

Validity::~Validity() {}

string Validity::GenerateDataValidityInfo(const Validity& vali,
                                          const TableDescriptor* table,
                                          int line, const string& file) {
  ostringstream oss;
  oss.clear();
  switch (vali.check_res_) {
    case rTooLargeData: {
      oss << "Data larger than range value for column '"
          << table->getAttribute(vali.column_index_).attrName
          << "' at line: " << line;
      if ("" != file) oss << " in file: " << file;
      oss << "\n";
      break;
    }
    case rTooSmallData: {
      oss << "Data smaller than range value for column '"
          << table->getAttribute(vali.column_index_).attrName
          << "' at line: " << line;
      if ("" != file) oss << " in file: " << file;
      oss << "\n";
      break;
    }
    case rInterruptedData: {
      oss << "Data truncated from non-digit for column '"
          << table->getAttribute(vali.column_index_).attrName
          << "' at line: " << line;
      if ("" != file) oss << " in file: " << file;
      oss << "\n";
      break;
    }
    case rTooLongData: {
      oss << "Data truncated for column '"
          << table->getAttribute(vali.column_index_).attrName
          << "' at line: " << line;
      if ("" != file) oss << " in file: " << file;
      oss << "\n";
      break;
    }
    case rIncorrectData: {
      oss << "Incorrect format value for column '"
          << table->getAttribute(vali.column_index_).attrName
          << "' at line: " << line;
      if ("" != file) oss << " in file: " << file;
      oss << "\n";
      break;
    }
    case rInvalidNullData: {
      oss << "Null Data value is invalid for column '"
          << table->getAttribute(vali.column_index_).attrName
          << "' at line: " << line;
      if ("" != file) oss << " in file: " << file;
      oss << "\n";
      break;
    }
    case rTooFewColumn: {
      oss << "Line: " << line;
      if ("" != file) oss << " in file: " << file;
      oss << " doesn't contain data for all columns\n";
      break;
    }
    case rTooManyColumn: {
      oss << "Line: " << line;
      if ("" != file) oss << " in file: " << file;
      oss << " was truncated; it contained more data than there were "
             "input columns\n";
      break;
    }
    case rInvalidInsertData: {
      oss << "Data value is invalid for column '"
          << table->getAttribute(vali.column_index_).attrName
          << "' at line: " << line;
      if ("" != file) oss << " in file: " << file;
      oss << "\n";
      break;
    }
    default:
      LOG(ERROR) << "Unknown ERROR" << endl;
      oss << "Unknown ERROR\n";
      break;
  }
  return oss.str();
}

} /* namespace loader */
} /* namespace claims */
