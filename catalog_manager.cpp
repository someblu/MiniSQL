#include "MiniSQL.h"
#include "catalog_manager.h"

using namespace std;

//函数ReadCatalog被用来从指定.cat文件中构建表和索引的链表

void CCatalogManager::ReadCatalog()
{
	char buffer[100];
	FILE *fp;
	int tag = 0;								//表的链表的标签，记录是否首次创建节点
	int tagA = 0;								//属性的链表的标签
	int tagI = 0;								//索引的链表的标签
	int attrtype = 0;
	int length = 0;								//表的链表的节点中已有项目的总长度
	
	TablePointer ptrTable = NULL;				//表的链表的移动指针
	AttrPointer ptrAttr = NULL;					//属性的链表的移动指针
	IndexPointer ptrIndex = NULL;				//索引的链表的移动指针

	m_tablehead = m_tabletail = NULL;
	m_indexhead = m_indextail = NULL;

	//创建表的链表

	if ((fp = fopen("tables.catlog","r")) == NULL)//文件不存在
	{
		fp = fopen("tables.catlog","w");			//新建文件
		fprintf(fp,"-1\n-1\n");
	}

	fclose(fp);

	fp = fopen("tables.catlog","r");
	fscanf(fp,"%s",buffer);
	while(strcmp(buffer,"-1")!=0)				//文件非空
	{
		if(tag == 0)							//首次创建节点
		{
			m_tablehead = new TableNode;

			ptrTable = m_tablehead;
			ptrTable->AllAttrs = NULL;
			ptrTable->Next = NULL;
			m_tabletail = ptrTable;

			tag = 1;
		}
		else
		{
			ptrTable->Next = new TableNode;
			ptrTable = ptrTable->Next;
			ptrTable->Next = NULL;
			m_tabletail = ptrTable;
		}

		strcpy(ptrTable->TableName,buffer);

		fscanf(fp,"%s",buffer);
		strcpy(ptrTable->primarykey,buffer);

		fscanf(fp,"%s",buffer);
		ptrTable->number = atoi(buffer);
		
		fscanf(fp,"%s",buffer);

		while(strcmp(buffer,"-1")!=0)
		{
			if (tagA == 0)
			{
				ptrTable->AllAttrs = new AttrNode;
				ptrAttr = ptrTable->AllAttrs;
				tagA = 1;
			}
			else 
			{
				ptrAttr->Next = new AttrNode;
				ptrAttr = ptrAttr->Next;
			}

			strcpy(ptrAttr->AttrName,buffer);

			ptrAttr->offset = length;				//首次循环中的第一个属性值的偏移量为0

			if(strcmp((ptrAttr->AttrName),(ptrTable->primarykey)) == 0)
				ptrAttr->IsPrimarykey = 1;
			else 
				ptrAttr->IsPrimarykey = 0;

			fscanf(fp,"%s",buffer);
			if((strcmp(buffer,"INT"))==0)
			{
				ptrAttr->type = INT;
				ptrAttr->length = 10;
				length = length + 10;
			}
			else if((strcmp(buffer,"FLOAT"))==0)
			{
				ptrAttr->type = FLOAT;
				ptrAttr->length = 20;
				length = length + 20;
			}
			else
			{
				fscanf(fp,"%d",&attrtype);
				ptrAttr->type = CHAR;
				ptrAttr->length = attrtype;
				length = length + attrtype;
			}

			fscanf(fp,"%d",&attrtype);
			ptrAttr->IsUnique = attrtype;

			fscanf(fp,"%s",buffer);
		}
		ptrAttr->Next = NULL;
		tagA = 0;
		fscanf(fp,"%s",buffer);
		length = 0;
	}
	fclose(fp);

	//创建索引的链表
	
	if ((fp = fopen("index.catlog","r")) == NULL)	//文件不存在
	{
		fp = fopen("index.catlog","w");			//新建文件
		fprintf(fp,"-1\n");
	}

	fclose(fp);

	fp = fopen("index.catlog","r");
	fscanf(fp,"%s",buffer);
	while((strcmp(buffer,"-1")) != 0)					//不等于-1
	{
		if(tagI==0)
		{
			m_indexhead = new IndexNode;

			ptrIndex = m_indexhead;
			ptrIndex->Next = NULL;
			m_indextail = ptrIndex;

			tagI = 1;
		}
		else
		{
			ptrIndex->Next = new IndexNode;
			ptrIndex = ptrIndex->Next;
			ptrIndex->Next = NULL;
			m_indextail = ptrIndex;
		}

		strcpy(ptrIndex->IndexName,buffer);
		fscanf(fp,"%s",buffer);

		strcpy(ptrIndex->TableName,buffer);
		fscanf(fp,"%s",buffer);

		strcpy(ptrIndex->AttrName,buffer);
		fscanf(fp,"%s",buffer);
	}

	fclose(fp);
}

