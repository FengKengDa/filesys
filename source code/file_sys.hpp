#include <fstream>
#include <ctime>
#include <iomanip>
#include <string>
#include <iostream>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include "basic_class.hpp"
#include "/osFileSys/global.cpp"
using std::string;
using std::cin;
using std::cout;
using std::endl;
using std::vector;
using std::left;
using std::setw;

class File_sys
{
public:
    string curpath;
    INode cur_inode;
    std::FILE *fp;
    Superblock superblock;
    INode root_inode;
    File_sys();
    //需求功能
    bool format();
    void print_welcome();
    bool create_file(string file_name, int file_size);
    bool delete_file(string file_name);
    bool create_dir(string dir_name);
    bool delete_dir(string dir_name);
    bool change_dir(string path);
    void dir();
    bool cp(string file1, string file2);
    void sum();
    bool cat(string file_name);
    //辅助函数
    int number_of_usable_block();
    int number_of_usable_inode();
    int find_usable_inode();
    int find_usable_block();
    bool reverse_inode_bitmap(int pos);
    bool reverse_block_bitmap(int pos);
    bool write_inode_to_basic(int pos, INode inode);
    INode read_inode_from_basic(int pos);
    INode find_inode_in_directory(INode inode, string file_name, bool &found);
    bool write_file_to_dentry(File file, INode inode);
    bool fill_random_string(int block_pos);
    bool write_address_to_block(Address address, int block_pos, int shift);
    bool delete_file_from_dentry(INode inode, string file_name);
};
