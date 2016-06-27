#ifndef __interpret_h__
#define __interpret_h__

#include "catalog_manager.h"

using namespace std;

class CInterpret {
public:
	//������,���е���������Է�ӳ������һ������
	unsigned int m_operation;		//Ҫִ�еĲ�����������,�ú��ʾ
	string m_tabname;				//Ҫ�����ı����
	string m_indname;				//Ҫ������������
	string m_filename;				//Ҫ�������ļ���,execfile�õ�
	column *m_cols;					//Ҫ��������������
	condition *m_conds;				//Ҫ�Ƚϵ�where�־������
	insertvalue *m_values;			//Ҫ�����ֵ����

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
