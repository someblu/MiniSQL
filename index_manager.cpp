

#include "index_manager.h"

using namespace std;

int CIndexManager::keycompare(char *a,char *b, int tag)//a>=b 返回1
{   
    char *cha; char *chb; char tempa,tempb;
	int numa,numb;
    double floata,floatb;

	switch(tag)
	{
	case CHAR:
		   cha=a;
		   chb=b;
		   for(;*cha!='\0'&&(*chb!='\0');cha++,chb++)
		   {    
			   tempa=*cha; tempb=*chb;
			   if(tempa>='A'&&tempa<='Z')   //忽略大小写，全部当做小写字母
				   tempa=tempa+32;
			   if(tempb>='A'&&tempb<='Z')
				   tempb=tempb+32;
			   if(tempa>tempb)
				   return 1;
			   if(tempa<tempb)
				   return 0;
		   }
		   if(*cha=='\0'&&*chb=='\0')
			   return 1;//相等
		   else if( *cha!='\0'&&*chb=='\0')
			   return 1;//a>b;
		   else return 0;

           break;
	case INT:
		   numa=atoi(a);  //转化成整型
		   numb=atoi(b);
		   if(numa>=numb)  return 1;
		   else return 0;
		   break;
	case FLOAT:
		   floata=atof(a);
		   floatb=atof(b);  //转化成float类型
		   if(floata>=floatb) return 1;
		   else return 0;
		   break;
	}
	return -1;
}

int CIndexManager::quick_insert(char *value,int tag,int blocknumber,int offsetinblock)//tag为比较方式
{
	unit_node *currentnode,*save_pointer;
	unit_node *newnode = new unit_node;
	newnode->block_number=blocknumber;
	newnode->offset_in_block=offsetinblock;
	strcpy(newnode->key,value);
	if(this->m_count>=31)   //如果该块已经存储了31条记录，已满
	{
		printf("mistake in using this function\n");
		return 0;
	}
	else
	{
		currentnode=this->m_first;
		while(currentnode!=NULL&&keycompare(newnode->key,currentnode->key,tag)==1)//遍历链表，找到新节点插入的位置
			save_pointer=currentnode;
			currentnode=currentnode->next;
	}
	save_pointer->next=newnode;
	newnode->next=currentnode;
	this->m_count++;
	return 1;
}

int CIndexManager::insert_entry(char *value,int tag,int blocknumber,int offsetinblock,int total_idx_block)  //返回index文件的总块数
{
	CIndexManager *currenthead=this,*newhead,*head_save=currenthead;
	unit_node *newnode;

    for(;currenthead!=NULL && keycompare(value,currenthead->m_minkey,tag)==1;currenthead=currenthead->m_next)
		head_save=currenthead;
	if(head_save!=currenthead && head_save->m_count<31)
	{
		if(head_save->quick_insert(value,tag,blocknumber,offsetinblock)==1)
			return total_idx_block;
		else return 0;
	}
	else if(head_save!=currenthead)   //链表的中间需要加块
	{
		total_idx_block++;
        newhead = new CIndexManager;
	    newnode = new unit_node;
	    newhead->m_count=1;
		
		strcpy(newhead->m_minkey,value);
		newhead->m_first=newnode;
		newhead->m_next=currenthead;
		head_save->m_next=newhead;

		newhead->m_blocknumber = total_idx_block-1; //为新块安排块号
		head_save->m_next_block_number = total_idx_block-1;
		if(currenthead!=NULL)        //插入的是中间节点不是最后的节点
			newhead->m_next_block_number = currenthead->m_blocknumber;
		else
			newhead->m_next_block_number = -1;   //如果新块是最后一块，记录的下一个块号为-1；

		newnode->next=NULL;
		strcpy(newnode->key,value);
		newnode->block_number=blocknumber;
		newnode->offset_in_block=offsetinblock;
		return total_idx_block;
	}
	else{
		total_idx_block++;				//最头上需要新增加一个节点
		newhead = new CIndexManager;
	    newnode = new unit_node;
		//因为this指针不能改变，因此新建一个节点后，将this的数据和新建的节点交换
		if(currenthead->m_count>0){                                       //链表不完全为空
			strcpy(newhead->m_minkey,currenthead->m_minkey);
			newhead->m_first=currenthead->m_first;
			newhead->m_next=currenthead->m_next;
			newhead->m_blocknumber = currenthead->m_blocknumber; 
			newhead->m_next_block_number = currenthead ->m_next_block_number;
			newhead->m_count = currenthead->m_count;
			currenthead->m_next = newhead;
			currenthead->m_first = newnode;
			currenthead->m_blocknumber = total_idx_block-1;
			currenthead->m_next_block_number = newhead->m_blocknumber;
			currenthead->m_count = 1;
		}
		else{
			currenthead->m_blocknumber = total_idx_block-1; //之前的链表
			currenthead->m_next_block_number = -1;
			currenthead->m_count = 1;
			currenthead->m_first = newnode;
			currenthead->m_next = NULL;
			strcpy(currenthead->m_minkey,value);
		}

		newnode->next=NULL;
		strcpy(newnode->key,value);
		newnode->block_number=blocknumber;
		newnode->offset_in_block=offsetinblock;
		return total_idx_block;
	}

	return 0;
}

