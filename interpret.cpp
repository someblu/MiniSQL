//head file including
#include "MiniSQL.h"
//#include "stdafx.h"
//#include "index_manager.h"
#include "buffer_manager.h"
#include "record_manager.h"
#include "catalog_manager.h"
#include "interpret.h"

using namespace std;

//static CCatalogManager* m_pCatalog = new CCatalogManager;
extern CCatalogManager Catalog;

//初始化 column 结构
void CInterpret::initcol(column *p)
{
	strcpy(p -> colname, "");
	p -> IsPrimary = 0;
	p -> IsUnique = 0;
	p -> next = NULL;
	p -> type = 0;
	p -> coloffset = 0;
	p -> collength = 0;
}

//初始化 condition 结构
void CInterpret::initcond(condition *p)
{
	strcpy(p -> attrname, "");
	p -> cond = 0;
	p -> next = NULL;
	p -> attroffset = 0;
	p -> attrlength = 0;
	strcpy(p -> value, "");
	p -> type = 0;
}

//初始化 insertvalue 结构
void CInterpret::initvalue(insertvalue *p)
{
	p -> type = 0;
	strcpy(p->value, "");
	p -> next = NULL;
	p -> length = 0;
}

//从 src 和 des 中获取一个字
bool CInterpret::GetWord(string& src, string& des)
{
	unsigned int srcpos = 0, despos = 0;
	char temp = ' ';

	for(;srcpos<src.length();srcpos++) {
		if(temp == ' ' || temp == '\t' || temp == 10 || temp == 13)
			temp = src.at(srcpos);
		else break;
	}
	if(srcpos == src.length() && (temp == ' ' || temp == '\t' || temp == 10 || temp == 13))
		return false;
			
	switch(temp)
	{
	case ',':
	case '(':
	case ')':
	case '*':
	case '=':
	case '\'':
		des.replace(despos++, 1, 1, temp);
		des.replace(despos++, 1, 1, '\0');
		src.erase(0,srcpos);
		break;
	case '<':
		des.replace(despos++, 1, 1, temp);
		temp = src.at(srcpos++);
		if(temp == '=' || temp == '>')
		{
			des.replace(despos++, 1, 1, temp);
			des.replace(despos++, 1, 1, '\0');
			src.erase(0,srcpos);
		}
		else
		{
			des.replace(despos++, 1, 1, '\0');
			src.erase(0,srcpos-1);
		}
		break;
	case '>':
		des.replace(despos++, 1, 1, temp);
		temp = src.at(srcpos++);
		if(temp == '=')
		{
			des.replace(despos++, 1, 1, temp);
			des.replace(despos++, 1, 1, '\0');
			src.erase(0,srcpos);
		}
		else
		{
			des.replace(despos++, 1, 1, '\0');
			src.erase(0,srcpos-1);
		}
		break;
	default:
		do{
			des.replace(despos++, 1, 1, temp);
			if (srcpos < src.length())
				temp = src.at(srcpos++);
			else  {
				src.erase(0,srcpos);
				des.replace(despos++, 1, 1, '\0');
				return true;
			}
		}while(temp != '*' && temp != ',' && temp != '(' && temp != ')'
			&& temp != ' ' && temp != '\t' && temp != '=' && temp != '>' 
			&& temp != '<' && temp != '\'' && temp != 10 && temp != 13 );
		src.erase(0,srcpos-1);
		des.replace(despos++, 1, 1, '\0');
	}
	return true;
}

bool CInterpret::GetStr(string& src, string& des)
{
	unsigned int pos=0;
	char temp;
	if (src.at(0) == '\'') {
		des = "";
		return true;
	}
	else for(;pos<src.length();pos++) {
		if((temp = src.at(pos)) != '\'')
			des.replace(pos, 1, 1, temp);
		else {
			des.replace(pos, 1, 1, '\0');
			src.erase(0,pos);
			return true;
		} 
	}
	return false;
}

