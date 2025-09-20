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
// Created by Wangyunlai on 2022/5/22.
//


#include "sql/stmt/update_stmt.h"
#include "common/log/log.h"
#include "storage/db/db.h"

class Db;

UpdateStmt::UpdateStmt(Table *table, Value *values, int value_amount,FilterStmt* filter_stmt,std::string attr_name)
    : table_(table), values_(values), value_amount_(value_amount),filter_stmt_(filter_stmt),attr_name_(attr_name)
{}

RC UpdateStmt::create(Db *db, const UpdateSqlNode &update, Stmt *&stmt)
{
  // TODO
  // check table,attr, null
  const char *table_name = update.relation_name.c_str();
  std::string attr_name  = update.attribute_name;
  if (nullptr == db || nullptr == table_name || attr_name.empty()) {
    LOG_WARN("invalid argument. db=%p, table_name=%p , attr_name=%s",
        db, table_name,attr_name.c_str());
    return RC::INVALID_ARGUMENT;
  }
  
  //check whether the table exists
  Table *table = db->find_table(table_name);
  if (nullptr == table) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }
  //check whether the attr exists
  const FieldMeta* field = table->table_meta().field(update.attribute_name.c_str());
  if (nullptr == field) {
    LOG_WARN("no such field in table: table %s, field %s", table->name(), update.attribute_name.c_str());
    table = nullptr;
    return RC::SCHEMA_FIELD_NOT_EXIST;
  }

  // create filter stmt
  std::unordered_map<std::string, Table *> table_map;
  table_map.insert(std::pair<std::string, Table *>(std::string(table_name), table));

  FilterStmt *filter_stmt = nullptr;
  RC          rc          = FilterStmt::create(
      db, table, &table_map, update.conditions.data(), static_cast<int>(update.conditions.size()), filter_stmt);
  //error check
  if(rc!=RC::SUCCESS){
    LOG_WARN("failed to create filter statement. rc=%d:%s", rc, strrc(rc));
    return rc;
  }
  // 目前只支持value为单个的情况
  stmt = new UpdateStmt(table, (Value *)&update.value, 1, filter_stmt,attr_name);
  return RC::SUCCESS;
}
