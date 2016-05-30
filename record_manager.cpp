#define _CRT_SECURE_NO_WARNINGS    
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer_manager.h"
#include "record_manager.h"

using namespace std;

short int CRecordManager::ValueCompare(char *value, unsigned int type, unsigned int cond, char *compvalue)
{
	long intvalue;
	double floatvalue;
	
	switch(type)
	{
	case INT:
		{
			intvalue = atoi(value);
			switch(cond)
			{
			case LT:
				if(intvalue < atoi(compvalue))
					return 1;
				break;
			case LE:
				if(intvalue <= atoi(compvalue))
					return 1;
				break;
			case GT:
				if(intvalue > atoi(compvalue))
					return 1;
				break;
			case GE:
				if(intvalue >= atoi(compvalue))
					return 1;
				break;
			case EQ:
				if(intvalue == atoi(compvalue))
					return 1;
				break;
			case NE:
				if(intvalue != atoi(compvalue))
					return 1;
				break;
			}
		}
		break;
	case FLOAT:
		{
			floatvalue = atof(value);
			switch(cond)
			{
			case LT:
				if(floatvalue < atof(compvalue))
					return 1;
				break;
			case LE:
				if(floatvalue <= atof(compvalue))
					return 1;
				break;
			case GT:
				if(floatvalue > atof(compvalue))
					return 1;
				break;
			case GE:
				if(floatvalue >= atof(compvalue))
					return 1;
				break;
			case EQ:
				if(floatvalue == atof(compvalue))
					return 1;
				break;
			case NE:
				if(floatvalue != atof(compvalue))
					return 1;
				break;
			}
		}
		break;
	case CHAR:
		{
			switch(cond)
			{
			case LT:
				if(strcmp(value, compvalue) < 0)
					return 1;
				break;
			case LE:
				if(strcmp(value, compvalue) <= 0)
					return 1;
				break;
			case GT:
				if(strcmp(value, compvalue) > 0)
					return 1;
				break;
			case GE:
				if(strcmp(value, compvalue) >= 0)
					return 1;
				break;
			case EQ:
				if(strcmp(value, compvalue) == 0)
					return 1;
				break;
			case NE:
				if(strcmp(value, compvalue) != 0)
					return 1;
				break;
			}
		}
		break;
	}
	return 0;

}

short int CRecordManager::SelectRecord(condition *conds, column *cols, unsigned int recordlen)//, unsigned int recordnum)
{
	int *intp;
	int mapnum, tabnum;				//map文件和tab文件块数
	int mapblocknum, tabblocknum;	//map文件何tab文件在内存中的块号
	int mapoffset = 0, taboffset = 0;//每次都从块的第一条开始读
	int record_perblock;//每个块中含记录数量
	string tabfilename, mapfilename;
	char value[VALUE_LENGTH];
	int i;
	column *tempcol = cols;
	condition *tempcond;
	int conform = 1;
	unsigned int printnum = 0; //已打印记录的计数器

	tabfilename = m_tablename + ".tab"; 
	mapfilename = m_tablename + ".map";


	mapblocknum = get_block(MAP, mapfilename, 0);//map文件的第0块存储该文件的总块数
	intp = (int *)m_ptheblocks[mapblocknum].m_address;
	mapnum = *intp;                              //map文件的总块数
	m_ptheblocks[mapblocknum].used_block();

	tabblocknum = get_block(TABLE, tabfilename, 0);//table文件的第0块存储该文件的总块数
	intp = (int *)m_ptheblocks[tabblocknum].m_address;
	tabnum = *intp;								//table文件的总块数
	record_perblock = BLOCK_SIZE / recordlen;   //块大小除以记录长度等于每块包含的记录数量
	m_ptheblocks[tabblocknum].used_block();

	//map文件不管有没有，先读第一块
	mapblocknum = get_block(MAP, mapfilename, 1);//map文件从第一块开始，记录了对应的每条记录是否被删除

	while(tempcol)
	{
		int remain = tempcol -> collength - strlen(tempcol -> colname);
		cout<<tempcol -> colname;
		for(; remain>0 ; remain --)
			cout<<" ";                  //为每一列的实际数据预留空间，打印时能够对齐
		tempcol = tempcol -> next;
	}
	cout<<endl;
	for(i = 1; i < tabnum; i++)                     //tab文件块循坏，从第一个块（存储了实际记录的块）开始
	{
		tabblocknum = get_block(TABLE, tabfilename, i);
		for(taboffset = 0;taboffset < record_perblock ; taboffset ++)	//tab一个块内offset循环,offset从0开始
		{
			//检查相应的.map
			if(*(m_ptheblocks[mapblocknum].m_address + mapoffset) =='1')//相应的.map表示该条记录存在未被删除
			{	
				tempcond = conds;
				conform = 1;
				//if(!tempcond)   //如果没有筛选条件，直接开始逐条打印
				//	goto print;
				while(tempcond)
				{
					strcpy(value, m_ptheblocks[tabblocknum].m_address + taboffset * recordlen + tempcond -> attroffset);
					if(!ValueCompare(value, tempcond -> type, tempcond -> cond, tempcond -> value))
						conform = 0;//只要有一个条件不符合，conform就置为0，不能打印
					tempcond = tempcond -> next;
				}
				//符合条件，开始打印
				print:
				if(conform)
				{
					tempcol = cols;
					printnum ++;
					while(tempcol)
					{
						int remain;
						strcpy(value, m_ptheblocks[tabblocknum].m_address + taboffset * recordlen + tempcol -> coloffset);
						remain = tempcol -> collength - strlen(value);
						cout<<value;
						for(; remain>0 ; remain --)
							cout<<" ";
						tempcol = tempcol -> next;
					}
					cout<<endl;
				}
			}
			//map offset加1，若满BLOCK_SIZE则读一新块。
			mapoffset ++;
			if(mapoffset >= BLOCK_SIZE)
			{
				mapoffset = 0;
				mapblocknum ++;
				if(mapblocknum > mapnum)
					return 1;
				mapblocknum = get_block(MAP, mapfilename, mapblocknum);
			}				
		}	
	}
	cout<<printnum<<" record(s) are selected from table "<<m_tablename<<endl;
	return 1;	
} 


