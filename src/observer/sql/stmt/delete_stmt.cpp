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

#include "sql/stmt/delete_stmt.h"
#include "common/log/log.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "sql/parser/expression_binder.h"

DeleteStmt::DeleteStmt(Table *table, vector<unique_ptr<Expression>> &filter_expressions) : table_(table), filter_expressions_(move(filter_expressions)) {}

DeleteStmt::~DeleteStmt()=default;


RC DeleteStmt::create(Db *db, DeleteSqlNode &delete_sql, Stmt *&stmt)
{
  const char *table_name = delete_sql.relation_name.c_str();
  if (nullptr == db || nullptr == table_name) {
    LOG_WARN("invalid argument. db=%p, table_name=%p", db, table_name);
    return RC::INVALID_ARGUMENT;
  }

  // check whether the table exists
  Table *table = db->find_table(table_name);
  if (nullptr == table) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }
  // binder_context init,add table
  BinderContext binder_context;
  binder_context.add_table(table);

  ExpressionBinder expression_binder(binder_context);

  // filter_expr bind
  vector<unique_ptr<Expression>> filter_expressions;
  RC rc=expression_binder.bind_expression(delete_sql.condition, filter_expressions);
  if (OB_FAIL(rc)) {
    LOG_INFO("bind expression failed. rc=%s", strrc(rc));
    return rc;
  }
  
  stmt = new DeleteStmt(table, filter_expressions);
  return rc;
}
