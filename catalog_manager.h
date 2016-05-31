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

	short int IsTableExists(const char* tablename);
	short int DeleteTableInfo(const char* tablename);
	short int IsIndexExists(const char* indexname);
	short int DeleteIndexInfo(const char* indexname);
	short int IsAttrExists(const char* tablename, const char* attrname);
	short int GetAttrTypeByName(const char* tablename, const char* attrname);
	short int GetAttrTypeByNum(const char* tablename, unsigned int attrnum);
	unsigned int GetAttrOffset(const char* tablename, const char* attrname);
	unsigned int GetAttrLength(const char* tablename, const char* attrname);
	unsigned int GetRecordLength(const char* tablename);
	short int IsIndexCreated(const char* tablename, const char* attrname);
	const char* GetIndexName(const char* tablename, const char* attrname);
	unsigned int GetAttrNum(const char* tablename);
	short int GetAttrInfo(const char* tablename, unsigned int attrnum, column *tempcol);
	short int CreateTableInfo(const char* tablename, column *cols);
	short int CreateIndexInfo(const char* indexname, const char* tablename, column *cols);
	short int IsAttrUnique(const char* tablename, const char* attrname);
	unsigned int GetRecordNumber(const char* tablename);
	void RecordNumDel(const char* tablename, unsigned int deleted);
	void RecordNumAdd(const char* tablename, unsigned int added);

protected:
	TablePointer m_tablehead;				//表的链表的头部指针
	TablePointer m_tabletail;				//表的链表的尾部指针
	IndexPointer m_indexhead;				//索引的链表的头部指针
	IndexPointer m_indextail;				//索引的链表的尾部指针

};

//CCatalogManager Catalog;

#endif
