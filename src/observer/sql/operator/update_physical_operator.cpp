#include "update_physical_operator.h"
#include "storage/table/table.h"
#include "storage/trx/trx.h"
#include "storage/record/record.h"
#include "common/log/log.h"

RC UpdatePhysicalOperator::open(Trx *trx)
{
  if (children_.empty()) {
    return RC::SUCCESS;
  }

  // update语句和delete语句一定会带有一个过滤节点。如果where字段为空，则过滤语句也为空，此时将不对记录进行过滤，返回表中全部的记录
  unique_ptr<PhysicalOperator> &child = children_[0];

  RC rc = child->open(trx);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to open child operator: %s", strrc(rc));
    return rc;
  }

  trx_ = trx;

  while (OB_SUCC(rc = child->next())) {
    Tuple *tuple = child->current_tuple();
    if (nullptr == tuple) {
      LOG_WARN("failed to get current record: %s", strrc(rc));
      return rc;
    }

    RowTuple *row_tuple = static_cast<RowTuple *>(tuple);
    Record   &record    = row_tuple->record();
    records_.emplace_back(std::move(record));
  }

  child->close();
  const FieldMeta *feild_meta = table_->table_meta().field(field_name_);
  if (feild_meta == nullptr) {
    rc = RC::EMPTY;
    return rc;
  }

  int field_offset = feild_meta->offset();
  // 先收集记录再进行更新
  // 记录的有效性由事务来保证，如果事务不保证删除的有效性，那说明此事务类型不支持并发控制，比如VacuousTrx
  for (Record &record : records_) {
    // 根据旧的record创建新的record
    Record new_record;
    rc = table_->make_updated_record(record, new_record, values_[0],field_offset);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to make record from record: %s", strrc(rc));
      return rc;
    }

    rc = trx_->update_record(table_, record, new_record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to update record: %s", strrc(rc));
      return rc;
    }
  }

  return RC::SUCCESS;
}

RC UpdatePhysicalOperator::next()
{
  // Update 操作在 open 中一次性完成所有更新，next 总是返回 RECORD_EOF
  return RC::RECORD_EOF;
}

RC UpdatePhysicalOperator::close() { return RC::SUCCESS; }