//分析命令，并返回解析树 
void CInterpret::Parse(char* command) 
{
	char tempbuf[COMLEN];
	string word = tempbuf;
	string temp = command;
	short int FindPriKey = 0, FindUnique = 0;
	column *lastcol = NULL;
	column *tempcol = NULL;
	condition *lastcon = NULL;
	condition *tempcon = NULL;
	insertvalue *tempval = NULL;
	insertvalue *lastval = NULL;
	
	MakeInitilate();
	
	bool flag;
	flag = GetWord(temp, word);
	//若是空查询
	if(!flag)
	{
		m_operation = EMPTY;
		return;
	}

	//若是退出命令
	else if(word == "quit")
	{
		flag = GetWord(temp, word);
		if(!flag)
			m_operation = QUIT;
		return;
	}

	//若是select命令
	else if(word == "select")
	{
		m_operation = SELERR;
		
		//解析select子句
		flag = GetWord(temp, word);
		if(!flag)
			return;
		tempcol = new column;
		initcol(tempcol);
		strcpy(tempcol -> colname, word.c_str());
		m_cols = lastcol = tempcol;
		flag = GetWord(temp, word);
		if(!flag)
			return;
		
		while(word == ",")
		{
			flag = flag = GetWord(temp, word);
			if(!flag)
				return;
			tempcol = new column;
			initcol(tempcol);
			strcpy(tempcol -> colname, word.c_str());
			lastcol -> next = tempcol;
			lastcol = tempcol;
			flag = flag = GetWord(temp, word);
			if(!flag)
				return;
		}
	
		if(word != "from")
			return;
		//解析from子句
		flag = GetWord(temp, word);
		if(!flag)
			return;
		m_tabname = word.c_str();

		//如果没有where子句，正确返回
		flag = GetWord(temp, word);
		if(!flag)
		{
			m_operation = SELECT;
			return;
		}
		if(word != "where")
			return;
		//解析where子句
		flag = GetWord(temp, word);
		if(!flag)
		{
			m_operation = SELECT;
		}
		tempcon = new condition;
		initcond(tempcon);
		strcpy(tempcon -> attrname, word.c_str());
		flag = GetWord(temp, word);
		if(!flag)
			return;
		if(word == "<")
			tempcon -> cond = LT;
		else if(word == "<=")
			tempcon -> cond = LE;
		else if(word == ">")
			tempcon -> cond = GT;
		else if(word == ">=")
			tempcon -> cond = GE;
		else if(word == "=")
			tempcon -> cond = EQ;
		else if(word == "<>")
			tempcon -> cond = NE;
		else return;
			
		flag = GetWord(temp, word);
		if(!flag)
			return;
		if(word == "\'")
		{
			tempcon -> type = CHAR;
			flag = GetStr(temp, word);
			if(!flag)
				return;
			strcpy(tempcon -> value, word.c_str());
			flag = GetWord(temp, word);
			if(!flag)
				return;
			if(word != "\'")
				return;
		}
		else
		{
			strcpy(tempcon -> value, word.c_str());
			tempcon -> type = NOTCHAR;
		}
		m_conds = lastcon = tempcon;
		flag = GetWord(temp, word);
		if(!flag)
		{
			m_operation = SELECT;
			return;
		}
		while(word == "and" || word == "or")
		{
			flag = GetWord(temp, word);
			if(!flag)
				return;
			tempcon = new condition;
			initcond(tempcon);
			strcpy(tempcon -> attrname, word.c_str());
			flag = GetWord(temp, word);
			if(!flag)
				return;
			if(word == "<")
				tempcon -> cond = LT;
			else if(word == "<=")
				tempcon -> cond = LE;
			else if(word == ">")
				tempcon -> cond = GT;
			else if(word == ">=")
				tempcon -> cond = GE;
			else if(word == "=")
				tempcon -> cond = EQ;
			else if(word == "<>")
				tempcon -> cond = NE;
			else return;
			flag = GetWord(temp, word);
			if(!flag)
				return;
			if(word == "\'")
			{
				tempcon -> type = CHAR;
				flag = GetStr(temp, word);
				if(!flag)
					return;
				strcpy(tempcon -> value, word.c_str());
				flag = GetWord(temp, word);
				if(!flag)
					return;
				if(word != "\'")
					return;
			}
			else
			{
				strcpy(tempcon -> value, word.c_str());
				tempcon -> type = NOTCHAR;
			}
			lastcon -> next = tempcon;
			lastcon = tempcon;
			flag = GetWord(temp, word);
			if(!flag)
			{
				m_operation = SELECT;
				return;
			}
		}
	}
	//若是create table/create index命令
	else if(word == "create")
	{
		flag = GetWord(temp, word);
		if(!flag)
			return;
		//解析create table命令
		if(word == "table")
		{
			m_operation = CRETABERR;
			flag = GetWord(temp, word);
			if(!flag)
				return;
			m_tabname  = word.c_str();
			flag = GetWord(temp, word);
			if(!flag)
				return;
			if(word != "(")
				return;
			flag = GetWord(temp, word);
			if(!flag)
				return;
			if(word == "unique" || word == "primary")
				return;
			tempcol = new column;
			initcol(tempcol);
			strcpy(tempcol -> colname, word.c_str());
			m_cols = lastcol = tempcol;
			flag = GetWord(temp, word);
			if(!flag)
				return;
			if(word == "int")
			{
				tempcol -> type = INT;
				tempcol -> collength = INTLEN;
				flag = GetWord(temp, word);
				if(!flag)
					return;
				if(word == "unique")
				{
					tempcol-> IsUnique = 1;
					flag = GetWord(temp, word);
					if(!flag)
						return;
				}
			}
			else if(word == "float")
			{
				tempcol -> type = FLOAT;
				tempcol -> collength = FLOATLEN;
				flag = GetWord(temp, word);
				if(!flag)
					return;
				if(word == "unique")
				{
					tempcol-> IsUnique = 1;
					flag = GetWord(temp, word);
					if(!flag)
						return;
				}
			}
			else if(word == "char")
			{
				tempcol -> type = CHAR;
				flag = GetWord(temp, word);
				if(!flag)
					return;
				if(word != "(")
					return;
				flag = GetWord(temp, word);
				if(!flag)
					return;
				if(!IsInt((word.c_str())))
					return;
				tempcol -> collength = atoi(word.c_str()) + 1;
				if(tempcol -> collength > 256 || tempcol -> collength < 2)
				{
					m_operation = CHARBOUD;
					return;
				}
				flag = GetWord(temp, word);
				if(!flag)
					return;
				if(word != ")")
					return;
				flag = GetWord(temp, word);
				if(!flag)
					return;
				if(word == "unique")
				{
					tempcol-> IsUnique = 1;
					flag = GetWord(temp, word);
					if(!flag)
						return;
				}
			}
			else return;
			while(word == ",")
			{
				flag = GetWord(temp, word);
				if(!flag)
					return;
				//如果是primary key命令
				if(word == "primary")
				{
					flag = GetWord(temp, word);
					if(!flag)
						return;
					if(word != "key")
						return;
					flag = GetWord(temp, word);
					if(!flag)
						return;
					if(word != "(")
						return;
					flag = GetWord(temp, word);
					if(!flag)
						return;
					tempcol = m_cols;
					while(tempcol)
					{
						if(!(word.compare(tempcol -> colname)))
						{
							FindPriKey = 1;
							tempcol -> IsPrimary = 1;
							tempcol -> IsUnique = 1;
						}
						tempcol = tempcol -> next;
					}
					if(!FindPriKey)
					{
						m_operation = VOIDPRI;
						return;
					}
					FindPriKey = 0;
					flag = GetWord(temp, word);
					if(!flag)
						return;
					if(word != ")")
						return;
					flag = GetWord(temp, word);
					if(!flag)
						return;
				}
				//如果是unique命令
				else if(word == "unique")
				{
					flag = GetWord(temp, word);
					if(!flag)
						return;
					if(word != "(")
						return;
					flag = GetWord(temp, word);
					if(!flag)
						return;
					tempcol = m_cols;
					while(tempcol)
					{
						if(!(word.compare(tempcol -> colname)))
						{
							FindUnique = 1;
							tempcol -> IsUnique = 1;
						}
						tempcol = tempcol -> next;
					}
					if(!FindUnique)
					{
						m_operation = VOIDUNI;
						return;
					}
					FindUnique = 0;
					flag = GetWord(temp, word);
					if(!flag)
						return;
					if(word != ")")
						return;
					flag = GetWord(temp, word);
					if(!flag)
						return;
				}
				//如果不是unique与primary key命令
				else
				{
					tempcol = new column;
					initcol(tempcol);
					strcpy(tempcol -> colname, word.c_str());
					flag = GetWord(temp, word);
					if(!flag)
						return;
					if(word == "int")
					{
						tempcol -> type = INT;
						tempcol -> collength = INTLEN;
						flag = GetWord(temp, word);
						if(!flag)
							return;
						if(word == "unique")
						{	
							tempcol-> IsUnique = 1;
							flag = GetWord(temp, word);
							if(!flag)
								return;
						}
					}
					else if(word == "float")
					{
						tempcol -> type = FLOAT;
						tempcol -> collength = FLOATLEN;
						flag = GetWord(temp, word);
						if(!flag)
							return;
						if(word == "unique")
						{
							tempcol-> IsUnique = 1;
							flag = GetWord(temp, word);
							if(!flag)
							return;
						}
					}
					else if(word == "char")
					{
						tempcol -> type = CHAR;
						flag = GetWord(temp, word);
						if(!flag)
							return;
						if(word != "(")
							return;
						flag = GetWord(temp, word);
						if(!flag)
							return;
						if(!IsInt(word.c_str()))
							return;
						tempcol -> collength = atoi(word.c_str())+1;
						if(tempcol -> collength > 255 || tempcol -> collength < 1)
						{
							m_operation = CHARBOUD;
							return;
						}
						flag = GetWord(temp, word);
						if(!flag)
							return;
						if(word != ")")
							return;
						flag = GetWord(temp, word);
						if(!flag)
							return;
						if(word == "unique")
						{
							tempcol-> IsUnique = 1;
							flag = GetWord(temp, word);
							if(!flag)
								return;
						}
					}
					else return;
					lastcol -> next = tempcol;
					lastcol = tempcol;
				}
			}
			if(word != ")")
				return;
			flag = GetWord(temp, word);
			if(!flag)
				m_operation = CRETAB;
		}
		//解析create index命令
		else if(word == "index")
		{
			m_operation = CREINDERR;
			flag = GetWord(temp, word);
			if(!flag)
				return;
			m_indname = word.c_str();
			flag = GetWord(temp, word);
			if(!flag)
				return;
			if(word != "on")
				return;
			flag = GetWord(temp, word);
			if(!flag)
				return;
			m_tabname  = word.c_str();
			flag = GetWord(temp, word);
			if(!flag)
				return;
			if(word != "(")
				return;
			flag = GetWord(temp, word);
			if(!flag)
				return;
			tempcol = new column;
			initcol(tempcol);
			strcpy(tempcol -> colname, word.c_str());
			m_cols = tempcol;
			flag = GetWord(temp, word);
			if(!flag)
				return;
			if(word != ")")
				return;
			flag = GetWord(temp, word);
			if(!flag)
				m_operation = CREIND;
		}
	}
	//若是delete from命令
	else if(word == "delete")
	{
		flag = GetWord(temp, word);
		if(!flag)
			return;
		if(word == "from")
		{
			m_operation = DELETEERR;
			flag = GetWord(temp, word);
			if(!flag)
				return;
			m_tabname  = word.c_str();
			flag = GetWord(temp, word);
			//若没有where子句，正确返回
			if(!flag)
			{
				m_operation = DELETED;
				return;
			}
			if(word != "where")
				return;
			//开始解析where子句
			flag = GetWord(temp, word);
			if(!flag)
				return;
			tempcon = new condition;
			initcond(tempcon);
			strcpy(tempcon -> attrname, word.c_str());
			flag = GetWord(temp, word);
			if(!flag)
				return;
			if(word == "<")
				tempcon -> cond = LT;
			else if(word == "<=")
				tempcon -> cond = LE;
			else if(word == ">")
				tempcon -> cond = GT;
			else if(word == ">=")
				tempcon -> cond = GE;
			else if(word == "=")
				tempcon -> cond = EQ;
			else if(word == "<>")
				tempcon -> cond = NE;
			else return;
				
			flag = GetWord(temp, word);
			if(!flag)
				return;
			if(word == "\'")
			{
				tempcon -> type = CHAR; 
				flag = GetStr(temp, word);
				if(!flag)
					return;
				strcpy(tempcon -> value, word.c_str());
				flag = GetWord(temp, word);
				if(!flag)
					return;
				if(word != "\'")
					return;
			}
			else
			{
				strcpy(tempcon -> value, word.c_str());
				tempcon -> type = NOTCHAR;
			}
			m_conds = lastcon = tempcon;
			flag = GetWord(temp, word);
			if(!flag)
			{
				m_operation = DELETED;
				return;
			}
			while(word == "and" || word == "or")
			{
				flag = GetWord(temp, word);
				if(!flag)
					return;
				tempcon = new condition;
				initcond(tempcon);
				strcpy(tempcon -> attrname, word.c_str());
				flag = GetWord(temp, word);
				if(!flag)
					return;
				if(word == "<")
					tempcon -> cond = LT;
				else if(word == "<=")
					tempcon -> cond = LE;
				else if(word == ">")
					tempcon -> cond = GT;
				else if(word == ">=")
					tempcon -> cond = GE;
				else if(word == "=")
					tempcon -> cond = EQ;
				else if(word == "<>")
					tempcon -> cond = NE;
				else return;
				flag = GetWord(temp, word);
				if(!flag)
					return;
				if(word == "\'")
				{
					tempcon -> type = CHAR; 
					flag = GetStr(temp, word);
					if(!flag)
						return;
					strcpy(tempcon -> value, word.c_str());
					flag = GetWord(temp, word);
					if(!flag)
						return;
					if(word != "\'")
						return;
				}
				else
				{
					strcpy(tempcon -> value, word.c_str());
					tempcon -> type = NOTCHAR; 
				}
				lastcon -> next = tempcon;
				lastcon = tempcon;
				flag = GetWord(temp, word);
				if(!flag)
				{
					m_operation = DELETED;
					return;
				}
			}	
		}
	}
	//若是insert into命令
	else if(word == "insert")
	{
		flag = GetWord(temp, word);
		if(!flag)
			return;			
		if(word == "into")
		{
			m_operation = INSERTERR;
			flag = GetWord(temp, word);
			if(!flag)	
				return;
			m_tabname  = word.c_str();
			flag = GetWord(temp, word);
			if(!flag)	
				return;
			if(word != "values")
				return;
			flag = GetWord(temp, word);
			if(!flag)	
				return;
			if(word != "(")
				return;
			tempval = new insertvalue;
			initvalue(tempval);
			m_values = lastval = tempval;
			flag = GetWord(temp, word);
			if(!flag)
				return;
			if(word == "\'")
			{
				tempval -> type = CHAR;
				flag = GetStr(temp, word);
				if(!flag)
					return;
				strcpy(tempval -> value, word.c_str());
				flag = GetWord(temp, word);
				if(!flag)
					return;
				if(word != "\'")
					return;
			}
			else
			{
				strcpy(tempval -> value, word.c_str());
				tempval -> type = NOTCHAR;
			}
			flag = GetWord(temp, word);
			if(!flag)
				return;
			while(word == ",")
			{
				tempval = new insertvalue;
				tempval -> next = NULL;
				flag = GetWord(temp, word);
				if(!flag)
					return;
				if(word == "\'")
				{
					tempval -> type = CHAR;
					flag = GetStr(temp, word);
					if(!flag)
						return;
					strcpy(tempval -> value, word.c_str());
					flag = GetWord(temp, word);
					if(!flag)
						return;
					if(word != "\'")
						return;
				}
				else
				{
					strcpy(tempval -> value, word.c_str());
					tempval -> type = NOTCHAR;
				}
				lastval -> next = tempval;
				lastval = tempval;
				flag = GetWord(temp, word);
				if(!flag)
					return;
			}
			if(word != ")")
				return;
			flag = GetWord(temp, word);
			if(!flag)
				m_operation  = INSERT;		
		}
	}
	//若是drop table/drop index命令
	else if(word == "drop")
	{
		flag = GetWord(temp, word);
		if(!flag)
			return;
		if(word == "table")
		{
			m_operation = DRPTABERR;
			flag = GetWord(temp, word);
			if(!flag)
				return;
			m_tabname  = word.c_str();
			flag = GetWord(temp, word);
			if(!flag)
				m_operation = DRPTAB;
		}
		else if(word == "index")
		{
			m_operation = DRPINDERR;
			flag = GetWord(temp, word);
			if(!flag)
				return;
			m_indname = word.c_str();
			flag = GetWord(temp, word);
			if(!flag)
				m_operation = DRPIND;
		}		
	}
	//若是execfile命令
	else if(word == "execfile")
	{
		m_operation = EXEFILERR;
		flag = GetWord(temp, word);
		if(!flag)
			return;
		m_filename = word.c_str();
		flag = GetWord(temp, word);
		if(!flag)
			m_operation = EXEFILE;
	}
	return;
}