//函数UpdateCatalog被用来写回表和索引的链表

void CCatalogManager::UpdateCatalog()
{
	char buffer[100];
	FILE* fp;

	if(m_tablehead != NULL){							//如果为空，说明.cat文件刚被readcatalog建立 
														//所以不需要写回

	TablePointer ptrTable = m_tablehead;				//表的链表的移动指针
	AttrPointer ptrAttr = ptrTable->AllAttrs;			//属性的链表的移动指针
	IndexPointer ptrIndex = m_indexhead;				//索引的链表的移动指针
	
	TablePointer tempT = ptrTable;						//删除表的链表时
	AttrPointer tempA = ptrAttr;						//用到的临时指针
	
	IndexPointer temp = ptrIndex;						//删除索引的链表时
														//用到的临时指针
	int attrtype = 0;
	unsigned int len = 0;
	int tagP = 0;										

	//向指定的.cat文件写回表的链表

	if ((fp = fopen("tables.catlog","r")) == NULL)				//文件不存在
	{	
		fp = fopen("tables.catlog","w");						//新建文件
	}	
	fclose(fp);

	fp = fopen("tables.catlog","w");							//开始写文件
	
	while(ptrTable != NULL)
	{
		strcpy(buffer,ptrTable->TableName);				//表的名字
		fprintf(fp,buffer);
		fprintf(fp,"\n");
		
		strcpy(buffer,ptrTable->primarykey);			//表的主键
		fprintf(fp,buffer);
	
		fprintf(fp,"\n");
		
		itoa(ptrTable->number,buffer,10);
		fprintf(fp,buffer);
		fprintf(fp,"\n");
		
		
		ptrAttr = ptrTable ->AllAttrs;
		while(ptrAttr != NULL)							//表非空
		{
			strcpy(buffer,ptrAttr->AttrName);			//第一个属性的名字
			fprintf(fp,buffer);
			fprintf(fp,"\n");

			if(ptrAttr->type == INT)
			{
				fprintf(fp,"INT");
				fprintf(fp,"\n");
			}
			else if(ptrAttr->type == FLOAT)
			{
				fprintf(fp,"FLOAT");
				fprintf(fp,"\n");
			}
			else 
			{
				fprintf(fp,"CHAR");
				fprintf(fp,"\n");
				len = ptrAttr->length;
				fprintf(fp,"%u",len);
				fprintf(fp,"\n");
			}

			tagP = ptrAttr->IsUnique;
			fprintf(fp,"%d",tagP);
			fprintf(fp,"\n");

			ptrAttr = ptrAttr->Next;
		}
		fprintf(fp,"-1\n");
		ptrTable = ptrTable->Next;

	}
	fprintf(fp,"-1\n");

	fclose(fp);

	//free the table_list memory

	ptrTable = m_tablehead;
	ptrAttr = ptrTable->AllAttrs;

	while(ptrTable != NULL)
	{
		while(ptrAttr != NULL)
		{
			ptrAttr = ptrAttr->Next;
			delete tempA;
			tempA = ptrAttr;
		}
		delete ptrAttr;
		
		ptrTable = ptrTable->Next;
		delete tempT;
		tempT = ptrTable;
	}

	//向指定的.cat文件写回索引的链表

	if ((fp = fopen("index.catlog","r")) == NULL)				//文件不存在
	{	
		fp = fopen("index.catlog","w");						//新建文件
	}	
	fclose(fp);

	fp = fopen("index.catlog","w");							//开始写文件
	
	while(ptrIndex != NULL)
	{
		strcpy(buffer,ptrIndex->IndexName);
		fprintf(fp,buffer);
		fprintf(fp,"\n");

		strcpy(buffer,ptrIndex->TableName);
		fprintf(fp,buffer);
		fprintf(fp,"\n");

		strcpy(buffer,ptrIndex->AttrName);
		fprintf(fp,buffer);
		fprintf(fp,"\n");

		ptrIndex = ptrIndex->Next;
	}

	fprintf(fp,"-1\n");

	fclose(fp);

	//free the index_list memory

	ptrIndex = m_indexhead;

	while(ptrIndex != NULL)
	{
		ptrIndex = ptrIndex->Next;
		delete temp;
		temp = ptrIndex;
	}
	}
	else{
		if ((fp = fopen("tables.catlog","r")) == NULL)				//文件不存在
		{	
			fp = fopen("tables.catlog","w");						//新建文件
		}	
		fclose(fp);	
		fp = fopen("tables.catlog","w");
		fprintf(fp,"-1\n-1\n");
		fclose(fp);	
		
		if ((fp = fopen("index.catlog","r")) == NULL)				//文件不存在
	    {	
		fp = fopen("index.catlog","w");						//新建文件
		}	
		fclose(fp);
		fp = fopen("index.catlog","w");	
		fprintf(fp,"-1\n");
		fclose(fp);		
	}
}

