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
// create by ywm,2025/9/18

#pragma once

#include "sql/stmt/stmt.h"

struct DropIndexSqlNode;
class Table;

/**
 * @brief 删除索引的语句
 * @ingroup Statement
 */
class DropIndexStmt : public Stmt
{
public:
  DropIndexStmt(Table *table, const string &index_name)
      : table_(table), index_name_(index_name)
  {}

  virtual ~DropIndexStmt() = default;

  StmtType type() const override { return StmtType::DROP_INDEX; }

  Table           *table() const { return table_; }
  const string    &index_name() const { return index_name_; }

public:
  static RC create(Db *db, const DropIndexSqlNode &drop_index, Stmt *&stmt);

private:
  Table           *table_      = nullptr;
  string           index_name_;
};
