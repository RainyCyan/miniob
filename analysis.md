# 题目分析文档
## miniob框架解析

### sql层

#### parser

#### 

### storage层


## Date

## drop table

- 实现删除表(drop table)，清除表相关的资源。
- 当前MiniOB支持建表与创建索引，但是没有删除表的功能。
- 在实现此功能时，除了要删除所有与表关联的数据，不仅包括磁盘中的文件，还包括内存中的索引等数据。
- 删除表的语句为 `drop table table-name`

**分析**：


**步骤**:

1) run 'drop index id1 on test;' result:{FAILURE}
	- ...
	- create index 跳过了optimize阶段，drop index也跳过，直接转到execute阶段
	- 增加drop_index case在command_executor.cpp
	- 实现drop_index_executor done
 **istorage:table->drop_index todo**:
	- table,table_engine add interface and base method
	- heap_table_engine:find_index(index_name),call bplustreeindex->drop()
	- bplustreeindex:继承自Index基类（增加virtual drop method);自己的drop函数调用bplustree_handler,note,删除之前需要先关闭handler;
	- bplustree_handler:在buffer_pool_manager中删除索引文件delete_file
	- 实现用remove,但是是否要加锁呢？？？？？？？？
	- note:在删除index时，需要同步删除掉indexes_中的元素，磁盘文件中的index_meta,table_meta中的index_meta
2) debug
done	

2) run 'drop table t; result:{FAILURE}观察日志
 - parser success
 - resolve:没有创建stmt,todo,已经创建了scf_drop_table和scf_drop_index的case在stmt.cpp
	- 实例化drop_table_stmt.h/cpp及drop_index_stmt.h/cpp,TODO:
	拆解，drop table执行删除表及表上面的索引，先实现drop_index











## update
- 实现更新行数据的功能。
- 当前实现update单个字段即可。现在MiniOB具有insert和delete功能，在此基础上实现更新功能。可以参考insert_record和delete_record的实现。目前仅能支持单字段update的语法解析，但是不能执行。需要考虑带条件查询的更新，和不带条件的更新，同时需要考虑带索引时的更新。

**步骤**

1) run `update t set a=1`,
	- resolver:todo,完成create func,update_sql_node有relation,attribute_name,value,conditions
	- optimizer:logical_plan_generator->create_plan todo
	构造谓词逻辑算子，构造更新逻辑算子
	- 更新逻辑算子实现，update_logical_operator.h/cpp TODO

	- storage层:

## like
- like 操作符是数据库中字符串查找非常常用的功能能，用于在 WHERE 子句中搜索符合一定格式的字段。
- 当前MiniOB中已经有条件查询的能力，需要扩展like相关的语法解析、查询匹配功能。
- '%'用于匹配零个到多个任意字符（英文单引号“'”除外），\'\_\'用于匹配一个任意字符（英文单引号“'”除外）。
- 当前只需要考虑char类型的字段即可（当前miniob支持int, float和char类型字段）。

**步骤**:


