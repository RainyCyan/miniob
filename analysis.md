# 题目分析文档
## miniob框架解析

### sql层

#### parser
在parser层可供修改的有三个文件：lex,yacc,parse_def,其中lex是词法解析器，如果有直接匹配规则如（数字）在lex实现；yacc是语法解析，按照语法规则解析语句 ；parse_def定义了语法节点sqlnode
#### resolver

#### optimizer

**logical_plan**

**physical_plan**

#### operator

#### excutor

### storage层
#### db

#### trx

#### table

#### field

#### record

#### index

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

like_op/not_like_op应当作为比较符号(如Equal)处理

- parser:add token in lex,add op in parse_def,add compop case
- 比较表达式在执行阶段call phy_op->>next :cmp_expr->get_value:compare_value,修改这里,增加like_match在value.h



## NULL basis

NULL 是数据库的一个基本功能。

表字段可以有NULL属性，表示此字段是否允许为NULL值。NULL在做数值运算、逻辑比较时，都有特殊的含义，同时在做聚合运算(count/avg等）都需要做不同的处理。使用NULL关键字，不区分大小写。比如 `create table t(id int null, name char not null);`，其中字段`id` 可以为NULL值，而 `name`字段不允许为NULL值。

注意：

   - NULL 与任何数值比较，结果都是false。

   - NULL 用例非常基础，它出现在许多其它用例中。

**步骤**

1) 查看test case:涉及create,insert语句（字段属性，插入值类型）,compare(比较，is/is not),聚合语句（TODO）

2) parser:yacc:
	- add null_option,需要给field增加null属性先给AttrInfosqlnode增加nullable;
	- add attr_def,valueadd value.attr_type:null
	- where condition, add compOP is/is not,
3) storage:table->create,table_meta.init()->fieldmeta->init 暂时将null_field做trx_field最后一项处理

4) make_record:null_bit_map处理

5) to_string:select * from t where;table_get_logical->predicator->project;physical table_scan_phy->...

6) where null,
	6.1)表达式null
	6.2)is/is not null,
7) update set null,修改make_updated_record函数，增加null_bit_map置位操作


## expression

**步骤**
- parser:插入负数无法识别,修改insert into expression_list,parse_def,修改resolver insert_stmt
- parser:replace condition using expression[change where]
- parser:replace select/update/delete
- parser:为了继承default_handler,storage/condition_filter.h的历史包袱，暂时保留condition sql node
- resolver:delete filter_stmt;
- filter_stmt refactor:只需要一个create函数,使用expression重构create函数
- 去除filter_stmt,直接使用expression代替，需要bind
	- 完成delete,update,select的替代,test resolver阶段

- optimizer/logical_plan:select_stmt,delete create_plan(filter_stmt),init predicatelogicaloperator using filter_expression; do update/delete as select; debug:

- bug:当在date属性列create index并执行equal compare时，无法返回数值.
	- debug:bplustreescanner->open(left_val,right_val),似乎miniob的indexscan对equal的处理那里有问题，pos:create_phy_plan(table_get_logic_oper)

## typecast
note:expr,null,aggregate都涉及到了typecast的相关实现
## aggregate
   实现聚合函数 max/min/count/avg/sum。

聚合函数会遍历所有相关的行数据做相关的统计，并输出结果。
可能出现如 `select id, count(age) from t;` 这样的聚合和单个字段混合的测试语句，返回FAILURE。
当前题目是简单题目，所以测试用例中不会包含一些比较复杂的处理，比如表达式。但是有些数据类型会有隐式转换，比如avg计算整数类型时，结果会是浮点数。
注意处理语义处理时的异常场景，比如:
查询不存在的字段；
查询空字段；

- parser不需要处理，目前aggregate_expression:ID LBRACE Expression RBRACE的解析可以使用
- aggregate在group_by_logical_oper被收集

- aggregate(expression_list)需要被parser识别，但是返回failure,修改parser,修改expression_binder


## groupby
分组功能也是数据库的基本功能之一，目的是为了方便用户查询数据结果，按照一定条件进行分组，方便分析数据。

按照一个或多个字段对查询结果分组，group by中的聚合函数不要求支持表达式。

需要支持having子句，因为聚合函数不能出现在where后面，所以增加having子句用于筛选分组后的数据。不过having只和聚合函数一起出现。

注意需要考虑分组字段为null的情况。 

**分析**

查看logical_plan,call create_group_by_plan:再group_byplan中，会执行绑定groupby expr,收集聚合表达式


