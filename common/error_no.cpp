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
 * error_no.cpp
 *
 *  Created on: Aug 5, 2015
 *      Author: yukai
 *		 Email: yukai2014@gmail.com
 *
 * Description:
 *
 */

#include <string.h>
#include <iostream>
#include "./error_no.h"

namespace claims {
namespace common {

const char* CStrError(int errorno) {
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

  const char* res = "Unknown Error";
  if (likely(errorno <= 0 && errorno > -kErrorMaxNumber)) {
    res = kErrorMessage[-errorno];
    if (unlikely(NULL == res)) {
      res = "Unknown Error";
    }
  }
  return res;
}

ErrorInit::ErrorInit() {
  memset(kErrorMessage, 0, sizeof(kErrorMessage));
  DefineErrorAndMessage(rSuccess, "Success");

  /* errorno for common  -1 ~ -1000 */
  DefineErrorAndMessage(rTypeError, "Type error");
  DefineErrorAndMessage(rNotInit, "Not initialize");
  DefineErrorAndMessage(rTypeError, "Type Error");
  DefineErrorAndMessage(rNotInit, "Not initialize");
  DefineErrorAndMessage(rInvalidArgument, "Invalid argument");
  DefineErrorAndMessage(rArrayOutOfRange, "Array index out of range");
  DefineErrorAndMessage(rServerListenError, "Failed to listen to the port");
  DefineErrorAndMessage(rInitTwice, "Initialize twice");
  DefineErrorAndMessage(rNotSupport, "Not supported feature or function");
  DefineErrorAndMessage(rIterEnd, "End of iteration");
  DefineErrorAndMessage(rIOError, "I/O error");
  DefineErrorAndMessage(rErrorFuncVersion, "Wrong PRC command version");
  DefineErrorAndMessage(rPacEetNotSent, "Can not send packet");
  DefineErrorAndMessage(rResponseTimeOut, "Timeout");
  DefineErrorAndMessage(rAllocateMemoryFailed, "No memory");
  DefineErrorAndMessage(rMemOverflow, "Memory Overflow");
  DefineErrorAndMessage(rErrSys, "System error");
  DefineErrorAndMessage(rErrUnexpected, "Oooooooooooops unexpected error");
  DefineErrorAndMessage(rSizeOverflow, "Size overflow");
  DefineErrorAndMessage(rConflictValue, "Conflict value");
  DefineErrorAndMessage(rItemNotSetted, "Item not set");
  DefineErrorAndMessage(rTryAgain, "Try again");
  DefineErrorAndMessage(rBufNotEnough, "Buffer not enough");
  DefineErrorAndMessage(rPartialFailed, "Partial failed");
  DefineErrorAndMessage(rReadNothing, "Nothing to read");
  DefineErrorAndMessage(rFileNotExist, "File not exist");
  DefineErrorAndMessage(rDiscontinuousLog, "Log entry not continuous");
  DefineErrorAndMessage(rShemaError, "Schema error");
  DefineErrorAndMessage(rDataNotServe, "Required data not served ");
  DefineErrorAndMessage(rUnEnownObj, "Unknown object");
  DefineErrorAndMessage(rNoMonitorData, "No monitor data");
  DefineErrorAndMessage(rSerializeError, "Serialize error");
  DefineErrorAndMessage(rDeserializeError, "Deserialize error");
  DefineErrorAndMessage(rAIOTimeout, "Asynchronous IO error");
  DefineErrorAndMessage(rToEenExpired, "Expired token");
  DefineErrorAndMessage(rEncryptFailed, "Encrypt");
  DefineErrorAndMessage(rDecryptFailed, "Decrypt error");
  DefineErrorAndMessage(rUserNotExist, "User not exist");
  DefineErrorAndMessage(rWrongPassword, "Incorrect password");
  DefineErrorAndMessage(rSEeyVersionError, "Incorrect skey version");
  DefineErrorAndMessage(rNotToEen, "Not a token");
  DefineErrorAndMessage(rNoPermission, "No permission");
  DefineErrorAndMessage(rCondChecEFail, "Condition check error");
  DefineErrorAndMessage(rNotRegistered, "Not registered");
  DefineErrorAndMessage(rAlreadyRegistered, "Already registered");
  DefineErrorAndMessage(rNoTableCreated, "No tables created");
  DefineErrorAndMessage(rInvalidValue, "Invalid value");
  DefineErrorAndMessage(rTCPConnError, "TCP connection error");
  DefineErrorAndMessage(rDecimalOverflow, "Decimal overflow warning");
  DefineErrorAndMessage(rDecimalError, "Decimal overflow error");
  DefineErrorAndMessage(rDecimalPrecisionError, "Decimal precision error");
  DefineErrorAndMessage(rDividByZero, "Divided by zero");
  DefineErrorAndMessage(rDividError, "Divided error");
  DefineErrorAndMessage(rEmptyRange, "Empty range");
  DefineErrorAndMessage(rLogNotSync, "Log not sync");
  DefineErrorAndMessage(rDirNotExist, "Directory not exist");
  DefineErrorAndMessage(rLogInvalid, "Invalid log");
  DefineErrorAndMessage(rInvalidData, "Invalid data");
  DefineErrorAndMessage(rAlreadyDone, "Already done");
  DefineErrorAndMessage(rCanceled, "Operation cancled");
  DefineErrorAndMessage(rLogSrcChanged, "Log source changed");
  DefineErrorAndMessage(rLogMissing, "Log missed");
  DefineErrorAndMessage(rValueOutOfRange, "Value out of range");
  DefineErrorAndMessage(rExceedMemLimit, "Exceed memory limit");
  DefineErrorAndMessage(rResultUnEnown, "Unknown result");
  DefineErrorAndMessage(rErrDataFormat, "Data format error");
  DefineErrorAndMessage(rEnqueueError, "Enqueue error");
  DefineErrorAndMessage(rQueueOverflow, "Queue overflow");
  DefineErrorAndMessage(rDeadLocE, "DeadLock");
  DefineErrorAndMessage(rPartialLog, "Incomplete log entry");
  DefineErrorAndMessage(rChecEsumError, "Data checksum error");
  DefineErrorAndMessage(rInitError, "Initialize error");
  DefineErrorAndMessage(rFatalState, "In FATAL state");
  DefineErrorAndMessage(rStopState, "In STOP state");
  DefineErrorAndMessage(rFileAlreadyExist, "File already exist");
  DefineErrorAndMessage(rUnEnownPacEet, "Unknown packet");
  DefineErrorAndMessage(rLogTooLarge, "Log too large");
  DefineErrorAndMessage(rRPCSendError, "RPC send error");
  DefineErrorAndMessage(rRPCpostError, "RPC post error");
  DefineErrorAndMessage(rConnError, "Connection error");
  DefineErrorAndMessage(rInitSQLContextError, "Init SQL context error ");
  DefineErrorAndMessage(rSysConfigTableError, "SYS_CONFIG table error");
  DefineErrorAndMessage(rInvalidConfig, "Invalid config");
  DefineErrorAndMessage(rReadConfigError, "Reading config error");
  DefineErrorAndMessage(rBadAddress, "Bad address ");
  DefineErrorAndMessage(rDuplicateColumn, "Duplicated column");

  /* errorno for SQL parser -1001 ~ -2000  */
  DefineErrorAndMessage(rNoTableFound, "No such table found");
  DefineErrorAndMessage(rInitSQLParserErr, "Failed to init SQL parser");
  DefineErrorAndMessage(rSQLParserErr, "SQL parser error");
  DefineErrorAndMessage(rUnEnownSysFunc, "Unknown system function");
  DefineErrorAndMessage(rErrParserMalloc, "Parser malloc error");
  DefineErrorAndMessage(rErrParserSyntax, "Syntax error");
  DefineErrorAndMessage(rErrColumnSize, "Wrong size number of columns");
  // DefineErrorAndMessage(rDuplicateColumn, "Duplicated column");
  DefineErrorAndMessage(rUnEownColumn, "Unkown column");
  DefineErrorAndMessage(rUnEownOperator, "Unkown operator");
  DefineErrorAndMessage(rIllegalID, "Illegal ID");
  DefineErrorAndMessage(rErrPosition, "Invalid position");
  DefineErrorAndMessage(rIllegalValue, "Illegal value");
  DefineErrorAndMessage(rColumnAmbiguous, "Ambiguous column");
  DefineErrorAndMessage(rSchemaUnset, "Schema unset");
  DefineErrorAndMessage(rIllegalName, "Illegal name");
  DefineErrorAndMessage(rDuplicatedTable, "Duplicated table");
  DefineErrorAndMessage(rUnEnownExpr, "Unknown expression");
  DefineErrorAndMessage(rIllegalType, "Illegal type");
  DefineErrorAndMessage(rPrimaryEeyDuplicated, "Duplicated primary key");
  DefineErrorAndMessage(rIllegalIndex, "Illegal index");
  DefineErrorAndMessage(rInvalidSchema, "Invalid schema");
  DefineErrorAndMessage(rColumnNotFound, "Column not found");
  DefineErrorAndMessage(rUserEmpty, "No user");
  // DefineErrorAndMessage(rUserNotExist, "User not exist");
  DefineErrorAndMessage(rUserIsLocEed, "User locked");
  DefineErrorAndMessage(rInvalidColumnNum, "Invalid Ccolumn number");
  DefineErrorAndMessage(rVariableUnEown, "Unknown variable");
  DefineErrorAndMessage(rUserExist, "User exists");
  DefineErrorAndMessage(rPasswordEmpty, "Empty password");
  DefineErrorAndMessage(rErrParamSize, "Wrong number of parameters");
  DefineErrorAndMessage(rDuplicatedParam, "Duplicated parameters");

  /* errorno for codegen -3001 ~ -4000 */
  DefineErrorAndMessage(rTestError, "test it is error ");

  /* errorno for logical_operator -4001~ -5000 */
  DefineErrorAndMessage(rUninitializedJoinPolicy,
                        "the dedicated join police is not initialized, e.g., "
                        "in corss join, equal join logical operator");
  DefineErrorAndMessage(rGeneratePlanContextFailed,
                        "generate the dataflow failed in the logical operator");

  /* errorno for physical query plan -5001 ~ -6000 */
  DefineErrorAndMessage(rGenerateSubPhyPlanFailed,
                        "generate the sub physical plan failed");
  DefineErrorAndMessage(rNoPartitionIdScan,
                        "The partition id does not existed.");
  DefineErrorAndMessage(rCodegenFailed, "codegen failed.");

  //  std::cout<<ERROR_MESSEGE[1]<<" , "<<ERROR_MESSEGE[2]<<std::endl;
}
}  // namespace common
}  // namespace claims
