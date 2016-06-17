#ifndef __catalog_h__
#define __catalog_h__

#include "MiniSQL.h"

typedef struct struct_AttrNode* AttrPointer;
typedef struct struct_AttrNode
{
	char AttrName[NAMELEN];
	unsigned int length;			//属性长度
	unsigned int offset;			//表中偏移量
	unsigned int type;				//类型
	short int IsPrimarykey;
	short int IsUnique;
	AttrPointer Next;
}AttrNode;

typedef struct struct_TableNode* TablePointer;
typedef struct struct_TableNode
{
	char TableName[NAMELEN];
	AttrPointer AllAttrs;
	char primarykey[NAMELEN];		//表的主键
	unsigned int number;			//当前记录的个数
	TablePointer Next;
}TableNode;

typedef struct struct_IndexNode* IndexPointer;
typedef struct struct_IndexNode
{
	char IndexName[NAMELEN];
	char TableName[NAMELEN];
	char AttrName[NAMELEN];
	IndexPointer Next;
}IndexNode;

class CCatalogManager
{
public:
	CCatalogManager(){
		m_tablehead = NULL;
		m_tabletail = NULL;
		m_indexhead = NULL;
		m_indextail = NULL;
	}
	~CCatalogManager(){}

	void ReadCatalog();
	void UpdateCatalog();

	short int IsTableExists(string tablename);
	short int DeleteTableInfo(string tablename);
	short int IsIndexExists(string indexname);
	short int DeleteIndexInfo(string indexname);
	short int IsAttrExists(string tablename, string attrname);
	short int GetAttrTypeByName(string tablename, string attrname);
	short int GetAttrTypeByNum(string tablename, unsigned int attrnum);
	unsigned int GetAttrOffset(string tablename, string attrname);
	unsigned int GetAttrLength(string tablename, string attrname);
	unsigned int GetRecordLength(string tablename);
	short int IsIndexCreated(string tablename, string attrname);
	string GetIndexName(string tablename, string attrname);
	unsigned int GetAttrNum(string tablename);
	short int GetAttrInfo(string tablename, unsigned int attrnum, column *tempcol);
	short int CreateTableInfo(string tablename, column *cols);
	short int CreateIndexInfo(string indexname, string tablename, column *cols);
	short int IsAttrUnique(string tablename, string attrname);
	unsigned int GetRecordNumber(string tablename);
	void RecordNumDel(string tablename, unsigned int deleted);
	void RecordNumAdd(string tablename, unsigned int added);

protected:
	TablePointer m_tablehead;				//表的链表的头部指针
	TablePointer m_tabletail;				//表的链表的尾部指针
	IndexPointer m_indexhead;				//索引的链表的头部指针
	IndexPointer m_indextail;				//索引的链表的尾部指针

};

//CCatalogManager Catalog;

#endif