int CIndexManager::CreateIndex(column *cols, unsigned int recordlen)
{
	int table_blocknum=0;//记录当前是TABLE 第几块BLOCK
	int index_blocknum=0;//记录当前是INDEX 第几块BLOCK
	int map_blocknum=0;
	unsigned int current_record=1;//当前第几条记录
	string this_table,this_index;
	int currentblock;
	int table_total_block;
	int offset_in_block;
	int records_per_block=BLOCK_SIZE/recordlen;
	insertvalue value; //插入一个属性
	int record_number;	
	char *p,*temp;
	int *intp;

	
	this_table = m_tablename + ".tab"; 
	this_index = m_indexname + ".idx";
   
    currentblock=get_block(INDEX, this_index,0);//准备整个index的第0块，该快不被放在树中
	p=m_ptheblocks[currentblock].m_address;
	intp=(int*)p;
	*intp=1;
	intp++;
	*intp=-1;//下一块号
	p=p+8;
	strcpy(p,cols->colname);

	m_ptheblocks[currentblock].written_block();
	m_ptheblocks[currentblock].used_block();
	
	currentblock=get_block(TABLE,this_table,0);
	/*
	if(currentblock==-1)
	{
		printf("can not find the proper table for index\n");
		return 0;
	}*/
	p=m_ptheblocks[currentblock].m_address;
	intp=(int*)p;
    table_total_block=*intp;
	m_ptheblocks[currentblock].used_block();
	////
	for(table_blocknum=1;table_blocknum<=table_total_block;table_blocknum++)//从table文件中的第1块开始读起，因为第0块存了总块数
	{ 
		 currentblock=get_block(TABLE,this_table,table_blocknum);
		 p=m_ptheblocks[currentblock].m_address;
		 for(;p<m_ptheblocks[currentblock].m_address+BLOCK_SIZE;p=p+recordlen)//逐条读取记录
		 {
			if(*p!='$')                                                      //记录存在
			{

				offset_in_block=(p-m_ptheblocks[currentblock].m_address)/recordlen; //获得该记录的相对于块的偏移地址
				record_number=records_per_block*(m_ptheblocks[currentblock].m_offset_number-1)+offset_in_block;
				//建立索引值
				temp=p+cols->coloffset;
				value.length=cols->collength;
				value.next=NULL;
				value.type=cols->type;
				strcpy(value.value,temp);
                
				if(InsertIndex(&value, record_number,recordlen)==0)
				{
					printf("Error when insert value %s into index",value.value);
					return 0;
				}
				
			}
		 }
		 m_ptheblocks[currentblock].used_block();;

	}
     printf("The index on attribute '%s' of table '%s' has been created successfully\n", cols->colname,m_tablename);
	 return 1;
}

