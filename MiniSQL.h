#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdlib.h>

//Macro definitions
//Buffer Manager
#define TABLE 1					//用来标记块中存的是table数据
#define INDEX 2					//用来标记块中存的是index数据
#define MAP 3					//用来标记块中存的是map数据

#define MAX_BLOCKS 1024		//buffer区内总块数
#define BLOCK_SIZE 8192			//每块的字节数
#define NAME_SIZE  100			//文件名的大小
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

//对应表格中的一列，即一个属性
typedef struct struct_column
{
	char colname[NAMELEN];		//属性名
	short int type;				//属性的类型，INT,CHAR或FLOAT
	unsigned int coloffset;		//属性在一条记录中的offset,以字节为单位
	unsigned int collength;		//属性的长度，以字节为单位
	struct struct_column *next;			//下一属性，在建索引命令中为NULL
	short int IsPrimary;		//该属性是否为primary key
	short int IsUnique;			//该属性书否为unique
}column;

//对应where语句中的一项,即一个条件
typedef struct struct_condition
{
	char attrname[NAMELEN];		//属性名
	unsigned int cond;			//比较条件,LT,GT,QU,NE,LE,GE中一种
	char value[255];			//参加比较的常数
	short int type;				//该属性的类型,这是必要的,因为数字和字符串比较方法不一样
	unsigned int attroffset;	//属性在一条记录中的offset,以字节为单位
	unsigned int attrlength;	//属性的长度，以字节为单位
	struct struct_condition *next;		//下一个条件
}condition;

//对应insert into语句中的一个插入项(不是一条记录!!)
typedef struct struct_insertvalue
{
	char value[VALLEN];			//插入的值
	short int type;				//插入的值类型,在interpret这一步只能做到判断CHAR或NOTCHAR
	unsigned int length;		//值长度
	struct struct_insertvalue *next;	//下一个值
}insertvalue;
