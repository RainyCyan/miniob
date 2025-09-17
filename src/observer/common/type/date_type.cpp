/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "common/lang/comparator.h"
#include "common/log/log.h"
#include "common/type/date_type.h"
#include "common/value.h"
// #include "common/time/datetime.h"
#include <iomanip>

bool DateType::is_leap_year(const int &year) const { return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0); }
bool DateType::is_valid_date(const int &year, const int &month, const int &day) const
{
  // 年的范围
  if (year < 1 || year > 9999) {
    return false;
  }
  // 月份范围
  if (month < 1 || month > 12) {
    return false;
  }
  // 闰年二月天数29
  int daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (is_leap_year(year))
    daysInMonth[1] = 29;
  // 日期范围
  if (day < 1 || day > daysInMonth[month - 1]) {
    return false;
  }
  return true;
}

int DateType::compare(const Value &left, const Value &right) const
{
  // ASSERT(left.attr_type() == AttrType::DATES && right.attr_type() == AttrType::DATES, "invalid type");
  ASSERT(left.attr_type()==AttrType::DATES,"invalid type");
  ASSERT(right.attr_type()==AttrType::DATES||right.attr_type()==AttrType::CHARS,"invalid type");
  // dates compare chars
  if (right.attr_type() == AttrType::DATES) {
    return common::compare_int((void *)&left.value_.int_value_, (void *)&right.value_.int_value_);
  }
  if (right.attr_type() == AttrType::CHARS) {
    Value right_cast_val;
    cast_to(right, AttrType::DATES, right_cast_val);
    return common::compare_int((void *)&left.value_.int_value_, (void *)&right_cast_val.value_.int_value_);
  }

  return 0;
}
RC DateType::set_value_from_str(Value &val, const string &data) const
{
  int year, month, day;
  // parse by sscanf
  if (sscanf(data.c_str(), "%d-%d-%d", &year, &month, &day) != 3) {
    return RC::INVALID_DATE_FORMAT;
  }
  // check data
  if (!this->is_valid_date(year, month, day)) {
    return RC::INVALID_DATE_FORMAT;
  }
  LOG_INFO("val.set_date,%d,%d,%d",year,month,day);
  val.set_date(year, month, day);
  return RC::SUCCESS;
}

RC DateType::cast_to(const Value &val, AttrType type, Value &result) const
{
  switch (type) {
    default: return RC::UNIMPLEMENTED;
  }
  return RC::SUCCESS;
}

int DateType::cast_cost(AttrType type)
{
  if (type == AttrType::DATES) {
    return 0;
  }
  return INT32_MAX;
}

RC DateType::to_string(const Value &val, string &result) const
{
  LOG_INFO("call to_string,%d",val.value_.int_value_);
  int          year  = val.value_.int_value_ / 10000;
  int          month = (val.value_.int_value_ % 10000) / 100;
  int          day   = val.value_.int_value_ % 100;
  stringstream ss;
  ss << std::setw(4) << std::setfill('0') << year << "-" << std::setw(2) << std::setfill('0') << month << "-"
     << std::setw(2) << std::setfill('0') << day;
  result = ss.str();
  // LOG_INFO("to_string,%s",result);
  return RC::SUCCESS;
}