所以需要增加having子句，having和where的功能是相通的，只不过执行在group by之后，所以可以有聚合的比较

having的聚合处理，将having_expression递归解析，有聚合就加入aggregator_expressions，pos:create_group_by_plan

解决group by为null的情况



## join
NNER JOIN功能，需要支持join多张表。
当前已经支持多表查询的功能，这里主要工作是语法扩展，并考虑数据量比较大时如何处理。
注意带有多条on条件的join操作。



## multi-index
多字段索引功能。即一个索引中同时关联了多个字段。比如 `CREATE INDEX i_1_12 ON multi_index(col1,col2);`。

此功能除了需要修改语法分析，还需要调整B+树相关的实现，帮助同学们增加B+树数据存储知识的理解。

**分析**
testcase:空表/非空表建立多列索引;插入;查询；删除；更新；删除表；错误（不存在的列）

sql层:

- 1)parser:修改create_index_stmt,不区分单列索引和多列索引,修改create_index_sqlnode,支持多个attr
- 2)stmt:修改单个field_meta_为多个(const vector<FieldMeta>)修改构造函数，并修改create,使用const vector约束力更强；
- 3)executor:call table->create_index,需要进storage修改

storage层
- 1)table->create_index,table_engine->create_index,主要修改heap_table_engine->create_index
	- 1.1 call index_meta.init,init只是初始化index_name和fields_name,注意，from_json也调用了init,需要修改json记录格式，多个列名拼接，使用,作为分隔符;
	- 需要修改indexfileheader和keycomparator,indexfileheader目前增加了attr_type[],attr_length[]

## vector_basic
向量数据库题目一：向量类型基础功能


实现向量类型：

1. 支持创建包含向量类型的表。
向量类型
语法：vector(size)，其中，size 表示向量的维度（必须指定）
最大支持维度为 16000（在基础功能中只需要支持最大 1000 维向量即可）
向量类型中的浮点数最多保留两位小数，并且去掉多余的0
向量类型中每个元素都是数值类型（包括int 和 float 类型）。
支持创建包含向量类型的表：

`CREATE TABLE items (id int, embedding vector(3));`


2. 支持插入向量类型的记录。注意：这里需要支持将字符串类型的值转换为向量类型存储）：

INSERT INTO items VALUES (1, '[1,2,3]');

3. io支持向量类型的算术运算（加法（+），减法（-），乘法（），比较运算）。
select embedding + '[1.5,2.3,3.3]', embedding - '[1,2,3]', '[1,2,3]' - embedding from items where embedding > '[0,0,0]';

其中，算术运算为逐个元素运算，如 [1,2,3] + [1,1,1] = [2,3,4], [1,2,3] - [1,1,1]=[0,1,2], [1,2] * [1,3] = [1,6]；比较运算为逐个元素的字典序比较。即两个向量比较时，从左到右逐个数值进行比较，如果某个位置的数值不同，则根据该位置的数值大小比较结果作为向量的比较结果，举例：[1,2,3]<[1,2,4], [2,1,2]>[1,2,2]

实现距离表达式计算：

l2_distance
语法：l2_distance(vector A, vector B)
计算公式：\([ D = \sqrt{\sum_{i=1}^{n} (A_{i} - B_{i})^2} ]\)

，cosine_distance，

语法：cosine_distance(vector A, vector B)
计算公式：\([ D = 1 - \frac{\mathbf{A} \cdot \mathbf{B}}{|\mathbf{A}| |\mathbf{B}|} = 1 - \frac{\sum_{i=1}^{n} A_i B_i}{\sqrt{\sum_{i=1}^{n} A_i^2} \sqrt{\sum_{i=1}^{n} B_i^2}} ]\)

inner_product。
语法：inner_product(vector A, vector B)
计算公式：\([ D = \mathbf{A} \cdot \mathbf{B} = a_1 b_1 + a_2 b_2 + ... + a_n b_n = \sum_{i=1}^{n} a_i b_i ]\)

距离表达式的计算精度为保留2位小数。

**分析**
1) vector type,语法vector(size),size代表维度，max_size=16000,基础只需要实现1000;


**todo**
parser already support vector type,actually we need to impl vector_type.h/cpp as date

vector_type:

2) vector的值传入形式可能为'[]',"[]",[]这三种模式，都需要解析出来，目前有两种解析思路:一种是在cast_to处理char->vector,另一种是在parser就解析出来
