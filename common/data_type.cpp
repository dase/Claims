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
 * /CLAIMS/common/data_type.cpp
 *
 *  Created on: Nov 17, 2015
 *      Author: lizhifang
 *		   Email:
 *
 * Description:
 *
 */

#include "data_type.h"

#include <cctype>
#include <iterator>

using claims::common::rSuccess;
using claims::common::rTooSmallData;
using claims::common::rTooLargeData;
using claims::common::rTooLongData;
using claims::common::rInterruptedData;
using claims::common::rIncorrectData;
using claims::common::rInvalidNullData;
using claims::common::kErrorMessage;
/**
 * if a string to input is warning, we modify it to a right value
 *     and return it's warning-code
 * if a string to input is error, we don't modify it, but return a error-code
 *
 * if there is no error or warning exist , we return success-code
 * @param str
 * @return warning/error code
 */
RetCode OperateInt::CheckSet(string& str) const {
  RetCode ret = rSuccess;
  /*
   * Input a null value, and it is valid (warning)
   */
  if (str == "" && nullable)
    return rSuccess;
  /*
   * Input a null value, but it is inValid (error)
   */
  if (str == "" && !nullable) {
    ret = rInvalidNullData;
    ELOG(ret, str);
    return ret;
  }
  /*
   * The input's format is error for number type like a12, c21, etc. (error)
   */
  if (!isdigit(str[0]) && str[0] != '-') {
    ret = rIncorrectData;
    ELOG(ret, str);
    return ret;
  }
  /*
   * The input need to be interrupted like 12a34, the string will be cut off to
   *   12. (warning)
   */
  for (auto i = 1; i < str.length(); i++)
    if (!isdigit(str[i]) && str[i] != '.' && str[i] != '-') {
      ret = rInterruptedData;
      WLOG(ret, str);
      str = str.substr(0, i);
      break;
    }
  long value = atol(str.c_str());
  /*
   * The input is less than int_min, we set it to int_min
   */
  if (value < INT_MIN) {
    ret = rTooSmallData;
    WLOG(ret, str);
    str = kIntMin;
  } else if (value > INT_MAX || (value == INT_MAX && nullable)) {
    /*
     * if input is int_max, we set it to int_max - 1, because int_max is
     *    null-value actually.
     * if input is larger than int_max, we set it to int_max-1
     */
    ret = rTooLargeData;
    WLOG(ret, str);
    str = kIntMax_1;
  }
  return ret;
}

RetCode OperateFloat::CheckSet(string& str) const {
  RetCode ret = rSuccess;
  if (str == "" && nullable)
    return rSuccess;
  if (str == "" && !nullable) {
    ret = rInvalidNullData;
    ELOG(ret, str)
    return ret;
  }
  if (!isdigit(str[0]) && str[0] != '-' && str[0] != '.') {
    ret = rIncorrectData;
    ELOG(ret, str);
    return ret;
  }
  for (auto i = 0; i < str.length(); i++)
    if (!isdigit(str[i]) && str[i] != '.' && str[i] != '-') {
      ret = rInterruptedData;
      WLOG(ret, str);
      str = str.substr(0, i);
      break;
    }
  double value = atof(str.c_str());
  if (value < -FLT_MAX) {
    ret = rTooSmallData;
    WLOG(ret, str);
    str = kFloatMin;
  } else if (value > FLT_MAX || (value == FLT_MAX && nullable)) {
    ret = rTooLargeData;
    WLOG(ret, str);
    str = kFloatMax_1;
  }
  return ret;
}
RetCode OperateDouble::CheckSet(string& str) const {
  RetCode ret = rSuccess;
  if (str == "" && nullable)
    return rSuccess;
  if (str == "" && !nullable) {
    ret = rInvalidNullData;
    ELOG(ret, str);
    return ret;
  }
  if (str == kDoubleMax && !nullable)
    return rSuccess;
  if (!isdigit(str[0]) && str[0] != '-') {
    ret = rIncorrectData;
    ELOG(ret, str);
    return ret;
  }
  for (auto i = 0; i < str.length(); i++)
    if (!isdigit(str[i]) && str[i] != '.' && str[i] != '-') {
      ret = rInterruptedData;
      WLOG(ret, str);
      str = str.substr(0, i);
      break;
    }
  /*
   *  integer part of a double number
   */
  auto len = 0;
  for (auto i = str.begin(); i != str.end(); i++, len++)
    if (*i == '.')
      break;
  /*
   * ToDo we could improve the size check for double type *_*
   */
  if (len >= 309) {
    if (str[0] == '-') {
      ret = rTooSmallData;
      WLOG(ret, str);
      str = kDoubleMin;
    } else {
      ret = rTooLargeData;
      WLOG(ret, str);
      str = kDoubleMax_1;
    }
  }
  return ret;
}