//释放解析树的内存 
void CInterpret::DumpTree()
{
	column *tempcol, *lastcol;
	condition *tempcond, *lastcond;
	insertvalue *tempval, *lastval;

	lastcol = m_cols;
	lastcond = m_conds;
	lastval = m_values;
	while(lastcol)
	{
		tempcol = lastcol;
		lastcol = tempcol -> next;
		delete tempcol;
	}
	while(lastcond)
	{
		tempcond = lastcond;
		lastcond = tempcond -> next;
		delete tempcond;
	}
	while(lastval)
	{
		tempval = lastval;
		lastval = tempval -> next;
		delete tempval;
	}
}

//根据解析树选择API，或返回错误信息 
void CInterpret::Execute()
{
	switch(m_operation)
	{
	case SELECT:
		ExecSelect();
		break;
	case DELETED:
		ExecDelete();
		break;
	case CRETAB:
		ExecCreateTable();
		break;
	case CREIND:
		ExecCreateIndex();
		break;
	case INSERT:
		ExecInsert();
		break;
	case DRPTAB:
		ExecDropTable();
		break;
	case DRPIND:
		ExecDropIndex();
		break;
	case EXEFILE:
		ExecFile();
		break;
	case QUIT:
		{
		CBufferManager::flush_all_blocks();
		Catalog.UpdateCatalog();
		DumpTree();	
		cout << "Press any key to close this window!" << endl;
		getchar();
		exit(0);
		}
		break;
	case EMPTY:
		cout << "Empty query! Please enter your command!" << endl;
		break;
	case UNKNOWN:
		cout << "Unknown query! Please check your input!" << endl;
		break;
	case SELERR:
		cout << "Incorrect usage of \"select\" query! Please check your input!" << endl;
		break;
	case CRETABERR:
		cout << "Incorrect usage of \"create table\" query! Please check your input!" << endl;
		break;
	case CREINDERR:
		cout << "Incorrect usage of \"create index\" query! Please check your input!" << endl;
		break;
	case DELETEERR:
		cout << "Incorrect usage of \"delete from\" query! Please check your input!" << endl;
		break;
	case INSERTERR:
		cout << "Incorrect usage of \"insert into\" query! Please check your input!" << endl;
		break;
	case DRPTABERR:
		cout << "Incorrect usage of \"drop table\" query! Please check your input!" << endl;
		break;
	case DRPINDERR:
		cout << "Incorrect usage of \"drop index\" query! Please check your input!" << endl;
		break;
	case VOIDPRI:
		cout << "Error: invalid primary key! Please check your input!" << endl;
		break;
	case VOIDUNI:
		cout << "Error: invalid unique key! Please check your input!" << endl;
		break;
	case CHARBOUD:
		cout << "Error: only 1~255 charactors is allowed! Please check your input!" << endl;
		break;
	case EXEFILERR:
		cout << "Incorrect usage of \"execfile\" command! Please check your input!" << endl;
		break;
	case NOPRIKEY:
		cout << "No primary key is defined! Please check your input!" << endl;
	}
	DumpTree();	
}

