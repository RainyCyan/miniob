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
2) debug
	

2) run 'drop table t; result:{FAILURE}观察日志
 - parser success
 - resolve:没有创建stmt,todo,已经创建了scf_drop_table和scf_drop_index的case在stmt.cpp
	- 实例化drop_table_stmt.h/cpp及drop_index_stmt.h/cpp,TODO:
	拆解，drop table执行删除表及表上面的索引，先实现drop_index