int CIndexManager::save_index()
{
	CIndexManager *currenthead=this;
	unit_node *currentnode;
	int current_block;
	int block_count=0;
	char *p; int *intp;
	string this_index = m_indexname + ".idx";

	for(;currenthead!=NULL;currenthead=currenthead->m_next) {     //从第一个节点开始写回
        block_count++; 
		current_block=get_block(INDEX,this_index,block_count);
		if(current_block==-1) {
			printf("can't allocate memory while saving\n");
			return 0;
		}
		p=m_ptheblocks[current_block].m_address;
		for(;p<m_ptheblocks[current_block].m_address+BLOCK_SIZE;p++)
			*p='$';
		p=m_ptheblocks[current_block].m_address;
		intp=(int*)p;
		*intp=currenthead->m_blocknumber;      //每个块头都写入块号，记录总数和下一个块号
		intp++;
		*intp=currenthead->m_count;
		intp++;
		*intp=currenthead->m_next_block_number;
		p=p+12;
		strcpy(p,currenthead->m_minkey);
		//把索引值和记录地址逐条写回块
		p=m_ptheblocks[current_block].m_address;
		for(currentnode=currenthead->m_first;currentnode!=NULL;currentnode=currentnode->next) {
			p=p+INDEX_RECORD_LEN;
			intp=(int*)p;
			*intp=currentnode->block_number;
			intp++;
			*intp=currentnode->offset_in_block;
			p=p+8;
			strcpy(p,currentnode->key);
		}

        m_ptheblocks[current_block].written_block();
		m_ptheblocks[current_block].used_block();

	}
	
	return 1;
}

int CIndexManager::build_index(){
	CIndexManager *currenthead=this,*newhead;
	unit_node *currentnode,*newnode;
	int current_block;
	int block_no;
	char *p; int *intp;

	string this_index = m_indexname + ".idx";

	current_block=get_block(INDEX, this_index,0);//读取
	p=m_ptheblocks[current_block].m_address;
	intp=(int*)p;
	int total_idxblock_number = *(intp);
    if(total_idxblock_number==1)   //如果这是一个没有记录的index，为了插入记录时方便判断，将this->m_count设置为0
    	this->m_count = 0;
	intp++;
	block_no = *(intp);
	m_ptheblocks[current_block].used_block();
	//...........
	while(block_no!=-1) {     //从第一个块开始读取
		current_block=get_block(INDEX,this_index,block_no);
		p=m_ptheblocks[current_block].m_address;
		intp=(int*)p;
		currenthead->m_blocknumber = *intp;      //每个块头都写入块号，记录总数和下一个块号
		intp++;
		currenthead->m_count = *intp;
		intp++;
		currenthead->m_next_block_number = *intp;
		p=p+12;
		strcpy(currenthead->m_minkey,p);

		//给下一个块和块的第一条记录分配空间
		newhead = new CIndexManager;
		currenthead->m_next = newhead;
		currentnode = new unit_node;
		currenthead ->m_first = currentnode;

		//逐条读取索引值和记录地址
		p=m_ptheblocks[current_block].m_address+INDEX_RECORD_LEN;
		for(;p<m_ptheblocks[current_block].m_address+BLOCK_SIZE;p=p+INDEX_RECORD_LEN) {
			if(*p!='$'){				
				intp=(int*)p;
				currentnode->block_number = *intp;
				intp++;
				currentnode->offset_in_block = *intp;
				p=p+8;
				strcpy(currentnode->key,p);
				newnode = new unit_node;
				currentnode->next = newnode;
			}
		}
		//
		delete newnode;
		currentnode->next = NULL;    //删除最后一个没有索引键的节点
        m_ptheblocks[current_block].written_block();
		m_ptheblocks[current_block].used_block();
		//.........
		if((block_count+1)<total_idxblock_number)
			currenthead = currenthead->m_next;
		else{
			delete newnode;                   //删除最后一个没有存数据的index manager
			currenthead->m_next = NULL;
		}

	}
	
	return 1;

}

void CIndexManager::freespace()           //清空用来暂时存储数据的b+树
{
	CIndexManager *current_head1,*current_head2;
	unit_node *current_node1,*current_node2;
	for(current_head1=this;current_head1!=NULL;)
	{
		for(current_node1=current_head1->m_first;current_node1!=NULL;)
		{
            current_node2=current_node1->next;
			delete current_node1;
			current_node1=current_node2;
		}
		current_head2=current_head1->m_next;
		delete current_head1;
		current_head1=current_head2;
	}

}