//检查 input 是否是数字 
short int CInterpret::IsInt(const char *input)
{
	int i;
	int length = strlen(input);
	if(!isdigit(input[0]) && !(input[0] == '-'))
		return 0;
	for(i = 1 ; i < length ; i ++)
	{
		if(!isdigit(input[i]))
			return 0;
	}
	return 1;
}

//检查 input 是否是浮点数 
short int CInterpret::IsFloat(char *input)
{
		int dot = 0;
		int i;
		int length = strlen(input);
		if(!isdigit(input[0]) && !(input[0] == '-'))
			return 0;
		for(i = 1 ; i < length ; i ++)
		{
			if(!isdigit(input[i]) &&  input[i] != '.')
				return 0;
			else if(input[i] == '.')
			switch(dot)
			{
				case 0:
					dot++;
					break;
				default:
					return 0;
			}
		}
		return 1;
}

////////////   API   //////////////// 

//实现SQL中 select 功能 
void CInterpret::ExecSelect()
{
	column *tempcol, *lastcol;
	condition *tempcond;
	unsigned int attrnum = 0;
	unsigned int i;
	short int condcount = 0;
	string indexname;
	short int type;
	unsigned int recordlen;
	unsigned int recordnum;

	if(Catalog.IsTableExists(m_tabname))
	{
		tempcol = m_cols;
		//如果要返回的是"*"，即全部返回，要从 catalog里取得所有属性的信息
		if(!strcmp(tempcol -> colname, "*") && tempcol -> next == NULL)
		{
			attrnum = Catalog.GetAttrNum(m_tabname);
			initcol(tempcol);
			Catalog.GetAttrInfo(m_tabname, 0, tempcol);
			lastcol = tempcol;
			for(i = 1 ; i < attrnum ; i ++)					
			{
				//得到第i个属性的信息
				tempcol = new column;
				initcol(tempcol);
				Catalog.GetAttrInfo(m_tabname, i, tempcol);
				lastcol -> next = tempcol;
				lastcol = tempcol;
			}
		}
		//否则只要验证是否有该属性并取得该属性的offset与length
		else while(tempcol)
		{
			if(!Catalog.IsAttrExists(m_tabname, tempcol -> colname))
			{
				printf("Error: '%s' is not an attribute of table '%s'! Please check your input!\n", m_tabname, tempcol -> colname);
				return;
			}
			tempcol->coloffset = Catalog.GetAttrOffset(m_tabname, tempcol -> colname);
			tempcol->collength = Catalog.GetAttrLength(m_tabname, tempcol -> colname);
			tempcol = tempcol -> next;
		}
		tempcond = m_conds;

		//验证where部分类型是否相符，并取得条件中各属性的offset及length
		while(tempcond)
		{
			condcount ++;
			if(!Catalog.IsAttrExists(m_tabname, tempcond -> attrname))
			{
				printf("Error: '%s' is not an attribute of table '%s'! Please check your input!\n", m_tabname, tempcond -> attrname);
				return;
			}
			type = Catalog.GetAttrTypeByName(m_tabname, tempcond -> attrname);
			if(type == INT && tempcond->type  == NOTCHAR)
			{
				if(!IsInt(tempcond->value))
				{
					printf("Error: data Type error with attribute '%s'! Please check your input!\n", tempcond ->attrname); 
					return;
				}
				tempcond -> type = INT;
			}
			else if(type == FLOAT && tempcond -> type == NOTCHAR)
			{
				if(!IsFloat(tempcond -> value))
				{
					printf("Error: data Type error with attribute '%s'! Please check your input!\n", tempcond ->attrname); 
					return;
				}
				tempcond -> type = FLOAT;
			}
			else if(type != CHAR || tempcond -> type != CHAR)
			{
					printf("Error: data Type error with attribute '%s'! Please check your input!\n", tempcond ->attrname); 
					return;
			}
			tempcond -> attroffset = Catalog.GetAttrOffset(m_tabname, tempcond -> attrname);
			tempcond -> attrlength = Catalog.GetAttrLength(m_tabname, tempcond -> attrname);
			tempcond = tempcond -> next;
		}
/*		//如果条件只有1项且是等值比较，可能存在index
		recordlen = Catalog.GetRecordLength(m_tabname);
		if(condcount == 1 && m_conds -> cond == EQ)
		{
			if(Catalog.IsIndexCreated(m_tabname, m_conds -> attrname))
			{
				indexname=Catalog.GetIndexName(m_tabname, m_conds -> attrname);
				//mirror
				CIndexManager idxmanager(indexname,m_tabname);
				idxmanager.SelectIndex(m_conds, m_cols, recordlen);
				return;
			}
		}*/
		recordnum = Catalog.GetRecordNumber(m_tabname);
		//mirror
		CRecordManager rdmanager(m_tabname);
		rdmanager.SelectRecord(m_conds, m_cols, recordlen);//, recordnum);	
	}
	else
		printf("Error: table '%s' dose not exist! Please check your input!\n", m_tabname);
}