RetCode OperateULong::CheckSet(string& str) const {
  RetCode ret = rSuccess;
  if (str == "" && nullable)
    return rSuccess;
  if (str == "" && !nullable) {
    ret = rInvalidNullData;
    ELOG(ret, str);
    return ret;
  }
  if (str == kULongMax && !nullable)
    return rSuccess;
  if (!isdigit(str[0]) && str[0] != '-') {
    ret = rIncorrectData;
    ELOG(ret, str);
    return ret;
  }
  for (auto i = 0; i < str.length(); i++)
    if (!isdigit(str[i]) && str[i] != '.' && str[i] != '-') {
      ret = rInterruptedData;
      WLOG(ret, str);
      str = str.substr(0, i);
      break;
    }
  if (str[0] == '-') {
    ret = rTooSmallData;
    WLOG(ret, str);
    str = '0';
  } else {
    auto len = 0;
    for (auto i = str.begin(); i != str.end(); i++, len++)
      if (*i == '.')
        break;
    /*
     * ToDo we could improve the size check for unsigned long type *_*
     */
    if (len >= 20 && str[19] > '1') {
      ret = rTooLargeData;
      WLOG(ret, str)
      str = kULongMax_1;
    }
  }
  return ret;
}

RetCode OperateString::CheckSet(string& str) const {
  RetCode ret = rSuccess;
  if (str[0] == NULL_STRING && nullable)
    return rSuccess;
  if (str[0] == NULL_STRING && !nullable) {
    ret = rInvalidNullData;
    ELOG(ret, str);
    return ret;
  }
  /*
   * The input string is too long to storage in a tuple
   */
  if (str.length() > size) {
    ret = rTooLongData;
    WLOG(ret,
         " length of str:" << str << " is longer than expected length:" << size);
    str = str.substr(0, size);
  }
  return ret;
}

RetCode OperateDate::CheckSet(string& str) const {
  RetCode ret = rSuccess;
  if (str == "" && nullable)
    return rSuccess;
  if (str == "" && !nullable) {
    LOG(ERROR)<< "[CheckSet]: [" << kErrorMessage[rInvalidNullData] << "] for "
    << str << endl;
    return rInvalidNullData;
  }
  if (str.length() == 8) {
    for (auto i = str.begin(); i != str.end(); i++)
      if (!isdigit(*i)) {
        ret = rIncorrectData;
        ELOG(ret, str);
        return ret;
      }
    if (ret == rSuccess) {
      /*
       * Claims don't support date before 1400-01-01, but why?
       */
      if (str < "14000101") {
        ret = rTooSmallData;
        WLOG(ret, str);
        str = "14000101";
      }
      if (str > "99991231") {
        ret = rTooLargeData;
        WLOG(ret, str);
        str = "99991231";
      }
    }
  } else if (str.length() == 10) {
    for (auto i = 0; i < str.length(); i++)
      if (!isdigit(str[i]) && i != 4 && i != 7) {
        ret = rIncorrectData;
        WLOG(ret, str);
        return ret;
      }
    if (str < "1400-01-01") {
      ret = rTooSmallData;
      WLOG(ret, str);
      str = "1400-01-01";
    }
    if (str > "9999-12-31") {
      ret = rTooLargeData;
      WLOG(ret, str);
      str = "9999-12-31";
    }
  } else {
    ret = rIncorrectData;
    ELOG(ret, str);
    return ret;
  }
  return ret;
}

RetCode OperateTime::CheckSet(string& str) const {
  RetCode ret = rSuccess;
  if (str == "" && nullable)
    return rSuccess;
  if (str == "" && !nullable) {
    ret = rInvalidNullData;
    ELOG(ret, str);
    return ret;
  }
  if (str.length() != 15 && str.length() != 8) {
    ret = rIncorrectData;
    ELOG(ret, str);
    return ret;
  }
  if (str.length() == 15) {
    for (auto i = 0; i < str.length(); i++)
      if (!isdigit(str[i]) && i != 2 && i != 5 && i != 8) {
        ret = rIncorrectData;
        ELOG(ret, str);
        return ret;
      }
    if (str < "00:00:00.000000") {
      ret = rTooSmallData;
      ELOG(ret, str);
      str = "00:00:00.000000";
    } else if (str > "23:59:59.999999") {
      ret = rTooLargeData;
      ELOG(ret, str)
      str = "23:59:59.999999";
    }
  }
  if (str.length() == 8) {
    for (auto i = 0; i < str.length(); i++)
      if (!isdigit(str[i]) && i != 2 && i != 5) {
        ret = rIncorrectData;
        ELOG(ret, str);
        return ret;
      }
    if (str < "00:00:00") {
      ret = rTooSmallData;
      ELOG(ret, str);
      str = "00:00:00";
    } else if (str > "23:59:59") {
      ret = rTooLargeData;
      ELOG(ret, str)
      str = "23:59:59";
    }
  }

  return ret;
}