int CIndexManager::SelectIndex(condition *conds, column *cols, unsigned int recordlen)
{
	CString this_table,this_index;
    int blocknumber,block_save,nextblock_number,recordblock;
	char *p,*min_key,*key,*attr_p;
	char *recordpoint;
	int  *intp;
	int destine_blocknum,destine_offset;
	column *col=cols;

	this_index = m_indexname + ".idx";
	this_table = m_tablename + ".tab";
	
    if(conds->cond!=EQ)    //index查找只允许等值查找
	{
		printf("compare condition error\n");
		return 0;
	}
	//查找时不会改变文件结构，因此不需要建立抽象的b+树
	blocknumber=get_block(INDEX,this_index,0);
	if(blocknumber==-1)
	{
		printf("memory access error");
		return 0;
	}
	p=m_ptheblocks[blocknumber].m_address;
	intp=(int*)p;
	if(*intp == 1){
		cout<<"no record existing!"<<endl;
		return 0;
	}
	intp++;
    nextblock_number=*intp;
	m_ptheblocks[blocknumber].used_block();
	 
	blocknumber=get_block(INDEX,this_index,nextblock_number);
	p=m_ptheblocks[blocknumber].m_address;
	intp=(int*)(p+8);
	nextblock_number=*intp;
	min_key=p+12;                   //获得该节点中最小的值
	while(keycompare(conds->value,min_key,conds->type)==1)
	 {
		 block_save=m_ptheblocks[blocknumber].m_offset_number;
		 m_ptheblocks[blocknumber].used_block();
		 if(nextblock_number==-1)//到了最后一块
			 break;
		 blocknumber=get_block(INDEX,this_index,nextblock_number);
		 p=m_ptheblocks[blocknumber].m_address;
	     intp=(int*)(p+8);
         nextblock_number=*intp;
	     min_key=p+12;
	}
	m_ptheblocks[blocknumber].used_block();
	blocknumber=get_block(INDEX,this_index,block_save);//找到目标块
	p=m_ptheblocks[blocknumber].m_address+INDEX_RECORD_LEN;

	while(col)                                       //打印各个属性的名称
	{
		int remain = col->collength - strlen(col->colname);
		printf("%s", col->colname);
		for(; remain ; remain --)
			printf(" ");
		col = col->next;
	 }
	printf("\n");
    while(p<m_ptheblocks[blocknumber].m_address+BLOCK_SIZE)
	 {
		key=p+8;
		if(strcmp(key,conds->value)==0)
		{
			intp=(int*)p;
			destine_blocknum=*intp;
			intp++;
			destine_offset=*intp;
            recordblock=get_block(TABLE,this_table,destine_blocknum);
			recordpoint=m_ptheblocks[recordblock].m_address+recordlen*(destine_offset-1);
			 
			col = cols;
			while(col)
			{
				int remain;
				char value[255];
				strcpy(value, attr_p=recordpoint+col->coloffset);
				remain = col->collength - strlen(attr_p);
				printf("%s", value);
				for(; remain ; remain --)
					printf(" ");
				col = col->next;
			}
			printf("\n");
			printf("1 record(s) are selected from table '%s'!\n", m_tablename);

			m_ptheblocks[recordblock].used_block();
			 return 1;
		}
		p=p+INDEX_RECORD_LEN;
	}
	printf("can not find this record by index\n");
	return 0; 
}

