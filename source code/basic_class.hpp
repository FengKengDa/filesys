#include</osFileSys/global.cpp>
#include<time.h>

class Superblock 
{
public:
    int system_size;
    int superblock_size;

    int block_size;
    int block_num;
    int block_bitmap_size;
    int block_bitmap_start;
    
    int inode_size;
    int inode_bitmap_size;
    int inode_bitmap_start;
    int inode_table_size;
    int inode_table_start;

    int max_filename_size;
    int max_file_in_dir;
    
    Superblock();
};

class INode 
{
public:
    int id;
    int file_size;
    int file_mode; // 0:file 1:dir
    int file_count; // 如果是目录的话，记录目录里面包含的文件数量，非目录-1；
    int range_count;
    time_t create_time;
    int dir_address[10]; //直接
    int indir_address;  //间接
    INode();
    void clear();
};

class Address
{
public:
	int address; //最大值2^24-1=16,777,215
public:
	Address();
	int get_block_id();
	void set_block_id(int id);
	int get_inode_id();
	void set_inode_id(int id);
};

class File {
public:
    int inode_id;
    char filename[MAX_FILENAME_SIZE];
    File();
};
