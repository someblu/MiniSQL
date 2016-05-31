//#if !defined(AFX_MINISQL_H__40FC0CD6_696B_4458_A61E_F4437538AA22__INCLUDED_)
//#define AFX_MINISQL_H__40FC0CD6_696B_4458_A61E_F4437538AA22__INCLUDED_

//#if _MSC_VER > 1000
//#pragma once
//#endif // _MSC_VER > 1000

//#include "resource.h"

#define _CRT_SECURE_NO_WARNINGS    
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

//Macro definitions
//Buffer Manager
#define TABLE 1					//�������ǿ��д�����table����
#define INDEX 2					//�������ǿ��д�����index����
#define MAP 3					//�������ǿ��д�����map����

#define MAX_BLOCKS 1024		//buffer�����ܿ���
#define BLOCK_SIZE 8192			//ÿ�����ֽ���
#define NAME_SIZE  100			//�ļ����Ĵ�С
//Buffer Manager

//Index Manager
#define VALUE_LENGTH 244
#define FILE_NAME_LENGTH 50
#define INDEX_RECORD_LEN 256
#define RECORDS_PER_BLOCK 62
//Index Manager

//Interpret & API
#define SELECT		0
#define CRETAB		1
#define	CREIND		2
#define	DRPTAB		3
#define DRPIND		4
#define DELETED		5
#define INSERT		6
#define QUIT		7
#define EXEFILE		8 

#define EMPTY		11
#define UNKNOWN		12
#define SELERR		13
#define	CREINDERR	14
#define CRETABERR	15
#define DELETEERR	16
#define INSERTERR	17
#define DRPTABERR	18
#define DRPINDERR	19 
#define EXEFILERR	20
#define NOPRIKEY    24
#define VOIDPRI		21
#define VOIDUNI		22
#define CHARBOUD    23

#define LT			100			//less than
#define LE			101			//less or equal
#define	GT			102			//great than
#define GE			103			//great or equal
#define EQ			104			//equal
#define NE			105			//not equal

#define INT			201
#define CHAR		202
#define FLOAT		203
#define NOTCHAR		204

#define COMLEN		400 
#define INPUTLEN	200 
#define WORDLEN		100
#define VALLEN		300
#define FILENAMELEN	100
#define INTLEN		10
#define FLOATLEN	20
#define NAMELEN		100

//��Ӧ�����е�һ�У���һ������
typedef struct struct_column
{
	char colname[NAMELEN];		//������
	short int type;				//���Ե����ͣ�INT,CHAR��FLOAT
	unsigned int coloffset;		//������һ����¼�е�offset,���ֽ�Ϊ��λ
	unsigned int collength;		//���Եĳ��ȣ����ֽ�Ϊ��λ
	struct struct_column *next;			//��һ���ԣ��ڽ�����������ΪNULL
	short int IsPrimary;		//�������Ƿ�Ϊprimary key
	short int IsUnique;			//����������Ϊunique
}column;

//��Ӧwhere�����е�һ��,��һ������
typedef struct struct_condition
{
	char attrname[NAMELEN];		//������
	unsigned int cond;			//�Ƚ�����,LT,GT,QU,NE,LE,GE��һ��
	char value[255];			//�μӱȽϵĳ���
	short int type;				//�����Ե�����,���Ǳ�Ҫ��,��Ϊ���ֺ��ַ����ȽϷ�����һ��
	unsigned int attroffset;	//������һ����¼�е�offset,���ֽ�Ϊ��λ
	unsigned int attrlength;	//���Եĳ��ȣ����ֽ�Ϊ��λ
	struct struct_condition *next;		//��һ������
}condition;	

//��Ӧinsert into�����е�һ��������(����һ����¼!!)
typedef struct struct_insertvalue
{
	char value[VALLEN];			//������ֵ
	short int type;				//������ֵ����,��interpret��һ��ֻ�������ж�CHAR��NOTCHAR
	unsigned int length;		//ֵ����
	struct struct_insertvalue *next;	//��һ��ֵ
}insertvalue;

//#endif // !defined(AFX_MINISQL_H__40FC0CD6_696B_4458_A61E_F4437538AA22__INCLUDED_)
