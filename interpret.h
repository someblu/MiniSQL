#ifndef __interpret_h__
#define __interpret_h__

#include "catalog_manager.h"

using namespace std;

class CInterpret {
public:
	//解析树,所有的命令均可以反映到这样一棵树上
	unsigned int m_operation;		//要执行的操作或错误代码,用宏表示
	string m_tabname;				//要操作的表格名
	string m_indname;				//要操作的索引名
	string m_filename;				//要操作的文件名,execfile用到
	column *m_cols;					//要操作的属性链表
	condition *m_conds;				//要比较的where字句的链表
	insertvalue *m_values;			//要插入的值链表

	//static CCatalogManager Catalog;

	CInterpret(){
		m_operation = UNKNOWN;
		m_tabname = "";
		m_indname = "";
		m_filename = "";
		m_cols = NULL;
		m_conds = NULL;
		m_values = NULL;		
	}
	~CInterpret(){}

	void Parse(char* command);
	void ExecSelect();
	void ExecDelete();
	void ExecDropTable();
	void ExecDropIndex();
	void ExecCreateTable();
	void ExecCreateIndex();
	void ExecInsert();
	void ExecFile();
	void Execute();
	void initcol(column *p);
	void DumpTree();

protected:
	void initcond(condition *p);
	void initvalue(insertvalue *p);
	void MakeInitilate(){
		m_operation = UNKNOWN;
		m_tabname = "";
		m_indname = "";
		m_filename = "";
		m_cols = NULL;
		m_conds = NULL;
		m_values = NULL;	
	}

	bool GetWord(string& src, string& des);
	bool GetStr(string& src, string& des);

	short int IsInt(const char*);
	short int IsFloat(char *input);
};

#endif