short int CCatalogManager::IsTableExists(const char* tablename)
{
	TablePointer ptrTable  = m_tablehead;
	
	while(ptrTable != NULL)
	{
		if(strcmp(ptrTable->TableName,tablename) == 0)
			return 1;
		else
			ptrTable = ptrTable->Next;
	}
	return 0;
}

short int CCatalogManager::DeleteTableInfo(const char* tablename)
{
	int i = 0;
	TablePointer ptrTable  = m_tablehead;
	TablePointer temp = m_tablehead;
	while(ptrTable != NULL)
	{
		if(strcmp(ptrTable->TableName,tablename) == 0)
		{
			if(ptrTable == m_tablehead)
			{
				m_tablehead = ptrTable->Next;
				if(ptrTable ==m_tabletail)
				m_tabletail = NULL;
				delete ptrTable;
				return 1;
			}
			else if(ptrTable == m_tabletail)
			{
				temp->Next = NULL;
				m_tabletail = temp;
				delete ptrTable;
				return 1;
			}
			else
			{
				temp->Next = ptrTable->Next;
				delete ptrTable;
				return 1;
			}
		}

		else
		{
			temp = ptrTable;
			ptrTable = ptrTable->Next;
		}
	
	}
	return 0;
}

short int CCatalogManager::IsIndexExists(const char* indexname)
{
	IndexPointer ptrIndex = m_indexhead;

	while(ptrIndex != NULL)
	{
		if(strcmp(ptrIndex->IndexName,indexname) == 0)
			return 1;
		else
			ptrIndex = ptrIndex->Next;
	}
	return 0;
}

short int CCatalogManager::DeleteIndexInfo(const char* indexname)
{
	IndexPointer ptrIndex  = m_indexhead;
	IndexPointer temp = m_indexhead;
	while(ptrIndex != NULL)
	{
		if(strcmp(ptrIndex->TableName,indexname) == 0)
		{
			if(ptrIndex == m_indexhead)
			{
				m_indexhead = ptrIndex->Next;
				if(ptrIndex==m_indextail)
				m_indextail=NULL;
				delete ptrIndex;
				return 1;
			}
			else if(ptrIndex == m_indextail)
			{
				temp->Next = NULL;
				m_indextail = temp;
				delete ptrIndex;
				return 1;
			}
			else
			{
				temp->Next = ptrIndex->Next;
				delete ptrIndex;
				return 1;
			}
		}
		else
		{
			temp = ptrIndex;
			ptrIndex = ptrIndex->Next;
		}
	}
	return 0;
}

short int CCatalogManager::IsAttrExists(const char* tablename, const char* attrname)
{
	if(IsTableExists(tablename) == 1)
	{
		TablePointer ptrTable  = m_tablehead;
		TablePointer temp = NULL;
		AttrPointer tempattr = NULL;
		
		while(ptrTable != NULL)
		{
			if(strcmp(ptrTable->TableName,tablename) == 0)
			{
				temp = ptrTable;								//tempָ指针暂存表指针
				ptrTable = NULL;
			}
			else
				ptrTable = ptrTable->Next;
		}
		tempattr = temp -> AllAttrs;
		while(tempattr != NULL)
		{
			if(strcmp(tempattr -> AttrName,attrname) == 0)
				return 1;
			else
				tempattr = tempattr -> Next;
		}
	}
	return 0;
}

