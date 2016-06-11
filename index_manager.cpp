

#include "index_manager.h"

using namespace std;

int CIndexManager::keycompare(char *a,char *b, int tag)//a>=b ����1
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
			   if(tempa>='A'&&tempa<='Z')   //���Դ�Сд��ȫ������Сд��ĸ
				   tempa=tempa+32;
			   if(tempb>='A'&&tempb<='Z')
				   tempb=tempb+32;
			   if(tempa>tempb)
				   return 1;
			   if(tempa<tempb)
				   return 0;
		   }
		   if(*cha=='\0'&&*chb=='\0')
			   return 1;//���
		   else if( *cha!='\0'&&*chb=='\0')
			   return 1;//a>b;
		   else return 0;

           break;
	case INT:
		   numa=atoi(a);  //ת��������
		   numb=atoi(b);
		   if(numa>=numb)  return 1;
		   else return 0;
		   break;
	case FLOAT:
		   floata=atof(a);
		   floatb=atof(b);  //ת����float����
		   if(floata>=floatb) return 1;
		   else return 0;
		   break;
	}
	return -1;
}

int CIndexManager::quick_insert(char *value,int tag,int blocknumber,int offsetinblock)//tagΪ�ȽϷ�ʽ
{
	unit_node *currentnode,*save_pointer;
	unit_node *newnode = new unit_node;
	newnode->block_number=blocknumber;
	newnode->offset_in_block=offsetinblock;
	strcpy(newnode->key,value);
	if(this->m_count>=31)   //����ÿ��Ѿ��洢��31����¼������
	{
		printf("mistake in using this function\n");
		return 0;
	}
	else
	{
		currentnode=this->m_first;
		while(currentnode!=NULL&&keycompare(newnode->key,currentnode->key,tag)==1)//���������ҵ��½ڵ�����λ��
			save_pointer=currentnode;
			currentnode=currentnode->next;
	}
	save_pointer->next=newnode;
	newnode->next=currentnode;
	this->m_count++;
	return 1;
}

