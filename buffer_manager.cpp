#include "buffer_manager.h"

using namespace std;

//CBufferManager m_ptheblocks[MAX_BLOCKS];
CBufferManager* CBufferManager::m_ptheblocks = NULL;

/*初始化buffer区，但不分配内存*/
void CBufferManager::initiate_blocks()
{
	m_ptheblocks = new CBufferManager [MAX_BLOCKS];
	for(int i=0;i<MAX_BLOCKS;i++){//对每一块的参数初始化
		m_ptheblocks[i].m_address = NULL;
		m_ptheblocks[i].m_index_table=0;
		m_ptheblocks[i].m_is_written=0;
		m_ptheblocks[i].m_being_used=0;
		m_ptheblocks[i].m_count=0;
	}
}

/*退出时调用，将所有需要写回的块数据写回相应文件，并释放buffer区*/
void CBufferManager::flush_all_blocks(){   
	for(int i=0;i<MAX_BLOCKS;i++){
		m_ptheblocks[i].flush_block();
		if(m_ptheblocks[i].m_address != NULL)
			delete [] m_ptheblocks[i].m_address;
	}
}

/*使用块函数，buffer内部函数*/
void CBufferManager::using_block(unsigned int number)
{
	unsigned int i;
	m_ptheblocks[number].m_being_used=1;
	for(i=0;i<MAX_BLOCKS;i++){
		if(i!=number && !m_ptheblocks[number].m_being_used)
			m_ptheblocks[i].m_count++;  //所有没正在使用的块count加1
		else if(i==number)
			m_ptheblocks[i].m_count=0;  //正在使用的块count置0，使正在被使用的块不被换出
	}
}

/*找到count值最大的块号（即近似LRU算法中要求替换出的块），buffer内部函数*/	
unsigned int CBufferManager::max_count_number()
{
	unsigned int i;
	unsigned int max_count=m_ptheblocks[0].m_count;
	unsigned int block_number=0;
	for(i=0;i<MAX_BLOCKS;i++)
		if(m_ptheblocks[i].m_count>max_count){
			max_count=m_ptheblocks[i].m_count;
			block_number=i;
		}
	return block_number;
}

/*若该块需要写回，将该块的数据写回相应文件，换出时和退出时调用,buffer内部函数*/
void CBufferManager::flush_block()
{
	fstream DBfile;
	const char* file_name = m_name.c_str();
	DBfile.open(file_name,ios::out|ios::binary);
	if(m_is_written && m_index_table) {
			DBfile.seekp(BLOCK_SIZE*m_offset_number,ios::beg);
			DBfile.write(m_address,BLOCK_SIZE);
			DBfile.close();
		}
	
}

/*为record manager和index manager提供块（写），在其初始建立时调用*/
unsigned int CBufferManager::get_blank_block(short int index_table)
{
	unsigned int number;
	unsigned int i;
	char *p;
	for(i=0;i<MAX_BLOCKS;i++)
		if(!m_ptheblocks[i].m_index_table && !m_ptheblocks[i].m_being_used)
			break;
	if(i<MAX_BLOCKS) { //表示有空块
		number=i;
		m_ptheblocks[number].m_address = new char [BLOCK_SIZE];	

        for(p=m_ptheblocks[number].m_address;p<m_ptheblocks[number].m_address+BLOCK_SIZE;p++)
		   *p='$';
	}
	else{ //表示无空块，须判断换出哪个块和是否有必要写回文件
		number=max_count_number();
		m_ptheblocks[number].flush_block();
	}
	using_block(number);
	m_ptheblocks[number].m_index_table=index_table;
	return number;
}

/*为record manager和index manager提供块（读或改）,若在buffer中找不到则调用get_blank_block为其分配空间并做标记*/
unsigned int CBufferManager::get_block(short int index_table, string filename, unsigned int offset_number)
{
	
	unsigned int number;
	unsigned int i,mark;
	
	for(i=0;i<MAX_BLOCKS;i++) {
		if(m_ptheblocks[i].m_index_table==index_table && m_ptheblocks[i].m_name == filename && 
			m_ptheblocks[i].m_offset_number==offset_number) { //表示在buffer中找到了该块
			number=i;
			using_block(number);
			return number;
		}
	}

	mark=get_blank_block(index_table);//未找到该块则调用get_blank_block为其分配空间并做标记
	m_ptheblocks[mark].mark_block(filename,offset_number);
	const char*file_name = filename.c_str();
	fstream DBfile(file_name,ios::binary|ios::in);
	DBfile.seekg(BLOCK_SIZE*offset_number, ios::beg);
	DBfile.read(m_ptheblocks[mark].m_address,BLOCK_SIZE);
	DBfile.close();

	using_block(mark);
	return mark;
}
/*
int main(){
	CBufferManager buffer;
	buffer.initiate_blocks();
	string filename = "buffer_test.txt";
	int block_num = buffer.get_block(TABLE, filename,0);
	for(char* text = buffer.m_ptheblocks[block_num].m_address; text <buffer.m_ptheblocks[block_num].m_address+BLOCK_SIZE;text++){
		cout<< *text;
	}
	return 0;
}
*/