short int CCatalogManager::GetAttrTypeByName(const char* tablename, const char* attrname)
{
	TablePointer ptrTable  = m_tablehead;
	TablePointer temp = NULL;
	AttrPointer tempattr; 

	if(ptrTable != NULL)									//表链表至少有一个节点
	{
		while(ptrTable != NULL)
		{
			if(strcmp(ptrTable->TableName,tablename) == 0)
			{
				temp = ptrTable;								//tempָ指针暂存表指针
				ptrTable = NULL;
			}
			else
				ptrTable = ptrTable->Next;
		}
		tempattr = temp -> AllAttrs;
		while(tempattr != NULL)
		{
			if(strcmp(tempattr->AttrName,attrname) == 0)
			{
				return tempattr->type;
			}
			else
			{
				tempattr = tempattr->Next;
			}
		}
	}
	return 0;
}

short int CCatalogManager::GetAttrTypeByNum(const char* tablename, unsigned int attrnum)
{
	int i = 0;
	TablePointer ptrTable  = m_tablehead;
	TablePointer temp = NULL;
	AttrPointer tempattr;

	if(ptrTable != NULL)
	{
		while(ptrTable != NULL)
		{
			if(strcmp(ptrTable->TableName,tablename) == 0)
			{
				temp = ptrTable;								//tempָ指针暂存表指针
				ptrTable = NULL;
			}
			else
			{
				ptrTable = ptrTable->Next;
			}
		}
		tempattr=temp->AllAttrs;
		while(tempattr != NULL)
		{
			for(i=0;i<(int)attrnum;i++)
			{
				tempattr = tempattr->Next;
			}
			return tempattr->type;
		}
	}
	return 0;
}

unsigned int CCatalogManager::GetAttrOffset(const char* tablename, const char* attrname)
{
	TablePointer ptrTable  = m_tablehead;
	TablePointer temp = NULL;
	AttrPointer tempattr;

	if(ptrTable != NULL)
	{
		while(ptrTable != NULL)
		{
			if(strcmp(ptrTable->TableName,tablename) == 0)
			{
				temp = ptrTable;								//tempָ指针暂存表指针
				ptrTable = NULL;
			}
			else
				ptrTable = ptrTable->Next;
		}
		tempattr=temp->AllAttrs;
		while(tempattr != NULL)
		{
			if(strcmp(tempattr->AttrName,attrname) == 0)
			{
				return tempattr->offset;
			}
			else
			{
				tempattr= tempattr->Next;
			}
		}
	}
	return 0;
}

unsigned int CCatalogManager::GetAttrLength(const char* tablename, const char* attrname)
{
	TablePointer ptrTable  = m_tablehead;
	TablePointer temp = NULL;
	AttrPointer tempattr;

	if(ptrTable != NULL)
	{
		while(ptrTable != NULL)
		{
			if(strcmp(ptrTable->TableName,tablename) == 0)
			{
				temp = ptrTable;								//tempָ指针暂存表指针
				ptrTable = NULL;
			}
			else
				ptrTable = ptrTable->Next;
		}
		tempattr=temp->AllAttrs;
		while(tempattr != NULL)
		{
			if(strcmp(tempattr->AttrName,attrname) == 0)
			{
				return tempattr->length;
			}
			else
			{
				tempattr = tempattr->Next;
			}
		}
	}
	return 0;
}

unsigned int CCatalogManager::GetRecordLength(const char* tablename)
{
	int len = 0;
	TablePointer ptrTable  = m_tablehead;
	TablePointer temp = NULL;
	AttrPointer tempattr;

	if(ptrTable != NULL)
	{
		if(IsTableExists(tablename) == 1)
		{
			while(ptrTable != NULL)
			{
				if(strcmp(ptrTable->TableName,tablename) == 0)
				{
					temp = ptrTable;							//tempָ指针暂存表指针
					ptrTable = NULL;
				}
				else
					ptrTable = ptrTable->Next;
			}
			tempattr=temp->AllAttrs;
			while(tempattr!= NULL)
			{
				len += tempattr->length;
				tempattr = tempattr->Next;
			}
		
			return len;
		}
	}
	return 0;
}