int CIndexManager::insert_entry(char *value,int tag,int blocknumber,int offsetinblock,int total_idx_block)  //����index�ļ����ܿ���
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
	else if(head_save!=currenthead)   //������м���Ҫ�ӿ�
	{
		total_idx_block++;
        newhead = new CIndexManager;
	    newnode = new unit_node;
	    newhead->m_count=1;
		
		strcpy(newhead->m_minkey,value);
		newhead->m_first=newnode;
		newhead->m_next=currenthead;
		head_save->m_next=newhead;

		newhead->m_blocknumber = total_idx_block-1; //Ϊ�¿鰲�ſ��
		head_save->m_next_block_number = total_idx_block-1;
		if(currenthead!=NULL)        //��������м�ڵ㲻�����Ľڵ�
			newhead->m_next_block_number = currenthead->m_blocknumber;
		else
			newhead->m_next_block_number = -1;   //����¿������һ�飬��¼����һ�����Ϊ-1��

		newnode->next=NULL;
		strcpy(newnode->key,value);
		newnode->block_number=blocknumber;
		newnode->offset_in_block=offsetinblock;
		return total_idx_block;
	}
	else{
		total_idx_block++;				//��ͷ����Ҫ������һ���ڵ�
		newhead = new CIndexManager;
	    newnode = new unit_node;
		//��Ϊthisָ�벻�ܸı䣬����½�һ���ڵ�󣬽�this�����ݺ��½��Ľڵ㽻��
		if(currenthead->m_count>0){                                       //������ȫΪ��
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
			currenthead->m_blocknumber = total_idx_block-1; //֮ǰ������
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
	int table_blocknum=0;//��¼��ǰ��TABLE �ڼ���BLOCK
	int index_blocknum=0;//��¼��ǰ��INDEX �ڼ���BLOCK
	int map_blocknum=0;
	unsigned int current_record=1;//��ǰ�ڼ�����¼
	string this_table,this_index;
	int currentblock;
	int table_total_block;
	int offset_in_block;
	int records_per_block=BLOCK_SIZE/recordlen;
	insertvalue value; //����һ������
	int record_number;	
	char *p,*temp;
	int *intp;

	
	this_table = m_tablename + ".tab"; 
	this_index = m_indexname + ".idx";
   
    currentblock=get_block(INDEX, this_index,0);//׼������index�ĵ�0�飬�ÿ첻����������
	p=m_ptheblocks[currentblock].m_address;
	intp=(int*)p;
	*intp=1;
	intp++;
	*intp=-1;//��һ���
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
	for(table_blocknum=1;table_blocknum<=table_total_block;table_blocknum++)//��table�ļ��еĵ�1�鿪ʼ������Ϊ��0������ܿ���
	{ 
		 currentblock=get_block(TABLE,this_table,table_blocknum);
		 p=m_ptheblocks[currentblock].m_address;
		 for(;p<m_ptheblocks[currentblock].m_address+BLOCK_SIZE;p=p+recordlen)//������ȡ��¼
		 {
			if(*p!='$')                                                      //��¼����
			{

				offset_in_block=(p-m_ptheblocks[currentblock].m_address)/recordlen; //��øü�¼������ڿ��ƫ�Ƶ�ַ
				record_number=records_per_block*(m_ptheblocks[currentblock].m_offset_number-1)+offset_in_block;
				//��������ֵ
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

	for(;currenthead!=NULL;currenthead=currenthead->m_next) {     //�ӵ�һ���ڵ㿪ʼд��
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
		*intp=currenthead->m_blocknumber;      //ÿ����ͷ��д���ţ���¼��������һ�����
		intp++;
		*intp=currenthead->m_count;
		intp++;
		*intp=currenthead->m_next_block_number;
		p=p+12;
		strcpy(p,currenthead->m_minkey);
		//������ֵ�ͼ�¼��ַ����д�ؿ�
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

	current_block=get_block(INDEX, this_index,0);//��ȡ
	p=m_ptheblocks[current_block].m_address;
	intp=(int*)p;
	int total_idxblock_number = *(intp);
    if(total_idxblock_number==1)   //�������һ��û�м�¼��index��Ϊ�˲����¼ʱ�����жϣ���this->m_count����Ϊ0
    	this->m_count = 0;
	intp++;
	block_no = *(intp);
	m_ptheblocks[current_block].used_block();
	//...........
	while(block_no!=-1) {     //�ӵ�һ���鿪ʼ��ȡ
		current_block=get_block(INDEX,this_index,block_no);
		p=m_ptheblocks[current_block].m_address;
		intp=(int*)p;
		currenthead->m_blocknumber = *intp;      //ÿ����ͷ��д���ţ���¼��������һ�����
		intp++;
		currenthead->m_count = *intp;
		intp++;
		currenthead->m_next_block_number = *intp;
		p=p+12;
		strcpy(currenthead->m_minkey,p);

		//����һ����Ϳ�ĵ�һ����¼����ռ�
		newhead = new CIndexManager;
		currenthead->m_next = newhead;
		currentnode = new unit_node;
		currenthead ->m_first = currentnode;

		//������ȡ����ֵ�ͼ�¼��ַ
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
		currentnode->next = NULL;    //ɾ�����һ��û���������Ľڵ�
        m_ptheblocks[current_block].written_block();
		m_ptheblocks[current_block].used_block();
		//.........
		if((block_count+1)<total_idxblock_number)
			currenthead = currenthead->m_next;
		else{
			delete newnode;                   //ɾ�����һ��û�д����ݵ�index manager
			currenthead->m_next = NULL;
		}

	}
	
	return 1;

}

void CIndexManager::freespace()           //���������ʱ�洢���ݵ�b+��
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
	
    if(conds->cond!=EQ)    //index����ֻ�����ֵ����
	{
		printf("compare condition error\n");
		return 0;
	}
	//����ʱ����ı��ļ��ṹ����˲���Ҫ���������b+��
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
	min_key=p+12;                   //��øýڵ�����С��ֵ
	while(keycompare(conds->value,min_key,conds->type)==1)
	 {
		 block_save=m_ptheblocks[blocknumber].m_offset_number;
		 m_ptheblocks[blocknumber].used_block();
		 if(nextblock_number==-1)//�������һ��
			 break;
		 blocknumber=get_block(INDEX,this_index,nextblock_number);
		 p=m_ptheblocks[blocknumber].m_address;
	     intp=(int*)(p+8);
         nextblock_number=*intp;
	     min_key=p+12;
	}
	m_ptheblocks[blocknumber].used_block();
	blocknumber=get_block(INDEX,this_index,block_save);//�ҵ�Ŀ���
	p=m_ptheblocks[blocknumber].m_address+INDEX_RECORD_LEN;

	while(col)                                       //��ӡ�������Ե�����
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
			head_save->m_count--;   //����������һ
			//ɾ��record��map�ļ��ж�Ӧ�ļ�¼
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
			if(node_save!=currentnode){     //ɾ�������м�ڵ�����Ľڵ�
				node_save->next = currentnode ->next;
				delete currentnode;
				save_index();
				freespace();
				return 1;
			}
			else if(currentnode->next!=NULL){           //ɾ�����ǵ�һ���������������ڵ��л���������������
				head_save->m_first = currentnode->next;
				delete currentnode;
				save_index();
				freespace();
				return 1;
			}
			else{						//ɾ�����ǽڵ������һ������ֵ����Ҫ�����ڵ�һ��ɾ��
				blocknumber = get_block(INDEX,this_index,0);
				p=m_ptheblocks[blocknumber].m_address;
	            intp=(int*)p;
	            int last_blk_num = *intp-1;
	            *intp--;
	            m_ptheblocks[blocknumber].written_block();
	            m_ptheblocks[block_count].used_block();      //��ı���Ƿ���Ҫ����������û�е���

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
	//ԭ���Ĵ��롣������������������
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
         if(nextblock_number==-1)//�������һ��
			 break;
		 blocknumber=get_block(INDEX,this_index,nextblock_number);
         m_ptheblocks[blocknumber].m_being_used=1;
		 p=m_ptheblocks[blocknumber].m_address;
	     intp=(int*)(p+8);
         nextblock_number=*intp;
	     min_key=p+12;
	 }
	 m_ptheblocks[blocknumber].used_block();
	 blocknumber=get_block(INDEX,this_index,block_save);//�ҵ�Ŀ���
	 m_ptheblocks[blocknumber].m_being_used=1;
	 p=m_ptheblocks[blocknumber].m_address+INDEX_RECORD_LEN;
     while(p<m_ptheblocks[blocknumber].m_address+BLOCK_SIZE)
	 {
		 key=p+8;
		 if(strcmp(key,conds->value)==0)//�ҵ�
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
			 //ɾ��RECORD
             for(temp=record_point;temp<record_point+recordlen;temp++)
				 *temp='$';
			
			 m_ptheblocks[recordblock].written_block();
			 m_ptheblocks[recordblock].used_block();
			 //ɾ��INDEX
             for(temp=p;temp<p+INDEX_RECORD_LEN;temp++)
			    *temp='$';
			 written_block();
			 m_ptheblocks[blocknumber].written_block();
			 m_ptheblocks[blocknumber].used_block();
			 //ɾ��MAP
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
			 temp=p+map_in_offset-1;//��ΪOFFSET��1����
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
	destine_offset=recordnum%r_p_b;		//��record�ļ��ж�λ

	blocknumber=get_block(INDEX, this_index,0);//׼������index�ĵ�0�飬�ÿ첻����������
	p=m_ptheblocks[blocknumber].m_address;
	intp=(int*)p;
	total_block_number = *(intp);

    build_index();
    total_block_number=insert_entry(value->value,value->type,destine_blocknum,destine_offset,total_block_number);
    *(intp) = total_block_number;
    intp++;											//�����������ҳ�ڵ㣬index�ļ��ĵ�0����Ҫ����
    *(intp) = this->m_blocknumber;
    m_ptheblocks[blocknumber].written_block();
    m_ptheblocks[blocknumber].used_block();
    //
    save_index();
    freespace();
	/*ԭ���Ĵ��롣����������������
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
		*intp=1;//HEAD�еĿ��
		intp++;
		*intp=1;//HEAD�е�COUNT
		intp++;
		*intp=-1;//��һ��Ŀ��
		p=m_ptheblocks[blocknumber].m_address+12;//integer ������4��byte, ��ͷ�洢��������
		strcpy(p,value->value);
		//дNODE
		p=m_ptheblocks[blocknumber].m_address+INDEX_RECORD_LEN;
		intp=(int*)p;
		*intp=destine_blocknum;//��¼Ҷ�ڵ�Ŀ��
		intp++;
        *intp=destine_offset;//��¼Ҷ�ڵ�ļ�¼��ƫ�Ƶ�ַ
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
	p=m_ptheblocks[blocknumber].m_address+12;//��ţ� count, ��һ�����
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
		(*intp2)++; //��0���еĿ�������һ
		intp2++;
		nextblock=*intp2;//��¼֮ǰ�Ŀ�ʼ���
		*intp2=m_ptheblocks[newblock].m_offset_number;

		m_ptheblocks[first_block].written_block();
		m_ptheblocks[first_block].used_block();
		//��д�¿�һ���Ĳ���
		p1=m_ptheblocks[newblock].m_address;
		for(temp=p1;temp<p1+BLOCK_SIZE;temp++)
			*temp='$';
		//д�¿��HEAD
		intp1=(int*)p1;
		*intp1=m_ptheblocks[newblock].m_offset_number;//HEAD�еĿ��
		intp1++;
		*intp1=1;//HEAD�е�COUNT
		intp1++;
		*intp1=nextblock;//��һ��Ŀ��????????????
		p1=m_ptheblocks[newblock].m_address+12;
		strcpy(p1,value->value);
		//дNODE
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
	//save_number��Ŀ��Ŀ�,nextblock������һ��
	blocknumber=get_block(INDEX,this_index,save_number);
	p=m_ptheblocks[blocknumber].m_address;
	intp=(int*)p;
	intp++;
	if(*intp<31)    //���пռ��㹻������¼
	{
		for(p1=p+INDEX_RECORD_LEN;*p1!='$';p1=p1+INDEX_RECORD_LEN)//�ҵ���λ��
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
        newblocknumber=*intp1;//��Ϊ��0���𣬲��1
		*intp1++;
	
		m_ptheblocks[firstblock].written_block();
		m_ptheblocks[firstblock].used_block();
		newblock=get_block(INDEX,this_index,newblocknumber);
		p1=m_ptheblocks[newblock].m_address;
		for(temp=p1;temp<p1+BLOCK_SIZE;temp++)
			*temp='$';
		//д�¿��HEAD
		intp1=(int*)p1;
		*intp1=newblocknumber;//HEAD�еĿ��
		intp1++;
		*intp1=1;//HEAD�е�COUNT
		intp1++;
		*intp1=nextblock;//��һ��Ŀ��
		p1=m_ptheblocks[newblock].m_address+12;
		strcpy(p1,value->value);
		//дNODE
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
		*intp=newblocknumber;//�ղ������ǿ����һ��Ӧ��������¿�
	
		m_ptheblocks[blocknumber].written_block();
		m_ptheblocks[blocknumber].used_block();

  		return 1;

	}
	return 0;
	}*/
}