RetCode OperateDatetime::CheckSet(string& str) const {
  RetCode ret = rSuccess;
  bool deci_sec_flag = 0;
  if (str == "" && nullable)
    return rSuccess;
  if (str == "" && !nullable) {
    ret = rInvalidNullData;
    ELOG(ret, str);
    return ret;
  }
  std::string def_datetime[7] =
      { "1400", "01", "01", "00", "00", "00", "000000" };
  char separator[6] = { '-', '-', ' ', ':', ':', '.' };
  str = trim(str);

  /*
   *  check only one or no space in str.
   */
  int space_pos = str.find(' ');
  if (space_pos != -1) {
    std::string str_res = str.substr(space_pos + 1);
    if ((space_pos = str_res.find(' ')) != -1) {
      ret = rIncorrectData;
      ELOG(ret, str);
      return ret;
    }
  }
  /*
   *  check str ,if there is warning fix it, if there is error,
   */
  int i = 0;
  while (i < 6) {
    int separator_pos = str.find(separator[i]);
    //    can't find ith separator
    if (separator_pos == -1) {
      if (is_all_digit(str)) {
        ret = rIncorrectData;
        ELOG(ret, str);
        return ret;
      }
      def_datetime[i] = str;
      str = check_datetime_range(def_datetime, deci_sec_flag, ret);
      ELOG(ret, str);
      return ret;
    } else {
      //    find ith separator
      std::string left_part = str.substr(0, separator_pos);
      str = str.substr(separator_pos + 1, str.size());
      if (is_all_digit(left_part)) {
        ret = rIncorrectData;
        ELOG(ret, str);
        return ret;
      }
      def_datetime[i] = left_part;
    }
    i++;
  }
  //    the data behind "." part
  if (is_all_digit(str)) {
    ret = rIncorrectData;
    ELOG(ret, str);
    return ret;
  }
  if (str.size() == 6) {
    def_datetime[6] = str;
    deci_sec_flag = 1;
  }
  else if(str.size() != 0 && str.size() != 6)
  {
    ret = rIncorrectData;
    ELOG(ret, str);
    return ret;
  }
  str = check_datetime_range(def_datetime, deci_sec_flag, ret);
  ELOG(ret, str);
  return ret;
}

std::string& trim(std::string &s) {
  if (s.empty()) {
    return s;
  }
  s.erase(0, s.find_first_not_of(" "));
  s.erase(s.find_last_not_of(" ") + 1);
  return s;
}

bool is_all_digit(const std::string &str) {
  bool flag = 0;
  for (auto i = str.begin(); i != str.end(); i++) {
    if (isdigit(*i) == 0) {
      flag = 1;
    }
  }
  return flag;
}
/*
 * 可把"0123"->"123"
 */
void str_to_int(const std::string &str, int &res) {
  std::stringstream ss;
  ss << str;
  ss >> res;
  ss.clear();
}

void int_to_str(std::string &str, int &res) {
  std::stringstream ss;
  ss << res;
  ss >> str;
  ss.clear();
}
/*
 *  flag : 是否还有小数点后面的时间  =1 则为有
 *  return : YYYY-MM-DD HH-mm-ss or YYYY-MM-DD HH-mm-ss.xxxxxx
 */