short int CCatalogManager::IsIndexCreated(const char* tablename, const char* attrname)
{
	IndexPointer ptrIndex = m_indexhead;
	
	while(ptrIndex != NULL)
	{
		if(strcmp(ptrIndex->TableName,tablename) == 0)
			if(strcmp(ptrIndex->AttrName,attrname) == 0)
				return 1;
		ptrIndex = ptrIndex->Next;
	}

	return 0;
}

const char* CCatalogManager::GetIndexName(const char* tablename, const char* attrname)
{
	IndexPointer ptrIndex = m_indexhead;

	while(ptrIndex != NULL)
	{
		if(strcmp(ptrIndex->TableName,tablename) == 0)
		{
			if(strcmp(ptrIndex->AttrName,attrname) == 0)
				return ptrIndex->IndexName;
			else 
				ptrIndex = ptrIndex->Next;
		}
		else
			ptrIndex = ptrIndex->Next;
	}

	return (const char*)("");
}

unsigned int CCatalogManager::GetAttrNum(const char* tablename)
{
	int num = 0;
	TablePointer ptrTable  = m_tablehead;
	AttrPointer ptrAttr;
	TablePointer temp = NULL;
	
	if(IsTableExists(tablename) == 1)
	{
		while(ptrTable != NULL)
		{
			if(strcmp(ptrTable->TableName,tablename) == 0)
			{
				temp = ptrTable;								//tempָ指针暂存表指针
				ptrTable = NULL;
			}
			else
				ptrTable = ptrTable->Next;
		}
		ptrAttr = temp ->AllAttrs;
		while(ptrAttr!= NULL)
		{
			num ++;
			ptrAttr = ptrAttr->Next;
		}		
		return num;
	}
	return 0;
}

short int CCatalogManager::GetAttrInfo(const char* tablename, unsigned int attrnum, column* tempcol)
{
	int i = 0;
	TablePointer ptrTable  = m_tablehead;
	TablePointer temp = NULL;
	AttrPointer tempattr;
	if(ptrTable != NULL)
	{
		while(ptrTable != NULL)
		{
			if(strcmp(ptrTable->TableName,tablename) == 0)
			{
				temp = ptrTable;								//tempָ指针暂存表指针
				ptrTable = NULL;
			}
			else
			{
				ptrTable = ptrTable->Next;
			}
		}
		tempattr=temp->AllAttrs;
		if(tempattr != NULL)
		{
			for(i=0;i<(int)attrnum;i++)
			{
				tempattr = tempattr->Next;
			}
			
			tempcol->collength = tempattr ->length;
			strcpy(tempcol->colname,tempattr->AttrName);
			tempcol->coloffset =tempattr->offset;
			tempcol->IsPrimary = tempattr->IsPrimarykey;
			tempcol->IsUnique = tempattr->IsUnique;
			tempcol->type = tempattr->type;

			return 1;
		}
	}
	return 0;
}

