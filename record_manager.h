#ifndef __record_h__
#define __record_h__
#pragma once

#include "MiniSQL.h"
#include "buffer_manager.h"

using namespace std;

class CRecordManager : public CBufferManager
{
public:
	string m_tablename;

	CRecordManager(string tablename){
		m_tablename = tablename;
	}
	~CRecordManager(){}

	short int SelectRecord(condition *conds, column *cols, unsigned int recordlen);//,unsigned int recordnum);
	short int DeleteRecord(condition *conds, unsigned int recordlen);
	short int IsValueExists(column *cols, char *value, unsigned int recornum);
	unsigned int InsertValues(insertvalue *values, unsigned int recordlen);
	
protected:
	short int ValueCompare(char *value, unsigned int type, unsigned int cond, char *compvalue);	  
};

#endif
