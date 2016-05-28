#include <iostream> 
#include <string>
#include <fstream>
#define TABLE 1					//用来标记块中存的是table数据
#define INDEX 2					//用来标记块中存的是index数据
#define MAP 3					//用来标记块中存的是map数据

#define MAX_BLOCKS 1024		//buffer区内总块数
#define BLOCK_SIZE 8192			//每块的字节数 8KB
#define NAME_SIZE  100			//文件名的大小

using namespace std;

class CBufferManager
{
public:
	char* m_address;				//块的首地址
	string m_name;					//块对应的表或索引名
	unsigned int m_offset_number;	//块对应的表或索引中的页序数
	short int m_index_table;		//块对应的是表，则值为TABLE;若是索引，则值为INDEX;若空为0
	short int m_is_written;			//该块是否被改过，初值为0，改过置1
	short int m_being_used;			//该块当前是否被读或写，若是则值为1
	unsigned int m_count;			//用于实现LRU算法

	CBufferManager(){}
	~CBufferManager(){}

	static void flush_all_blocks();
	static void initiate_blocks();

	unsigned int get_block(short int index_table, string filename, unsigned int offset_number);
	unsigned int get_blank_block(short int index_table);

	/*标志该块已被改过*/
	void written_block() {
		m_is_written=1;
	}
	/*标志该块已经不在用了*/
	void used_block() {
		m_being_used=0;
	}
    //static CBufferManager *m_ptheblocks;
protected:
	static CBufferManager *m_ptheblocks;

	static void using_block(unsigned int number);
	static unsigned int max_count_number();

	void flush_block();
	/*为新块做标记*/
	void mark_block(string filename,unsigned int offset) {
		m_name = filename;
		m_offset_number = offset;
	}
	  
};



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

