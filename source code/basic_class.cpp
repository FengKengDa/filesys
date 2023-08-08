#include</osFileSys/basic_class.hpp>
#include<cstring>

Superblock::Superblock() 
    :system_size(0), superblock_size(0),block_size(0),block_num(0),
    block_bitmap_size(0),block_bitmap_start(0),
    inode_size(0),inode_bitmap_size(0),inode_bitmap_start(0),
    inode_table_size(0),max_filename_size(0), inode_table_start(0),
    max_file_in_dir(0)
{
    
}

INode::INode() 
    :id(-1), file_size(0), file_mode(0), create_time(0), 
    file_count(0), range_count(0), indir_address(0)
{
    memset(this->dir_address, 0, sizeof(dir_address));
}

void INode::clear() 
{
    this->id = -1;
    this->file_size = 0;
    this->file_mode = 0;
    this->create_time = 0;
    this->file_count = 0;
    memset(dir_address, 0, sizeof(dir_address));
    this->indir_address = 0;
    
}

File::File() 
    :inode_id(-1)
{

}

Address::Address() 
    :address(0)
{

}

//最小的14位是block值，block前面10位是inode值，剩下8个bit无用
int Address::get_block_id()
{
	return address%16384; //对2^14取模,获取余数的是后14个bit的值
}
int Address::get_inode_id()
{
	return address/16384;
}
void Address::set_block_id(int id)
{
	address = address-get_block_id()+id;
}
void Address::set_inode_id(int id)
{
	address = address-(get_inode_id()+id)*16384;
}
