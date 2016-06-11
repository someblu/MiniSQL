#ifndef __index_h__
#define __index_h__

#include "MiniSQL.h"
#include "buffer_manager.h"
#include <iostream>
#include <string>

using namespace std;

typedef struct struct_unit_node
{
	int block_number;//������¼�ǵڼ����
	int offset_in_block;//��1��ʼ��
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

protected:
	int m_blocknumber;  //���
	int m_count;        //�ڵ����ܼ�¼��
	CIndexManager *m_next;//ָ����һ���ڵ�
	unit_node *m_first;   //ָ���һ����¼
	char m_minkey[VALUE_LENGTH];//�ýڵ�����Сֵ
	int m_next_block_number; //��һ���ڵ�Ŀ��

	void freespace();
	int save_index();
	int insert_entry(char *value,int tag,int blocknumber,int offsetinblock,int total_idx_block);
	int quick_insert(char *value,int tag,int blocknumber,int offsetinblock);
	int build_index();
};

#endif