int CIndexManager::DeleteIndex(condition *conds,unsigned int recordlen)
{
	string  this_table,this_index,this_map;
    int blocknumber,block_save,nextblock_number,recordblock,mapblock;
	char *p,*min_key,*key;
	char *record_point;
	int  *intp;
	int destine_blocknum,destine_offset;
	char *temp;
	int records_per_block=BLOCK_SIZE/recordlen;

	this_index = m_indexname + ".idx";
	this_table = m_tablename + ".tab";
	this_map = m_tablename + ".map";

    if(conds->cond!=EQ)
	{
		printf("compare condition error\n");
		return 0;
	}
	build_index();
	CIndexManager *currenthead=this,*newhead,*head_save=currenthead, *pre_head = head_save;
	unit_node *currentnode,*node_save=currentnode;

    for(;currenthead!=NULL && keycompare(value,currenthead->m_minkey,tag)==1;currenthead=currenthead->m_next){
    	pre_head = head_save;
		head_save=currenthead;
    }	
	currentnode = head_save->m_first;
	for(currentnode!=NULL;currentnode=currentnode->next){
		if(strcmp(conds->value,currentnode->value)==1 ){
			head_save->m_count--;   //总索引数减一
			//删除record和map文件中对应的记录
			recordblock = get_block(TABLE,this_table,currentnode->block_number);
			p = m_ptheblocks[recordblock].m_address+recordlen*(currentnode->offset_in_block-1);
			for(;p<m_ptheblocks[recordblock].m_address+recordlen*(currentnode->offset_in_block);p++)
				*p='$';
			int recordnum = (currentnode->block_number-1) * records_per_block+currentnode->offset_in_block;
			int map_blocknum = recordnum/BLOCK_SIZE + 1;
			int map_offset = recordnum % BLOCK_SIZE;
			mapblock = get_block(MAP,this_map,map_blocknum);
			p=m_ptheblocks[mapblock].m_address+map_offset-1;
			*p = '$';
			//.......
			if(node_save!=currentnode){     //删除的是中间节点或最后的节点
				node_save->next = currentnode ->next;
				delete currentnode;
				save_index();
				freespace();
				return 1;
			}
			else if(currentnode->next!=NULL){           //删除的是第一个的索引键，但节点中还有其他的索引键
				head_save->m_first = currentnode->next;
				delete currentnode;
				save_index();
				freespace();
				return 1;
			}
			else{						//删除的是节点中最后一个索引值，需要整个节点一起删除
				blocknumber = get_block(INDEX,this_index,0);
				p=m_ptheblocks[blocknumber].m_address;
	            intp=(int*)p;
	            int last_blk_num = *intp-1;
	            *intp--;
	            m_ptheblocks[blocknumber].written_block();
	            m_ptheblocks[block_count].used_block();      //块的编号是否需要调整？这里没有调整

				if(pre_head!=currenthead){
					pre_head->m_next = currenthead;
					delete head_save;
					save_index();
					freespace();
					return 1;
				}
				else{
					this->m_blocknumber=currenthead->m_blocknumber;
					this->m_count = currenthead->m_count;        
					this->m_next = currenthead->m_next;
					this->m_first = currenthead->m_first;   
					strcpy(this->m_minkey,currenthead->value);
					this->m_next_block_number=currenthead->m_next_block_number; 
					delete currenthead;
					save_index();
					freespace();
					return 1;
				}
			}							
		}
		node_save = currentnode;
	}
	//原来的代码。。。。。。。。。。
	/*
	 blocknumber=get_block( INDEX,this_index,0);
	 if(blocknumber==-1)
	 {
		 printf("memory access error");
		 return 0;
	 }
	 m_ptheblocks[blocknumber].m_being_used=1;
	 p=m_ptheblocks[blocknumber].m_address;
	 intp=(int*)(p+4);
     nextblock_number=*intp;
	 m_ptheblocks[blocknumber].used_block();
     blocknumber=get_block(INDEX,this_index,nextblock_number);
     m_ptheblocks[blocknumber].m_being_used=1;
	 p=m_ptheblocks[blocknumber].m_address;
	 intp=(int*)(p+8);
	 nextblock_number=*intp;
	 min_key=p+12;
	 while(keycompare(conds->value,min_key,conds->type)==1)
	 {
		 block_save=m_ptheblocks[blocknumber].m_offset_number;
		 m_ptheblocks[blocknumber].used_block();
         if(nextblock_number==-1)//到了最后一块
			 break;
		 blocknumber=get_block(INDEX,this_index,nextblock_number);
         m_ptheblocks[blocknumber].m_being_used=1;
		 p=m_ptheblocks[blocknumber].m_address;
	     intp=(int*)(p+8);
         nextblock_number=*intp;
	     min_key=p+12;
	 }
	 m_ptheblocks[blocknumber].used_block();
	 blocknumber=get_block(INDEX,this_index,block_save);//找到目标块
	 m_ptheblocks[blocknumber].m_being_used=1;
	 p=m_ptheblocks[blocknumber].m_address+INDEX_RECORD_LEN;
     while(p<m_ptheblocks[blocknumber].m_address+BLOCK_SIZE)
	 {
		 key=p+8;
		 if(strcmp(key,conds->value)==0)//找到
		 {
			 int map_block_number,map_in_offset,record_num;
			 intp=(int*)p;
			 destine_blocknum=*intp;
			 intp++;
			 destine_offset=*intp;
			 record_num=records_per_block *(destine_blocknum-1)+destine_offset;
             recordblock=get_block( TABLE,this_table,destine_blocknum);
             m_ptheblocks[recordblock].m_being_used=1;
			 record_point=m_ptheblocks[recordblock].m_address+recordlen*(destine_offset+1);
			 //删除RECORD
             for(temp=record_point;temp<record_point+recordlen;temp++)
				 *temp='$';
			
			 m_ptheblocks[recordblock].written_block();
			 m_ptheblocks[recordblock].used_block();
			 //删除INDEX
             for(temp=p;temp<p+INDEX_RECORD_LEN;temp++)
			    *temp='$';
			 written_block();
			 m_ptheblocks[blocknumber].written_block();
			 m_ptheblocks[blocknumber].used_block();
			 //删除MAP
             map_block_number=record_num/BLOCK_SIZE+1;
			 map_in_offset=record_num-(map_block_number-1)*BLOCK_SIZE;
			 mapblock=get_block(MAP,this_index,map_block_number);
			 if(mapblock==-1)
			 {
		      printf("memory access error");
		      return 0;
			 }
			 m_ptheblocks[mapblock].m_being_used=1;
			 p=m_ptheblocks[mapblock].m_address;
			 temp=p+map_in_offset-1;//因为OFFSET从1算起
			 *p='$';
			 written_block();
			 m_ptheblocks[mapblock].written_block();
			 m_ptheblocks[mapblock].used_block();
			 return 1;
		 }
		 p=p+INDEX_RECORD_LEN;
	 }

	 printf("Can not find this record by index\n");
	 return 0;
	 */
}