std::string check_datetime_range(std::string datetime[],
                                 const bool &deci_sec_flag, RetCode &retcode) {
  int year, month, day, hour, minute, second, deci_sec;
  bool is_leap_year = 0;
  int days[2][12] = { { 31, 28, 31, 30, 31, 30, 31, 30, 31, 30, 31, 30 }, { 31,
      29, 31, 30, 31, 30, 31, 30, 31, 30, 31, 30 } };
  str_to_int(datetime[0], year);

  if (year > 9999 ||year < 1400) {
    retcode = rIncorrectData;
    return "";
  } else {
    int_to_str(datetime[0], year);
  }
  str_to_int(datetime[0], year);
  if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
    is_leap_year = 1;

  str_to_int(datetime[1], month);
  if (month > 12 || month < 1) {
    retcode = rIncorrectData;
    return "";
  }else {
    int_to_str(datetime[1], month);
    if (datetime[1].size() == 1)
      datetime[1] = "0" + datetime[1];
  }

  str_to_int(datetime[2], day);
  if (day > days[is_leap_year][month - 1] || day == 0) {
    retcode = rIncorrectData;
    return "";
  } else {
    int_to_str(datetime[2], day);
    if (datetime[2].size() == 1) {
      datetime[2] = "0" + datetime[2];
    }
  }

  str_to_int(datetime[3], hour);
  if (hour > 24) {
    retcode = rIncorrectData;
    return "";
  } else {
    int_to_str(datetime[3], hour);
    if (datetime[3].size() == 1) {
      datetime[3] = "0" + datetime[3];
    }
  }

  str_to_int(datetime[4], minute);
  if (minute > 59) {
    retcode = rIncorrectData;
    return "";
  } else {
    int_to_str(datetime[4], minute);
    if (datetime[4].size() == 1) {
      datetime[4] = "0" + datetime[4];
    }
  }

  str_to_int(datetime[5], second);
  if (second > 59) {
    retcode = rIncorrectData;
    return "";
  } else {
    int_to_str(datetime[5], second);
    if (datetime[5].size() == 1) {
      datetime[5] = "0" + datetime[5];
    }
  }
  //    return datetime format like 1400-01-01 00:00:00.000000 or 9999-12-30 23:59:59
  if (deci_sec_flag == 0)
  {
    return datetime[0] + "-" + datetime[1] + "-" + datetime[2] + " "
        + datetime[3] + ":" + datetime[4] + ":" + datetime[5];
  }else{
    return datetime[0] + "-" + datetime[1] + "-" + datetime[2] + " "
        + datetime[3] + ":" + datetime[4] + ":" + datetime[5] + "."
        + datetime[6];
  }
}

RetCode OperateSmallInt::CheckSet(string& str) const {
  RetCode ret = rSuccess;
  if (str == "" && nullable)
    return rSuccess;
  if (str == "" && !nullable) {
    ret = rInvalidNullData;
    ELOG(ret, str);
    return ret;
  }
  if (!isdigit(str[0]) && str[0] != '-') {
    ret = rIncorrectData;
    ELOG(ret, str);
    return ret;
  }
  for (auto i = 0; i < str.length(); i++)
    if (!isdigit(str[i]) && str[i] != '.' && str[i] != '-') {
      ret = rInterruptedData;
      WLOG(ret, str);
      str = str.substr(0, i);
      break;
    }
  long value = atol(str.c_str());
  if (value < SHRT_MIN) {
    ret = rTooSmallData;
    WLOG(ret, str);
    str = kSmallIntMin;
  } else if (value > SHRT_MAX || (value == SHRT_MAX && nullable)) {
    ret = rTooLargeData;
    WLOG(ret, str);
    str = kSmallIntMax_1;
  }
  return ret;
}

RetCode OperateUSmallInt::CheckSet(string& str) const {
  RetCode ret = rSuccess;
  if (str == "" && nullable)
    return rSuccess;
  if (str == "" && !nullable) {
    ret = rInvalidNullData;
    ELOG(ret, str);
    return ret;
  }
  if (!isdigit(str[0]) && str[0] != '-') {
    ret = rIncorrectData;
    ELOG(ret, str);
    return ret;
  }
  for (auto i = 0; i < str.length(); i++)
    if (!isdigit(str[i]) && str[i] != '.' && str[i] != '-') {
      ret = rInterruptedData;
      WLOG(ret, str);
      str = str.substr(0, i);
      break;
    }
  long value = atol(str.c_str());
  if (value < 0) {
    ret = rTooSmallData;
    WLOG(ret, str);
    str = "0";
  } else if (value > USHRT_MAX || (value == USHRT_MAX && nullable)) {
    ret = rTooLargeData;
    WLOG(ret, str);
    str = kUSmallIntMax_1;
  }
  return ret;
}
/*
 * ToDo There is still some work for decimal type
 */
RetCode OperateDecimal::CheckSet(string& str) const {
  RetCode ret = rSuccess;
  if (str == "" && nullable)
    return rSuccess;
  if (str == "" && !nullable) {
    ret = rInvalidNullData;
    ELOG(ret, str);
    return ret;
  }
  return ret;
}

RetCode OperateBool::CheckSet(string& str) const {
  RetCode ret = rSuccess;
  if (str == "" && nullable)
    return rSuccess;
  if (str == "" && !nullable) {
    ret = rInvalidNullData;
    ELOG(ret, str);
    return ret;
  }
  if (str != "false" && str != "true") {
    ret = rIncorrectData;
    ELOG(ret, str);
    return ret;
  }
  return ret;
}
