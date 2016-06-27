#ifndef __buffer_h__
#define __buffer_h__
#pragma once

#include "MiniSQL.h"
//#define TABLE 1					//用来标记块中存的是table数据
//#define INDEX 2					//用来标记块中存的是index数据
//#define MAP 3					//用来标记块中存的是map数据

//#define MAX_BLOCKS 1024		//buffer区内总块数
//#define BLOCK_SIZE 8192			//每块的字节数 8KB
//#define NAME_SIZE  100			//文件名的大小
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
    
	void mark_block(string filename, unsigned int offset) {
		m_name = filename;
		m_offset_number = offset;
	}
	void buffer_clear(string filename,short int index_table);
protected:
	static CBufferManager *m_ptheblocks;

	static void using_block(unsigned int number);
	static unsigned int max_count_number();

	void flush_block();
	/*为新块做标记
	void mark_block(string filename,unsigned int offset) {
		m_name = filename;
		m_offset_number = offset;
	}
*/
	  
};
#endif