short int CRecordManager::DeleteRecord(condition *conds, unsigned int recordlen,unsigned int recordnum)
{
	int *intp;
	int mapnum, tabnum;				//map文件和tab文件块数
	int mapblocknum, tabblocknum;	//map文件何tab文件在内存中的块号
	int mapoffset = 0, taboffset = 0;
	int record_perblock;
	string tabfilename, mapfilename;
	char value[VALUE_LENGTH];
	int i;
	unsigned int j;
	condition *tempcond;
	int conform = 1;
	short int deleted = 0;
	char *ptr;
	
	tabfilename = m_tablename + ".tab"; 
	mapfilename = m_tablename + ".map";

	//读map文件
	mapblocknum = get_block(MAP, mapfilename, 0);
	intp = (int *)m_ptheblocks[mapblocknum].m_address;
	mapnum = *intp;
	m_ptheblocks[mapblocknum].used_block();

	//读tab文件
	tabblocknum = get_block(TABLE, tabfilename, 0);
	intp = (int *)m_ptheblocks[tabblocknum].m_address;
	tabnum = *intp;
	record_perblock = BLOCK_SIZE / recordlen;
	m_ptheblocks[tabblocknum].used_block();

	//map文件不管有没有，先读第一块
	mapblocknum = get_block(MAP, mapfilename, 1);

	for(i = 1; i < tabnum; i++)                     //tab文件块循坏
	{
		tabblocknum = get_block(TABLE, tabfilename, i);
		for(taboffset = 0;taboffset < record_perblock ; taboffset ++)	//tab一个块内offset循环
		{
			//检查相应的.map
			if(*(m_ptheblocks[mapblocknum].m_address + mapoffset) =='1')
			{	
				tempcond = conds;
				conform = 1;
				//if(!tempcond) //如果没有条件，直接逐条删除
				//	goto todelete;
				while(tempcond)
				{
					strcpy(value, m_ptheblocks[tabblocknum].m_address + taboffset * recordlen + tempcond -> attroffset);
					if(!ValueCompare(value, tempcond -> type, tempcond -> cond, tempcond -> value))
						conform = 0;
					tempcond = tempcond -> next;
				}
				//符合条件，开始删
				todelete:
				if(conform)
				{
					*(m_ptheblocks[mapblocknum].m_address + mapoffset) ='$';
					deleted++;
					ptr = m_ptheblocks[tabblocknum].m_address + taboffset * recordlen;
					for(j = 0 ; j < recordlen ; j++)
						*(ptr + j) = '$';					
				}
			}
			//map offset加1，若满BLOCK_SIZE则读一新块。
			mapoffset ++;
			if(mapoffset >= BLOCK_SIZE)
			{
				mapoffset = 0;
				mapblocknum ++;
				if(mapblocknum > mapnum)
					return deleted;
				mapblocknum = get_block(MAP, mapfilename, mapblocknum);
			}				
		}	
	}
	return deleted;	
	
}

