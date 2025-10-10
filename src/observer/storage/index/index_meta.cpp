/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai.wyl on 2021/5/18.
//

#include "storage/index/index_meta.h"
#include "common/lang/string.h"
#include "common/log/log.h"
#include "storage/field/field_meta.h"
#include "storage/table/table_meta.h"
#include "json/json.h"

const static Json::StaticString FIELD_NAME("name");
const static Json::StaticString FIELD_FIELD_NAME("field_name");

RC IndexMeta::init(const char *name, const vector<FieldMeta> &fields)
{
  if (common::is_blank(name)) {
    LOG_ERROR("Failed to init index, name is empty.");
    return RC::INVALID_ARGUMENT;
  }

  name_ = name;
  // field_ = field.name();
  // collect fields' name
  vector<string> fields_name;
  for (auto &field : fields) {
    fields_name.emplace_back(field.name());
  }

  field_.swap(fields_name);
  for (int i = 0; i < fields_.size(); i++) {
    fields_str_ += fields[i];
    if (i != fields_.size()) {
      fields_str_ += ",";
    }
  }

  return RC::SUCCESS;
}

void IndexMeta::to_json(Json::Value &json_value) const
{
  json_value[FIELD_NAME]       = name_;
  json_value[FIELD_FIELD_NAME] = fields_str_.c_str();
}

RC IndexMeta::from_json(const TableMeta &table, const Json::Value &json_value, IndexMeta &index)
{
  const Json::Value &name_value   = json_value[FIELD_NAME];
  const Json::Value &fields_value = json_value[FIELD_FIELD_NAME];
  if (!name_value.isString()) {
    LOG_ERROR("Index name is not a string. json value=%s", name_value.toStyledString().c_str());
    return RC::INTERNAL;
  }

  if (!fields_value.isString()) {
    LOG_ERROR("Fields name of index [%s] is not a string. json value=%s",
        name_value.asCString(), fields_value.toStyledString().c_str());
    return RC::INTERNAL;
  }
  // slice
  // vector<string>    fields_name;
  vector<FieldMeta> fields;
  auto              fields_str = field_value.asCstring();
  stringstream      ss(fields_str);
  string            field_name;
  while (getline(ss, field_name, ',')) {
    const FieldMeta *field = table.field(field_name.c_str());
    if (nullptr == field) {
      LOG_ERROR("Deserialize index [%s]: no such field: %s", name_value.asCString(), field_value.asCString());
      return RC::SCHEMA_FIELD_MISSING;
    }
    fields.emplace_back(*field);
  }

  return index.init(name_value.asCString(), fields);
}

const char *IndexMeta::name() const { return name_.c_str(); }

vector<string> IndexMeta::fields() { return fields_; }
const char          *IndexMeta::fields_str() const { return fields_str_.c_str(); }

void IndexMeta::desc(ostream &os) const { os << "index name=" << name_ << ", fields=" << fields_str_.c_str(); }