//实现SQL中的 delete 功能 
void CInterpret::ExecDelete()
{
	condition *tempcond = m_conds;
	unsigned int condcount = 0;
	unsigned int type;
	string indexname;
	unsigned int recordlen;
	unsigned int recordnum;
	unsigned int deleted;

	if(Catalog.IsTableExists(m_tabname))
	{		
		//验证条件部分类型是否相符，并取得条件中各属性的offset及length
		while(tempcond)
		{
			condcount ++;
			if(!Catalog.IsAttrExists(m_tabname, tempcond -> attrname))
			{
				printf("Error: '%s' is not an attribute of table '%s'! Please check your input!\n", m_tabname, tempcond -> attrname);
				return;
			}
			type = Catalog.GetAttrTypeByName(m_tabname, tempcond -> attrname);
			if(type == INT && tempcond -> type  == NOTCHAR)
			{
				if(!IsInt(tempcond -> value))
				{
					printf("Error: data Type error with attribute '%s'! Please check your input!\n", tempcond ->attrname); 
					return;
				}
				tempcond -> type = INT;
			}
			else if(type == FLOAT && tempcond -> type == NOTCHAR)
			{
				if(!IsFloat(tempcond -> value))
				{
					printf("Error: data Type error with attribute '%s'! Please check your input!\n", tempcond ->attrname); 
					return;
				}
				tempcond -> type = FLOAT;
			}
			else if(type != CHAR || tempcond -> type != CHAR)
			{
					printf(" Error: data Type error with attribute '%s'! Please check your input!\n", tempcond ->attrname); 
					return;
			}
			tempcond -> attroffset = Catalog.GetAttrOffset(m_tabname, tempcond -> attrname);
			tempcond -> attrlength = Catalog.GetAttrLength(m_tabname, tempcond -> attrname);
			tempcond = tempcond -> next;
		}
/*		//如果条件只有1项，并且是等值比较,可能存在index
		recordlen = Catalog.GetRecordLength(m_tabname);
		if(condcount == 1 && m_conds -> cond == EQ)
		{
			if(Catalog.IsIndexCreated(m_tabname, m_conds -> attrname))
			{
				indexname=Catalog.GetIndexName(m_tabname, m_conds -> attrname);
				//mirror
				CIndexManager idxmanager(indexname, m_tabname);
				idxmanager.DeleteIndex(m_conds, recordlen);
				printf("1 record(s) are deleted from table '%s'\n", m_tabname);
				Catalog.RecordNumDel(m_tabname, 1);
				return;
			}
		}*/
		recordnum = Catalog.GetRecordNumber(m_tabname);
		//mirror
		CRecordManager rdmanager(m_tabname);
		deleted = rdmanager.DeleteRecord(m_conds, recordlen);//, recordnum);	
		Catalog.RecordNumDel(m_tabname, deleted);
		printf("%d record(s) are deleted from table '%s'\n", deleted, m_tabname);
	}
	else
		printf("Error: table '%s' dose not exist! Please check your input!\n", m_tabname);
}