int CIndexManager::InsertIndex(insertvalue *value, unsigned int recordnum, int recordlen)
{
	string this_table,this_index;
	int destine_blocknum,destine_offset;
	int blocknumber,nextblock,save_number=0,newblock,firstblock,first_block;
	char *p,*temp,*p1,*p2;
	int *intp,*intp1,*intp2;
	int newblocknumber;
	int total_block_number;
	int start_block=1;

	this_index = m_indexname + ".idx";
	
	int r_p_b = BLOCK_SIZE/recordlen;	
	destine_blocknum=recordnum/r_p_b+1;
	destine_offset=recordnum%r_p_b;		//在record文件中定位

	blocknumber=get_block(INDEX, this_index,0);//准备整个index的第0块，该快不被放在树中
	p=m_ptheblocks[blocknumber].m_address;
	intp=(int*)p;
	total_block_number = *(intp);

    build_index();
    total_block_number=insert_entry(value->value,value->type,destine_blocknum,destine_offset,total_block_number);
    *(intp) = total_block_number;
    intp++;											//如果有新增加页节点，index文件的第0块需要更新
    *(intp) = this->m_blocknumber;
    m_ptheblocks[blocknumber].written_block();
    m_ptheblocks[blocknumber].used_block();
    //
    save_index();
    freespace();
	/*原来的代码。。。。。。。。。
	blocknumber=get_block(INDEX,this_index,0);
	p=m_ptheblocks[blocknumber].m_address;
	intp=(int*)p;
	total_block_number=*intp;
	intp++;
	start_block=*intp;
	if(total_block_number==1)
	{
		intp--;
		(*intp)++;
		intp++;
		*intp=1;
		start_block=*intp;

		m_ptheblocks[blocknumber].written_block();
		m_ptheblocks[blocknumber].used_block();
		blocknumber=get_block(INDEX,this_index,1);
		p=m_ptheblocks[blocknumber].m_address;
		for(temp=p;temp<p+BLOCK_SIZE;temp++)
			*temp='$';
		intp=(int*)p;
		*intp=1;//HEAD中的块号
		intp++;
		*intp=1;//HEAD中的COUNT
		intp++;
		*intp=-1;//下一块的块号
		p=m_ptheblocks[blocknumber].m_address+12;//integer 类型是4个byte, 块头存储三个整数
		strcpy(p,value->value);
		//写NODE
		p=m_ptheblocks[blocknumber].m_address+INDEX_RECORD_LEN;
		intp=(int*)p;
		*intp=destine_blocknum;//记录叶节点的块号
		intp++;
        *intp=destine_offset;//记录叶节点的记录的偏移地址
		p=p+8;
		strcpy(p,value->value);
		
		m_ptheblocks[blocknumber].written_block();
		m_ptheblocks[blocknumber].used_block();
		return 1;
	}
	else
	{
	save_number=0;
	blocknumber=get_block( INDEX,this_index,start_block);
	if(blocknumber==-1)
	{
	    printf("memory access error");
	    return 0;
	}
	p=m_ptheblocks[blocknumber].m_address+12;//块号， count, 下一个块号
	while(keycompare(value->value,p,value->type)==1)
	{
		intp=(int*)m_ptheblocks[blocknumber].m_address;
		save_number=*intp;
		intp=intp+2;
		nextblock=*intp;
		m_ptheblocks[blocknumber].used_block();
		if(nextblock==-1)
		    break;
		blocknumber=get_block(INDEX,this_index,nextblock);
		if(blocknumber==-1)
		{
			printf("memory access error");
			return 0;
		}
		p=m_ptheblocks[blocknumber].m_address+12;
	}
	m_ptheblocks[blocknumber].used_block();
	if(save_number==0)
	{
		newblock=get_block(INDEX,this_index,total_block_number);
		first_block=get_block(INDEX,this_index,0);
		total_block_number++;
		p2=m_ptheblocks[first_block].m_address;
		intp2=(int*)p2;
		(*intp2)++; //第0块中的块总数加一
		intp2++;
		nextblock=*intp2;//记录之前的开始块号
		*intp2=m_ptheblocks[newblock].m_offset_number;

		m_ptheblocks[first_block].written_block();
		m_ptheblocks[first_block].used_block();
		//和写新快一样的步骤
		p1=m_ptheblocks[newblock].m_address;
		for(temp=p1;temp<p1+BLOCK_SIZE;temp++)
			*temp='$';
		//写新块的HEAD
		intp1=(int*)p1;
		*intp1=m_ptheblocks[newblock].m_offset_number;//HEAD中的块号
		intp1++;
		*intp1=1;//HEAD中的COUNT
		intp1++;
		*intp1=nextblock;//下一块的块号????????????
		p1=m_ptheblocks[newblock].m_address+12;
		strcpy(p1,value->value);
		//写NODE
		p1=m_ptheblocks[newblock].m_address+INDEX_RECORD_LEN;
		intp1=(int*)p1;
		*intp1=destine_blocknum;
		intp1++;
        *intp1=destine_offset;
		p1=p1+8;
		strcpy(p1,value->value);
		
		m_ptheblocks[newblock].written_block();
		m_ptheblocks[newblock].used_block();
		return 1;
	}
	//save_number是目标的块,nextblock是其下一块
	blocknumber=get_block(INDEX,this_index,save_number);
	p=m_ptheblocks[blocknumber].m_address;
	intp=(int*)p;
	intp++;
	if(*intp<31)    //块中空间足够新增记录
	{
		for(p1=p+INDEX_RECORD_LEN;*p1!='$';p1=p1+INDEX_RECORD_LEN)//找到空位置
		;
		if(p1>=m_ptheblocks[blocknumber].m_address+BLOCK_SIZE)
		{
			printf("error:this m_ptheblocks is already full\n");
			return 0;
		}
		intp1=(int*)p1;
		*intp1=destine_blocknum;
		intp1++;
		*intp1=destine_offset;
		temp=p1+8;
		strcpy(temp,value->value);
		(*intp)++;                         //count++

		m_ptheblocks[blocknumber].written_block();
		m_ptheblocks[blocknumber].used_block();

		return 1;
	}
	else
	{
        firstblock=get_block(INDEX,this_index,0);
		p1=m_ptheblocks[firstblock].m_address;
		intp1=(int*)p1;
        newblocknumber=*intp1;//因为从0算起，差个1
		*intp1++;
	
		m_ptheblocks[firstblock].written_block();
		m_ptheblocks[firstblock].used_block();
		newblock=get_block(INDEX,this_index,newblocknumber);
		p1=m_ptheblocks[newblock].m_address;
		for(temp=p1;temp<p1+BLOCK_SIZE;temp++)
			*temp='$';
		//写新块的HEAD
		intp1=(int*)p1;
		*intp1=newblocknumber;//HEAD中的块号
		intp1++;
		*intp1=1;//HEAD中的COUNT
		intp1++;
		*intp1=nextblock;//下一块的块号
		p1=m_ptheblocks[newblock].m_address+12;
		strcpy(p1,value->value);
		//写NODE
		p1=m_ptheblocks[newblock].m_address+INDEX_RECORD_LEN;
		intp1=(int*)p1;
		*intp1=destine_blocknum;
		intp1++;
        *intp1=destine_offset;
		p1=p1+8;
		strcpy(p1,value->value);
	
		m_ptheblocks[newblock].written_block();
		m_ptheblocks[newblock].used_block();
		
		intp++;
		*intp=newblocknumber;//刚才满的那块的下一块应该是这个新块
	
		m_ptheblocks[blocknumber].written_block();
		m_ptheblocks[blocknumber].used_block();

  		return 1;

	}
	return 0;
	}*/
}