short int CRecordManager::IsValueExists(column *cols, char *value,unsigned int recordlen)		//是否应改传个recordlen给我
{
	int number,iter,totalblock;
	int i,find= 0;
	char *p,*temp; 
	int *intp;
	string tablefilename;
	int  recordpblk=BLOCK_SIZE/recordlen;

	tablefilename = m_tablename + ".tab"; 
	
	number=get_block(TABLE,tablefilename,0);
    p=m_ptheblocks[number].m_address;
	intp=(int*)p;
    totalblock=*intp;
	m_ptheblocks[number].used_block();
	for(iter = 1;iter<totalblock;iter++)
	{
	   number = get_block(TABLE,tablefilename,iter);
	   p=m_ptheblocks[number].m_address;
	   for(i = 0; i < recordpblk; i++)
		{	
			temp=p+recordlen*i+cols->coloffset;
			if(strcmp(temp,value)==0)
			{	
	    	m_ptheblocks[number].used_block();
			return 1;
			}
		}
	    m_ptheblocks[number].used_block();
	}
   return 0;
}

//插入一条记录。先在。Map文件中找第一个为0的位，然后就把values链表的value部分一项一项写到
//相应记录文件中去。若没有，则分别在。Map文件尾追加一个1，在记录文件尾追加一条记录。要返回所加
//记录的条目号。
//record和map文件在初始化时必须在BLOCK0中存上当前有多少个BLOCK
unsigned int CRecordManager::InsertValues(insertvalue *values, unsigned int recordlen)
{
	//返回记录的条目号
	int map_block_num,tab_block_num;
	int map_total_block,tab_total_block; //map文件和table文件的块总数，便于判断之前有无记录
	insertvalue *current_value;
	char *p,*temp,*point;
	int *intp;
	int i;
	string tablefilename,mapfilename;
	int record_per_block=BLOCK_SIZE/recordlen;//每个块中包含的记录总数
	int block_no,offset_in_block,record_number;


	tablefilename = m_tablename + ".tab"; 
	mapfilename = m_tablename + ".map";
		
  	tab_block_num=get_block(TABLE,tablefilename,0);
	intp=(int*)m_ptheblocks[tab_block_num].m_address;
	if (*m_ptheblocks[tab_block_num].m_address =='$')
		*intp = 1;
	tab_total_block=*intp;//获得table文件的总块数
	m_ptheblocks[tab_block_num].used_block();
	//
	map_block_num=get_block(MAP,mapfilename,0);
	/*if(map_block_num==-1)
	{
		printf("memory allocation error\n");
		return 0;
	} 
	*/
	intp=(int*)m_ptheblocks[map_block_num].m_address;
	if (*m_ptheblocks[map_block_num].m_address == '$')
		*intp = 1;
    map_total_block=*intp;//读出当前MAP共有几个块，
	if(map_total_block==1)//如果只有第0块，说明之前不存在任何记录
	{
		*intp=2; //map总块数置为2，新增一个存放记录的块
		
		m_ptheblocks[map_block_num].written_block();
		m_ptheblocks[map_block_num].used_block();  //第0块已被修改
		map_block_num=get_blank_block(MAP);
		m_ptheblocks[map_block_num].mark_block(mapfilename,1);
		p=m_ptheblocks[map_block_num].m_address;

		*p='1';//从BLOCK头放起
		m_ptheblocks[map_block_num].written_block();
		m_ptheblocks[map_block_num].used_block();
  
		tab_block_num=get_blank_block(TABLE);
		m_ptheblocks[tab_block_num].mark_block(tablefilename,1);
		p=m_ptheblocks[tab_block_num].m_address;
	    //intp=(int*)p;
		//*intp=1;//该块的号码
		//p=m_ptheblocks[tab_block_num].m_address+recordlen*2;
		for(current_value=values;current_value!=NULL;current_value=current_value->next)
		{
		   strcpy(p,current_value->value);//将记录中的所有value写入块
		   p=p+current_value->length;
		}
	
		m_ptheblocks[tab_block_num].written_block();
		m_ptheblocks[tab_block_num].used_block();
		//
		tab_block_num=get_block(TABLE,tablefilename,0);
		intp=(int*)m_ptheblocks[tab_block_num].m_address;
		*intp=2;
	
		m_ptheblocks[tab_block_num].written_block();
		m_ptheblocks[tab_block_num].used_block();
		return 1;//返回所加记录的条目号
	}
	else     //之前已经存在其他记录
	{
		int map_space = 0;
		m_ptheblocks[map_block_num].used_block();//不再使用map文件的第0块
        for(i=1;i<map_total_block;i++)
		{
           map_block_num=get_block(MAP,mapfilename,i);
           /*if(map_block_num==-1)
		   {
		       printf("memory allocation error\n");
		       return 0;
		   }
		   */
		   p=m_ptheblocks[map_block_num].m_address;
		   for(temp=p;temp<p+BLOCK_SIZE;temp++)
			   if(*temp=='$') //找到一条已被懒惰删除的记录
			   {    
				   *temp='1'; //置为1，说明该记录存在
				   m_ptheblocks[map_block_num].written_block();
				   m_ptheblocks[map_block_num].used_block();//map文件修改完毕
				   record_number=(i-1)*BLOCK_SIZE+temp-p+1;//从1开始算起，获得新增记录的条目号
				   map_space=1;
				   break;
			   }
		  if(map_space) break;
          m_ptheblocks[map_block_num].used_block();
		}
		if(!map_space){//若到这里表示所有的MAP已经都满了
	        map_block_num=get_blank_block(MAP);
			m_ptheblocks[map_block_num].mark_block(mapfilename,map_total_block); 
			p=m_ptheblocks[map_block_num].m_address;

			temp=p;
			*temp='1';////从BLOCK头放起
			m_ptheblocks[map_block_num].written_block();
			m_ptheblocks[map_block_num].used_block();
	       
			map_total_block++;
			map_block_num=get_block(MAP,mapfilename,0);	    
		    intp=(int*)m_ptheblocks[map_block_num].m_address;
			*intp=map_total_block;
			m_ptheblocks[map_block_num].written_block();
			m_ptheblocks[map_block_num].used_block();
			record_number=(map_total_block-2)*BLOCK_SIZE+1;
	    }
		
		block_no=record_number/record_per_block+1;
        offset_in_block=record_number-record_per_block*(block_no-1);
		/////
	    if(block_no>=tab_total_block)//需要为table文件新开一个块
	    {
		    char *t1; int *it1;
		    tab_block_num=get_block(TABLE,tablefilename,0); //table文件中第0块存储的总块数加1
            it1=(int*)m_ptheblocks[tab_block_num].m_address;
		    (*it1)++;
		    m_ptheblocks[tab_block_num].written_block();
		    m_ptheblocks[tab_block_num].used_block();
		    ////
		    tab_block_num=get_blank_block(TABLE);
		    m_ptheblocks[tab_block_num].mark_block(tablefilename,tab_total_block);//tab_total_block不需要加1，因为块编号从0开始
		    tab_total_block++;
		    //it1=(int*)m_ptheblocks[tab_block_num].m_address;
		    //*it1=m_ptheblocks[tab_block_num].m_offset_number;
	    }
        else 
	    {
		    tab_block_num=get_block(TABLE,tablefilename,block_no);
	    }
        point=m_ptheblocks[tab_block_num].m_address+recordlen*(offset_in_block-1);
        for(current_value=values;current_value!=NULL;current_value=current_value->next)
	    {
            strcpy(point,current_value->value); //将新增记录写入相应的块
            point=point+current_value->length;
	    }
     	
		m_ptheblocks[tab_block_num].written_block();
    	m_ptheblocks[tab_block_num].used_block();
		return record_number;//返回所加记录的条目号
        
	}
	return 0;
}
/*只测试了增加记录和查找记录功能
int main(){
	
	CRecordManager A("student");
	CBufferManager::initiate_blocks();
	insertvalue a1,a2,b1,b2;
     //insert values(10��'amy')and(12��'john') into table student
	strcpy(a1.value,"10");
	strcpy(a2.value,"amy");
	strcpy(b1.value,"12");
	strcpy(b2.value,"john");
	a1.type=INT;
	a2.type=CHAR;
	b1.type=INT;
	b2.type=CHAR;
	a1.length=3;
	a2.length=5;
	b1.length=3;
	b2.length=5;
	a1.next = &a2;
	a2.next = NULL;
	b1.next = &b2;
	b2.next = NULL;	
	//print record number 
	cout<<A.InsertValues(&a1,8)<<endl;
	cout<<A.InsertValues(&b1,8)<<endl;
	column std_id,name;
	strcpy(std_id.colname,"std_id");
	strcpy(name.colname,"name");
	std_id.type = INT;
	name.type = CHAR;
	std_id.coloffset=0;
	std_id.collength=3;
	name.coloffset=3;
	name.collength =5;
	std_id.next = &name;
	name.next = NULL;
	A.SelectRecord(NULL,&std_id,8);
	
	return 0;
}
*/