//实现SQL中的 insert 功能 
void CInterpret::ExecInsert()
{
	insertvalue *tempval;
	unsigned AttrNum, i = 0;
	short int type;
	column *tempcol;
	unsigned int recordno;
	char indexname[NAMELEN];
	unsigned int recordlen;

	if(Catalog.IsTableExists(m_tabname))
	{
		tempval = m_values;
		tempcol = new column;
		AttrNum = Catalog.GetAttrNum(m_tabname);
		recordlen = Catalog.GetRecordLength(m_tabname);
		while(tempval && i < AttrNum)
		{

			Catalog.GetAttrInfo(m_tabname, i, tempcol);
			type = tempcol -> type;
			//检查类型是否正确
			if(type == INT && tempval -> type  == NOTCHAR)
			{
				if(!IsInt(tempval -> value))
				{
					printf("Error: data Type error ! Please check your input!\n"); 
					return;
				}

			}		
			else if(type == FLOAT && tempval -> type == NOTCHAR)
			{
				if(!IsFloat(tempval -> value))
				{
					printf("Error: data Type error! Please check your input!\n"); 
					return;
				}
			}
			else if(type != CHAR || tempval -> type != CHAR)
			{
					printf("Error: data Type error! Please check your input!\n"); 
					return;
			}

			//检查长度是否越界
			if(strlen(tempval -> value) > tempcol -> collength-1)
			{
				printf("Error: value of attribute '%s' is out of boundry! Please check your input!\n");
				return;
			}
			tempval -> length = tempcol -> collength;
			//检查唯一性
			if(tempcol -> IsUnique)
			{
				//mirror
				CRecordManager rdmanager(m_tabname);
				if(rdmanager.IsValueExists(tempcol, tempval -> value, recordlen))
				{
					printf("Error: Value duplicated on attribute '%s'!\n", tempcol -> colname);
					return;
				}
			}
			tempval->length = Catalog.GetAttrLength(m_tabname, tempcol->colname);
			i++;
			tempval = tempval -> next;
		}
		if(i != AttrNum || tempval)
		{
			printf("Error: the number of values to be inserted differs from the attributes number! Please check your input!\n");
			return;
		}
		//mirror
		CRecordManager rdmanager(m_tabname);
		recordno = rdmanager.InsertValues(m_values, recordlen);
		Catalog.RecordNumAdd(m_tabname, 1);
		//有索引的要插入索引
/*		tempval = m_values;
		for(i = 0 ; i < AttrNum ; i++)
		{
			Catalog.GetAttrInfo(m_tabname, i, tempcol);
			if(Catalog.IsIndexCreated(m_tabname, tempcol -> colname))
			{
				strcpy(indexname, Catalog.GetIndexName(m_tabname, tempcol -> colname));
				//mirror
				CIndexManager idxmanager(indexname, m_tabname);
				idxmanager.InsertIndex(tempval, recordno);
			}
			tempval = tempval -> next;
		}*/
		delete tempcol;
		printf("The record is inserted into table '%s' successful!\n", m_tabname);
	}
	else
		printf("Table '%s' dose not exist! Please check your input!\n", m_tabname);
}

