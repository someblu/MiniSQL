#include "MiniSQL.h"
#include "catalog_manager.h"

using namespace std;

//å‡½æ•°ReadCatalogè¢«ç”¨æ¥ä»ŽæŒ‡å®š.catæ–‡ä»¶ä¸­æž„å»ºè¡¨å’Œç´¢å¼•çš„é“¾è¡¨

void CCatalogManager::ReadCatalog()
{
	char buffer[100];
	FILE *fp;
	int tag = 0;								//è¡¨çš„é“¾è¡¨çš„æ ‡ç­¾ï¼Œè®°å½•æ˜¯å¦é¦–æ¬¡åˆ›å»ºèŠ‚ç‚¹
	int tagA = 0;								//å±žæ€§çš„é“¾è¡¨çš„æ ‡ç­¾
	int tagI = 0;								//ç´¢å¼•çš„é“¾è¡¨çš„æ ‡ç­¾
	int attrtype = 0;
	int length = 0;								//è¡¨çš„é“¾è¡¨çš„èŠ‚ç‚¹ä¸­å·²æœ‰é¡¹ç›®çš„æ€»é•¿åº¦
	
	TablePointer ptrTable = NULL;				//è¡¨çš„é“¾è¡¨çš„ç§»åŠ¨æŒ‡é’ˆ
	AttrPointer ptrAttr = NULL;					//å±žæ€§çš„é“¾è¡¨çš„ç§»åŠ¨æŒ‡é’ˆ
	IndexPointer ptrIndex = NULL;				//ç´¢å¼•çš„é“¾è¡¨çš„ç§»åŠ¨æŒ‡é’ˆ

	m_tablehead = m_tabletail = NULL;
	m_indexhead = m_indextail = NULL;

	//åˆ›å»ºè¡¨çš„é“¾è¡¨

	if ((fp = fopen("tables.catlog","r")) == NULL)//æ–‡ä»¶ä¸å­˜åœ¨
	{
		fp = fopen("tables.catlog","w");			//æ–°å»ºæ–‡ä»¶
		fprintf(fp,"-1\n-1\n");
	}

	fclose(fp);

	fp = fopen("tables.catlog","r");
	fscanf(fp,"%s",buffer);
	while(strcmp(buffer,"-1")!=0)				//æ–‡ä»¶éžç©º
	{
		if(tag == 0)							//é¦–æ¬¡åˆ›å»ºèŠ‚ç‚¹
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

			ptrAttr->offset = length;				//é¦–æ¬¡å¾ªçŽ¯ä¸­çš„ç¬¬ä¸€ä¸ªå±žæ€§å€¼çš„åç§»é‡ä¸º0

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

	//åˆ›å»ºç´¢å¼•çš„é“¾è¡¨
	
	if ((fp = fopen("index.catlog","r")) == NULL)	//æ–‡ä»¶ä¸å­˜åœ¨
	{
		fp = fopen("index.catlog","w");			//æ–°å»ºæ–‡ä»¶
		fprintf(fp,"-1\n");
	}

	fclose(fp);

	fp = fopen("index.catlog","r");
	fscanf(fp,"%s",buffer);
	while((strcmp(buffer,"-1")) != 0)					//ä¸ç­‰äºŽ-1
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

//å‡½æ•°UpdateCatalogè¢«ç”¨æ¥å†™å›žè¡¨å’Œç´¢å¼•çš„é“¾è¡¨

void CCatalogManager::UpdateCatalog()
{
	char buffer[100];
	FILE* fp;

	if(m_tablehead != NULL){							//Èç¹ûÎª¿Õ£¬ËµÃ÷.catÎÄ¼þ¸Õ±»readcatalog½¨Á¢ 
														//ËùÒÔ²»ÐèÒªÐ´»Ø

	TablePointer ptrTable = m_tablehead;				//±íµÄÁ´±íµÄÒÆ¶¯Ö¸Õë
	AttrPointer ptrAttr = ptrTable->AllAttrs;			//ÊôÐÔµÄÁ´±íµÄÒÆ¶¯Ö¸Õë
	IndexPointer ptrIndex = m_indexhead;				//Ë÷ÒýµÄÁ´±íµÄÒÆ¶¯Ö¸Õë
	
	TablePointer tempT = ptrTable;						//É¾³ý±íµÄÁ´±íÊ±
	AttrPointer tempA = ptrAttr;						//ÓÃµ½µÄÁÙÊ±Ö¸Õë
	
	IndexPointer temp = ptrIndex;						//É¾³ýË÷ÒýµÄÁ´±íÊ±
														//ÓÃµ½µÄÁÙÊ±Ö¸Õë
	int attrtype = 0;
	unsigned int len = 0;
	int tagP = 0;										

	//ÏòÖ¸¶¨µÄ.catÎÄ¼þÐ´»Ø±íµÄÁ´±í

	if ((fp = fopen("tables.catlog","r")) == NULL)				//ÎÄ¼þ²»´æÔÚ
	{	
		fp = fopen("tables.catlog","w");						//ÐÂ½¨ÎÄ¼þ
	}	
	fclose(fp);

	fp = fopen("tables.catlog","w");							//¿ªÊ¼Ð´ÎÄ¼þ
	
     while(ptrTable != NULL)
	{
		strcpy(buffer,ptrTable->TableName);				//±íµÄÃû×Ö
		fprintf(fp,buffer);
		fprintf(fp,"\n");
		
		strcpy(buffer,ptrTable->primarykey);			//±íµÄÖ÷¼ü
		fprintf(fp,buffer);
	
		fprintf(fp,"\n");
		
		itoa(ptrTable->number,buffer,10);
		fprintf(fp,buffer);
		fprintf(fp,"\n");
		
		
		ptrAttr = ptrTable ->AllAttrs;
		while(ptrAttr != NULL)							//±í·Ç¿Õ
		{
			strcpy(buffer,ptrAttr->AttrName);			//µÚÒ»¸öÊôÐÔµÄÃû×Ö
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

	//ÏòÖ¸¶¨µÄ.catÎÄ¼þÐ´»ØË÷ÒýµÄÁ´±í

	if ((fp = fopen("index.catlog","r")) == NULL)				//ÎÄ¼þ²»´æÔÚ
	{	
		fp = fopen("index.catlog","w");						//ÐÂ½¨ÎÄ¼þ
	}	
	fclose(fp);

	fp = fopen("index.catlog","w");							//¿ªÊ¼Ð´ÎÄ¼þ
	
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
		if ((fp = fopen("tables.catlog","r")) == NULL)				//ÎÄ¼þ²»´æÔÚ
		{	
			fp = fopen("tables.catlog","w");						//ÐÂ½¨ÎÄ¼þ
		}	
		fclose(fp);	
		fp = fopen("tables.catlog","w");
		fprintf(fp,"-1\n-1\n");
		fclose(fp);	
		
		if ((fp = fopen("index.catlog","r")) == NULL)				//ÎÄ¼þ²»´æÔÚ
	    {	
		fp = fopen("index.catlog","w");						//ÐÂ½¨ÎÄ¼þ
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
				temp = ptrTable;								//tempÖ¸æŒ‡é’ˆæš‚å­˜è¡¨æŒ‡é’ˆ
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

	if(ptrTable != NULL)									//è¡¨é“¾è¡¨è‡³å°‘æœ‰ä¸€ä¸ªèŠ‚ç‚¹
	{
		while(ptrTable != NULL)
		{
			if(strcmp(ptrTable->TableName,tablename) == 0)
			{
				temp = ptrTable;								//tempÖ¸æŒ‡é’ˆæš‚å­˜è¡¨æŒ‡é’ˆ
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
				temp = ptrTable;								//tempÖ¸æŒ‡é’ˆæš‚å­˜è¡¨æŒ‡é’ˆ
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
				temp = ptrTable;								//tempÖ¸æŒ‡é’ˆæš‚å­˜è¡¨æŒ‡é’ˆ
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
				temp = ptrTable;								//tempÖ¸æŒ‡é’ˆæš‚å­˜è¡¨æŒ‡é’ˆ
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
					temp = ptrTable;							//tempÖ¸æŒ‡é’ˆæš‚å­˜è¡¨æŒ‡é’ˆ
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
				temp = ptrTable;								//tempÖ¸æŒ‡é’ˆæš‚å­˜è¡¨æŒ‡é’ˆ
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
				temp = ptrTable;								//tempÖ¸æŒ‡é’ˆæš‚å­˜è¡¨æŒ‡é’ˆ
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
	column* temp = cols;									//ä¿å­˜åˆå§‹åˆ—æŒ‡é’ˆ
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
	
	strcpy(ptrTable->TableName,tablename);					//å¤åˆ¶è¡¨çš„åå­—
		
	ptrTable->number = 0;
	while((cols->IsPrimary == 0) && (cols->next != NULL))	//å¤åˆ¶ä¸»é”®
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

	while(cols != NULL)										//å¤åˆ¶å±žæ€§
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
				temp = ptrTable;								//tempÖ¸æŒ‡é’ˆæš‚å­˜è¡¨æŒ‡é’ˆ
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
				temp = ptrTable;								//tempÖ¸æŒ‡é’ˆæš‚å­˜è¡¨æŒ‡é’ˆ
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
				temp = ptrTable;								//tempÖ¸æŒ‡é’ˆæš‚å­˜è¡¨æŒ‡é’ˆ
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
				temp = ptrTable;								//tempÖ¸æŒ‡é’ˆæš‚å­˜è¡¨æŒ‡é’ˆ
				ptrTable = NULL;
			}	
			else
				ptrTable = ptrTable->Next;
		}

		temp->number = temp->number + added;
	}
}