short int CCatalogManager::CreateTableInfo(const char* tablename, column* cols)
{
	AttrPointer ptrAttr = NULL;	
	column* temp = cols;									//保存初始列指针
	int tagA = 0;
	int len = 0;

	TablePointer ptrTable = new TableNode;
	TablePointer ptr = m_tablehead;
	if(m_tabletail != NULL)
	{
		m_tabletail->Next = ptrTable;
		m_tabletail = ptrTable;
		ptrTable->Next = NULL;
	}
	else
	{
		m_tabletail = ptrTable;
		m_tablehead = ptrTable;
		ptrTable->Next = NULL;
	}
	
	strcpy(ptrTable->TableName,tablename);					//复制表的名字
		
	ptrTable->number = 0;
	while((cols->IsPrimary == 0) && (cols->next != NULL))	//复制主键
	{
		cols = cols->next;
	}
	if((cols->IsPrimary == 0) && (cols->next == NULL))
	{
		strcpy(ptrTable->primarykey,"NULL");
	}
	else
	{
		strcpy(ptrTable->primarykey,cols->colname);
	}
	cols = temp;

	while(cols != NULL)										//复制属性
	{
		if(tagA == 0)
		{
			ptrTable->AllAttrs = new AttrNode;
			ptrAttr = ptrTable->AllAttrs;
			tagA = 1;
		}
		else
		{
			ptrAttr->Next = new AttrNode;
			ptrAttr = ptrAttr->Next;
		}

		strcpy(ptrAttr->AttrName,cols->colname);
		ptrAttr->IsPrimarykey = cols->IsPrimary;
		ptrAttr->IsUnique = cols->IsUnique;
		ptrAttr->length = cols->collength;
		ptrAttr->type = cols->type;

		if(cols->type == INT)
		{
			ptrAttr->offset = len;
			len = len + 10;
		}
		else if (cols->type == FLOAT)
		{
			ptrAttr->offset = len;
			len = len + 20;
		}
		else
		{
			ptrAttr->offset = len;
			len = len + cols->collength;
		}

		cols = cols->next;
	}
	ptrAttr->Next = NULL;
	cols = temp;

	return 1;
}

short int CCatalogManager::CreateIndexInfo(const char* indexname, const char* tablename, column* cols)
{
	IndexPointer ptrIndex = new IndexNode;
	
	if(m_indextail != NULL)
	{
		m_indextail->Next = ptrIndex;
		m_indextail = ptrIndex;
		ptrIndex->Next = NULL;
	}
	else
	{
		m_indextail = ptrIndex;
		m_indextail = ptrIndex;
		ptrIndex->Next = NULL;
	}
	strcpy(ptrIndex->IndexName,indexname);
	strcpy(ptrIndex->TableName,tablename);
	strcpy(ptrIndex->AttrName,cols->colname);

	return 1;
}

short int CCatalogManager::IsAttrUnique(const char* tablename, const char* attrname)
{
	TablePointer ptrTable = m_tablehead;
	TablePointer temp = NULL;
	AttrPointer tempattr;
	if(ptrTable != NULL)
	{
		while(ptrTable != NULL)
		{
			if(strcmp(ptrTable->TableName,tablename) == 0)
			{
				temp = ptrTable;								//tempָ指针暂存表指针
				ptrTable = NULL;
			}
			else
				ptrTable = ptrTable->Next;
		}
		tempattr=temp->AllAttrs;
		while(tempattr != NULL)
		{
			if(strcmp(tempattr->AttrName,attrname) == 0)
			{
				if(tempattr->IsUnique == 1)
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else
			{
				tempattr = tempattr->Next;
			}
		}
	}
	return 0;
}

unsigned int CCatalogManager::GetRecordNumber(const char* tablename)
{
	TablePointer ptrTable  = m_tablehead;
	TablePointer temp = NULL;

	if(ptrTable != NULL)
	{
		while(ptrTable != NULL)
		{
			if(strcmp(ptrTable->TableName,tablename) == 0)
			{
				temp = ptrTable;								//tempָ指针暂存表指针
				ptrTable = NULL;
			}	
			else
				ptrTable = ptrTable->Next;
		}

		return temp->number;
	}
	return 0;
}

void CCatalogManager::RecordNumDel(const char* tablename, unsigned int deleted)
{
	TablePointer ptrTable  = m_tablehead;
	TablePointer temp = NULL;

	if(ptrTable != NULL)
	{
		while(ptrTable != NULL)
		{
			if(strcmp(ptrTable->TableName,tablename) == 0)
			{
				temp = ptrTable;								//tempָ指针暂存表指针
				ptrTable = NULL;
			}	
			else
				ptrTable = ptrTable->Next;
		}

		temp->number = temp->number - deleted;
	}
}

void CCatalogManager::RecordNumAdd(const char* tablename, unsigned int added)
{
	TablePointer ptrTable  = m_tablehead;
	TablePointer temp = NULL;

	if(ptrTable != NULL)
	{
		while(ptrTable != NULL)
		{
			if(strcmp(ptrTable->TableName,tablename) == 0)
			{
				temp = ptrTable;								//tempָ指针暂存表指针
				ptrTable = NULL;
			}	
			else
				ptrTable = ptrTable->Next;
		}

		temp->number = temp->number + added;
	}
}