//实现SQL中的 drop table 功能 
void CInterpret::ExecDropTable()
{
	char ch,syscommand[FILENAMELEN];
	unsigned int AttrNum, i;
	column *tempcol;
	char indexname[NAMELEN];
	
	if(Catalog.IsTableExists(m_tabname))
	{
		printf("Are you sure to delete table '%s'?\n",  m_tabname);
		cout << "'y' for 'yes', 'n' for 'no'" << endl;
		cin >> ch;
		if(ch == 'y')
		{
			//先删索引
			AttrNum = Catalog.GetAttrNum(m_tabname);
			tempcol = new column;
			for(i = 0 ; i < AttrNum ; i ++)
			{
				Catalog.GetAttrInfo(m_tabname, i, tempcol);
				if(Catalog.IsIndexCreated(m_tabname, tempcol -> colname))
				{
					strcpy(indexname, Catalog.GetIndexName(m_tabname, tempcol -> colname));
					Catalog.DeleteIndexInfo(indexname);
					strcpy(syscommand, "del ");
					strcat(syscommand, indexname);
					strcat(syscommand, ".idx");
					system(syscommand);
				}
			}
			delete tempcol;
			//删表格信息
			Catalog.DeleteTableInfo(m_tabname);
			//删除所有相关文件
			strcpy(syscommand, "del ");
			strcat(syscommand, m_tabname);
			strcat(syscommand, ".map");
			system(syscommand);
			strcpy(syscommand, "del ");
			strcat(syscommand, m_tabname);
			strcat(syscommand, ".tab");
			system(syscommand);
			printf("Table '%s' has been removed successfully!\n", m_tabname);
		}
		else
			printf("The drop table operation has been cancelled!\n");
	}
	else
		printf("Error: table '%s' dose not exist! Please check your input!\n", m_tabname);
}

