#ifndef __index_h__
#define __index_h__
#pragma once

#include "MiniSQL.h"
#include "buffer_manager.h"

using namespace std;

typedef struct struct_unit_node
{
	int block_number;//这条纪录是第几块的
	int offset_in_block;//从1开始算
	char key[VALUE_LENGTH];
	struct struct_unit_node *next;
}unit_node;

class CIndexManager : public CBufferManager
{
public:
	string m_tablename;
	string m_indexname;

	CIndexManager(){}
	CIndexManager(string indexname, string tablename){
		m_tablename = tablename;
		m_indexname = indexname;
	}
	~CIndexManager(){}

	int SelectIndex(condition *conds, column *cols, unsigned int recordlen);
	int DeleteIndex(condition *conds, unsigned int recordlen);
	int CreateIndex(column *cols, unsigned int recordlen);
	int InsertIndex(insertvalue *value, unsigned int recordnum, int recordlen);
	int keycompare(char *a,char *b, int tag);
	void clear_idxblk();
protected:
	int m_blocknumber;  //块号
	int m_count;        //节点中总记录数
	CIndexManager *m_next;//指向下一个节点
	unit_node *m_first;   //指向第一条记录
	char m_minkey[VALUE_LENGTH];//该节点中最小值
	int m_next_block_number; //下一个节点的块号

	void freespace();
	int save_index();
	int insert_entry(char *value,int tag,int blocknumber,int offsetinblock,int total_idx_block);
	int quick_insert(char *value,int tag,int blocknumber,int offsetinblock);
	int build_index();
};

#endif