//实现SQL中的 drop index 功能 
void CInterpret::ExecDropIndex()
{
	char ch,syscommand[FILENAMELEN];
	if(Catalog.IsIndexExists(m_indname))
	{
		printf("Are you sure to delete index '%s'?\n",  m_indname);
		cout << "'y' for 'yes', 'n' for 'no'" << endl;
		cin >> ch;
		if(ch == 'y')
		{
			Catalog.DeleteIndexInfo(m_indname);
			strcpy(syscommand, "del ");
			strcat(syscommand, m_indname);
			strcat(syscommand, ".idx");
			system(syscommand);
			printf("Index '%s' has been removed successfully!", m_indname);
		}
		else
			printf("The drop index operation has been cancelled!\n");
	}
	else
		printf("Error: index '%s' dose not exist! Please check your input!\n", m_indname);

}

//实现SQL中的 create table 功能 
void CInterpret::ExecCreateTable()
{
	short int primarynum = 0;
	column *tempcol = m_cols;
	column *primarycol = NULL;
	char filename[FILENAMELEN];
	int count=1;
	FILE *fp;
	
	//检查同名表格是否存在
	int flag = Catalog.IsTableExists(m_tabname);
	if(flag)
	{
		printf("Table '%s' already exists!\n", m_tabname);
		return;
	}
	//检查primary key是否超过1个
	while(tempcol)
	{
		if(tempcol -> IsPrimary)
		{
			primarynum ++;
			primarycol = tempcol;
		}
		tempcol = tempcol -> next;
	}
	if(primarynum > 1)
	{
		printf("Error: only one primary key allowed! Please check your input!\n");
		return;
	}
	//建table 的catalog
	Catalog.CreateTableInfo(m_tabname, m_cols);
	strcpy(filename, m_tabname);
	strcat(filename, ".tab");
	
	fp = fopen(filename, "w");
	fwrite(&count, sizeof(int), 1, fp);
	fclose(fp);
	
	strcpy(filename, m_tabname);
	strcat(filename, ".map");
	
	fp = fopen(filename, "w");
	fwrite(&count, sizeof(int), 1, fp);
	fclose(fp);
	
	//如果有primary key,建索引
	if(primarycol)
	{
		strcpy(filename,m_tabname);
		strcat(filename, ".idx");
	
		fp = fopen(filename, "w");
		fwrite(&count,sizeof(int),1,fp);
		fwrite(&count,sizeof(int),1,fp);
		fclose(fp);
	
		m_indname=m_tabname;
		Catalog.CreateIndexInfo(m_indname, m_tabname, primarycol);
	}
	printf("Table '%s' is created successfully\n",m_tabname);
}

//实现SQL中的 create index 功能 
void CInterpret::ExecCreateIndex()
{
	char filename[FILENAMELEN];
	FILE *fp;
	unsigned int recordlen;
	unsigned int recordnum;
	int count = 1;

	if(Catalog.IsTableExists(m_tabname))
	{
		//若属性不存在
		if(!Catalog.IsAttrExists(m_tabname, m_cols -> colname))
		{
			printf("Error: '%s' is not an attribute of table '%s'! Please check your input!\n", m_cols -> colname, m_tabname);
			return;
		}
		//若非unique属性
		if(!Catalog.IsAttrUnique(m_tabname, m_cols -> colname))
		{
			printf("Error '%s' is not an unique attribute! Please check your input!\n", m_cols -> colname);
			return;
		}
/*		//若该属性已建索引
		if(Catalog.IsIndexCreated(m_tabname, m_cols -> colname))
		{
			printf("Error: Index on attribute '%s' has already been built! Please check your input!", m_cols -> colname);
			return;
		}		
		m_cols -> coloffset = Catalog.GetAttrOffset(m_tabname, m_cols -> colname);
		m_cols -> collength = Catalog.GetAttrLength(m_tabname, m_cols -> colname);	
		m_cols -> type = Catalog.GetAttrTypeByName(m_tabname, m_cols -> colname);
		recordlen = Catalog.GetRecordLength(m_tabname);
		recordnum = Catalog.GetRecordNumber(m_tabname);
		strcpy(filename, m_tabname);
		strcat(filename, ".map");
		fp = fopen(filename, "w");
		fwrite(&count, sizeof(int), 1, fp);
		fclose(fp);
		strcpy(filename, m_indname);
		strcat(filename, ".idx");
		fp = fopen(filename, "w");
		fwrite(&count, sizeof(int), 1, fp);
		fclose(fp);
		//mirror
		CIndexManager idxmanager(m_indname, m_tabname);
		idxmanager.CreateIndex(m_cols, recordlen, recordnum);
		Catalog.CreateIndexInfo(m_indname, m_tabname, m_cols);	*/
	}
	else
		printf("Error: table '%s' dose not exist! Please check your input!\n", m_tabname);
}

//实现文件操作 
void CInterpret::ExecFile()
{
	FILE *fp;
	char command[COMLEN];
	unsigned int comnum;
	char c;
	CInterpret parsetree;

	if((fp = fopen(m_filename, "rb")) == NULL)
	{
		printf("Error: can not open file '%s'! Please check your input!\n", m_filename);
		return;
	}
	while(1)
	{
		comnum = 0;
		c = fgetc(fp);
		if(c == EOF)
			goto finish;
		while(c != ';')
		{
			//读到文件结束了
			if(c == EOF)
				goto finish;
			command[comnum++] = c;
			c = fgetc(fp);
		}
		command[comnum++] = '\0';
		parsetree.Parse(command);
		parsetree.Execute();
	}
finish:
	fclose(fp);
}

