#include "/osFileSys/file_sys.hpp"

char buffer[SYS_SIZE];

File_sys::File_sys()
{
    curpath = ROOT_DIR;
}


/////////////////
///辅助函数实现/////////////////////////////////////////////
/////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
bool File_sys::reverse_inode_bitmap(int pos)
{
    int bp = pos/8;
    int shift = pos%8;
    char c;
    if(fseek(fp, superblock.inode_bitmap_start+bp, SEEK_SET)!=0)
    {
        cout<<"reverse_inode_bitmap: fseek error"<<endl;
        return false;
    }
    if(fread(&c, 1, 1, fp)!=1)
    {
        cout<<"reverse_inode_bitmap: fread error"<<endl;
        return false;
    }
    //将相应位置的bit设置为相反值
    c = (c ^ (1<<shift));
    if(fseek(fp, superblock.inode_bitmap_start+bp, SEEK_SET)!=0)
    {
        cout<<"reverse_inode_bitmap: fseek error"<<endl;
        return false;
    }
    if(fwrite(&c, 1, 1, fp)!=1)
    {
        cout<<"reverse_inode_bitmap: fwrite error"<<endl;
        return false;
    }
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
bool File_sys::reverse_block_bitmap(int pos)
{
    int bp = pos/8;
    int shift = pos%8;
    char c;
    if(fseek(fp, superblock.block_bitmap_start+bp, SEEK_SET)!=0)
    {
        cout<<"reverse_block_bitmap: fseek error"<<endl;
        return false;
    }
    if(fread(&c, 1, 1, fp)!=1)
    {
        cout<<"reverse_block_bitmap: fread error"<<endl;
        return false;
    }
    //将相应位置的bit设置为相反值
    c = (c ^ (1<<shift));
    if(fseek(fp, superblock.block_bitmap_start+bp, SEEK_SET)!=0)
    {
        cout<<"reverse_block_bitmap: fseek error"<<endl;
        return false;
    }
    if(fwrite(&c, 1, 1, fp)!=1)
    {
        cout<<"reverse_block_bitmap: fwrite error"<<endl;
        return false;
    }
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
bool File_sys::write_inode_to_basic(int pos, INode node)
{
    if(fseek(fp,superblock.inode_table_start+superblock.inode_size*pos, SEEK_SET)!=0)
    {
        cout<<"write_inode_to_basic: fseek error"<<endl;
        return false;
    }
    if(fwrite(&node, sizeof(INode),1, fp)!=1)
    {
        cout<<"write_inode_to_basic: fwrite error"<<endl;
        return false;
    }
    return true;

}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
INode File_sys::read_inode_from_basic(int pos)
{
    INode node;
    if(fseek(fp,superblock.inode_table_start+superblock.inode_size*pos, SEEK_SET)!=0)
    {
        cout<<"write_inode_to_basic: fseek error"<<endl;
    }
    if(fread(&node, sizeof(INode), 1, fp)!=1)
    {
        cout<<"read_inode_from_basic: fread error"<<endl;
    }
    return node;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
INode File_sys::find_inode_in_directory(INode inode, string fileName, bool &found)
{
    int cnt = inode.range_count;
    int FILE_PER_BLOCK = superblock.block_size/sizeof(File);
    for(int i = 0; i < 10; ++i) 
    {
        if(cnt == 0) break;
        fseek(fp, BLOCK_SIZE*inode.dir_address[i], SEEK_SET);
        for(int j = 0; j < FILE_PER_BLOCK; ++j) 
        {
            if(cnt == 0) break;
            cnt--;
            File file;
            fread(&file, sizeof(File), 1, fp);
            if(file.inode_id == -1) continue;
            if(strcmp(file.filename, fileName.c_str()) == 0) 
            {
                return read_inode_from_basic(file.inode_id);
            }
        }
    }
    found = false;
    return inode;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
bool File_sys::write_file_to_dentry(File file, INode inode)
{
    int parent_dir_count = inode.file_count;
    int range_count = inode.range_count;
    int block_file_num = superblock.block_size/sizeof(File);
    if(parent_dir_count == range_count)
    {
        //证明前面都是满的，所以只需要在后面添加
        if(parent_dir_count%block_file_num==0)
        {
            //需要添加新块
            inode.dir_address[parent_dir_count/block_file_num] = find_usable_block();
            reverse_block_bitmap(inode.dir_address[parent_dir_count/block_file_num]);
            if(fseek(fp, superblock.block_size*inode.dir_address[parent_dir_count/block_file_num], SEEK_SET)!=0)
            {
                cout<<"write_file_to_dentry: fseek error"<<endl;
                return false;
            }
        }
        else
        {
            //不需要添加新块
            if(fseek(fp, superblock.block_size*inode.dir_address[parent_dir_count/block_file_num]
            +sizeof(File)*(parent_dir_count%block_file_num), SEEK_SET)!=0)
            {
                cout<<"write_file_to_dentry: fseek error"<<endl;
                return false;
            }
        }

        if(fwrite(&file, sizeof(File), 1, fp)!=1)
        {
            cout<<"write_file_to_dentry: fwrite error"<<endl;
            return false;
        }
        inode.file_count++;
        inode.range_count++;
        write_inode_to_basic(inode.id, inode);
        if(inode.id == cur_inode.id) 
        {
            cur_inode = read_inode_from_basic(cur_inode.id);
        }
        if(inode.id == root_inode.id) 
        {
            root_inode = read_inode_from_basic(root_inode.id);
        }
        return true;
    }
    else
    {
        //前面删除了一些文件，所以从头遍历寻找空位
        int range = inode.range_count;
        for(int i=0;i<10;i++)
        {
            if(range<=0)
            {
                break;
            }
            for(int j=0;j<block_file_num;j++)
            {
                if(range<=0)
                {
                    break;
                }
                range--;
                File temp;
                if(fseek(fp, superblock.block_size*inode.dir_address[i]+sizeof(File)*j, SEEK_SET)!=0)
                {
                    cout<<"write_file_to_dentry: fseek error"<<endl;
                    return false;
                }
                if(fread(&temp, sizeof(File), 1, fp)!=1)
                {
                    cout<<"write_file_to_dentry: fread error"<<endl;
                    return false;
                }
                if(temp.inode_id == -1)
                {
                    //找到空位
                    inode.file_count++;
                    write_inode_to_basic(inode.id, inode);
                    if(fseek(fp, superblock.block_size*inode.dir_address[i]
                    +sizeof(File)*j, SEEK_SET)!=0)
                    {
                        cout<<"write_file_to_dentry: fseek error"<<endl;
                        return false;
                    }
                    if(fwrite(&file, sizeof(File), 1, fp)!=1)
                    {
                        cout<<"write_file_to_dentry: fwrite error"<<endl;
                        return false;
                    }
                    if(inode.id == cur_inode.id) 
                    {
                        cur_inode = read_inode_from_basic(cur_inode.id);
                    }
                    if(inode.id == root_inode.id) 
                    {
                        root_inode = read_inode_from_basic(root_inode.id);
                    }
                    return true;
                }
            }
        }
        inode.file_count++;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
bool File_sys::fill_random_string(int block_pos)
{
    srand(time(0));
    if(fseek(fp, superblock.block_size*block_pos, SEEK_SET)!=0)
    {
        cout<<"fill_random_string: fseek error"<<endl;
        return false;
    }
    for(int i=0;i<superblock.block_size;i++)
    {
        char c = 'a'+(rand()%26);
        if(fwrite(&c, 1, 1, fp)!=1)
        {
            cout<<"fill_random_string: fwrite error"<<endl;
            return false;
        }
    }
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
bool File_sys::write_address_to_block(Address address, int block_pos, int shift)
{
    if(fseek(fp, block_pos*superblock.block_size+shift*sizeof(Address), SEEK_SET)!=0)
    {
        cout<<"write_address_to_block: fseek error"<<endl;
        return false;
    }
    if(fwrite(&address, sizeof(Address), 1, fp)!=1)
    {
        cout<<"write_address_to_block: fwrite error"<<endl;
        return false;

    }
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
bool File_sys::delete_file_from_dentry(INode inode, string fileName)
{
    int range = inode.range_count;
    int block_file_num = superblock.block_size/sizeof(File);
    for(int i=0;i<10;i++)
    {
        if(range<=0)
        {
            break;
        }
        if(fseek(fp, inode.dir_address[i]*superblock.block_size, SEEK_SET)!=0)
        {
            cout<<"delete_file_from_dentry: fseek error"<<endl;
            return false;
        }
        for(int j=0;j<block_file_num;j++)
        {
            if(range<=0)
            {
                break;
            }
            range--;
            File temp;
            if(fread(&temp, sizeof(File), 1, fp)!=1)
            {
                cout<<"delete_file_from_dentry: fread error"<<endl;
                return false;
            }
            if(temp.inode_id==-1)
            {
                continue;
            }
            if(strcmp(temp.filename, fileName.c_str())==0)
            {
                if(fseek(fp, -sizeof(File), SEEK_CUR)!=0)
                {
                    cout<<"delete_file_from_dentry: fseek error"<<endl;
                    return false;
                }
                temp.inode_id = -1;
                if(fwrite(&temp, sizeof(File), 1, fp)!=1)
                {
                    cout<<"delete_file_from_dentry: fwrite error"<<endl;
                    return false;
                }
                inode.file_count--;
                write_inode_to_basic(inode.id, inode);
                return true;
            }
        }
    }
    return false;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
int File_sys::find_usable_inode()
{
    if(fseek(fp, superblock.inode_bitmap_start, SEEK_SET)!=0)
    {
        cout<<"find_usable_inode: fseek error"<<endl;
        return -1;
    }
    int res = -1;
    for(int i=0;i<superblock.inode_bitmap_size;i++)
    {
        char c;
        if(fread(&c, 1, 1, fp)!=1)
        {
            cout<<"find_usable_inode: fread error"<<endl;
            return -1;
        }
        for(int j=0;j<8;j++)
        {
            if(((c>>j)^1)==1)
            {
                res = 8*i+j;
                break;
            }
        }
        if(res!=-1)
        {
            break;
        }
    }
    cout<<"inode res: "<<res<<endl;
    return res;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
int File_sys::find_usable_block()
{
    if(fseek(fp, superblock.block_bitmap_start, SEEK_SET)!=0)
    {
        cout<<"find_usable_block: fseek error"<<endl;
        return -1;
    }
    int res = -1;
    for(int i=0;i<superblock.block_bitmap_size;i++)
    {
        char c;
        if(fread(&c, 1, 1, fp)!=1)
        {
            cout<<"find_usable_block: fread error"<<endl;
            return -1;
        }
        for(int j=0;j<8;j++)
        {
            if(((c>>j)^1)==1)
            {
                res = 8*i+j;
                break;
            }
        }
        if(res!=-1)
        {
            break;
        }
    }
    return res;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
int File_sys::number_of_usable_block()
{
    if(fseek(fp, superblock.block_bitmap_start, SEEK_SET)!=0)
    {
        cout<<"number_of_usable_block: fseek error"<<endl;
        return -1;
    }
    int res=0;
    for(int i=0;i<superblock.block_bitmap_size;i++)
    {
        char c;
        if(fread(&c, 1, 1, fp)!=1)
        {
            cout<<"number_of_usable_block: fread error"<<endl;
            return -1;
        }
        for(int j=0;j<8;j++)
        {
            if(((c>>j)^1)==1)
            {
                res++;
            }
        }
    }
    return res;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
int File_sys::number_of_usable_inode()
{
    if(fseek(fp, superblock.inode_bitmap_start, SEEK_SET)!=0)
    {
        cout<<"number_of_usable_inode: fseek error"<<endl;
        return -1;
    }
    int res=0;
    for(int i=0;i<superblock.inode_bitmap_size;i++)
    {
        char c;
        if(fread(&c, 1, 1, fp)!=1)
        {
            cout<<"number_of_usable_inode: fread error"<<endl;
            return -1;
        }
        for(int j=0;j<8;j++)
        {
            if(((c>>j)^1)==1)
            {
                res++;
            }
        }
    }
    return res;
}

///////////////////
//需求功能函数实现////////////////////////////////////////
//////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
void File_sys::print_welcome()
{
    cout<<endl;
    cout<<"Welcome to the file system"<<endl;
    cout<<"Group info:"<<endl;
    cout<<"\tFeng Kengda: 202030430066"<<endl;
    cout<<"\tChen Guoan: 202030430295"<<endl;
    cout<<"\tXiao Jiajian: 202030430219\n"<<endl;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
bool File_sys::format()
{
    //打开文件，读取
    std::fstream f;
    f.open(BASIC_FILE, std::ios::in | std::ios::out);
    //不存在文件，第一次初始化
    if(!f)
    {
        //打开文件
        fp = fopen(BASIC_FILE, "wb+");
        if(fp == NULL)
        {
            cout<<"format: file create error"<<endl;
            return false;
        }
        if(fwrite(buffer, SYS_SIZE, 1, fp)!=1)
        {
            cout<<"format: fwrite error"<<endl;
            return false;
        }
        //配置超级块（block 0）
        superblock.system_size = SYS_SIZE;
        superblock.superblock_size = SUPER_BLOCK_SIZE;

        superblock.block_size = BLOCK_SIZE;
        superblock.block_num = BLOCK_NUM;
        superblock.block_bitmap_size = BLOCK_BITMAP_SIZE;
        superblock.block_bitmap_start = BLOCK_BITMAP_START;

        superblock.inode_size = INODE_SIZE;
        superblock.inode_bitmap_size = INODE_BITMAP_SIZE;
        superblock.inode_bitmap_start = INODE_BITMAP_START;
        superblock.inode_table_size = INODE_TABLE_SIZE;
        superblock.inode_table_start = INODE_TABLE_START;

        superblock.max_filename_size = MAX_FILENAME_SIZE;
        superblock.max_file_in_dir = BLOCK_SIZE/sizeof(File)*10;
        if(fseek(fp, 0, SEEK_SET)!=0)
        {
            cout<<"format: fseek error"<<endl;
            return false;
        }
        if(fwrite(&superblock, sizeof(Superblock), 1, fp)!=1)
        {
            cout<<"format: fwrite error"<<endl;
            return false;
        }
        //inode的bitmap块（block 1）
        if(fseek(fp, superblock.inode_bitmap_start, SEEK_SET)!=0)
        {
            cout<<"format: fseek error"<<endl;
            return false;
        }
        for(int i=0;i<superblock.inode_bitmap_size;i++)
        {
            char c = 0;
            if(fwrite(&c, 1, 1, fp)!=1)
            {
                cout<<"format: fwrite error"<<endl;
                return false;
            }
        }
        //block的bitmap块（block 2、3）
        if(fseek(fp, superblock.block_bitmap_start, SEEK_SET)!=0)
        {
            cout<<"format: fseek error"<<endl;
            return false;
        }
        for (int i = 0; i < superblock.block_bitmap_size; i++)
        {
            char c=0;
            if(fwrite(&c, 1, 1, fp)!=1)
            {
                cout<<"format: fseek error"<<endl;
                return false;
            }
        }
        
        //将超级块、bitmap、table的block置为1
        //超级
        reverse_block_bitmap(0);
        //inode bitmap
        reverse_block_bitmap(1);
        //block bitmap
        reverse_block_bitmap(2);
        reverse_block_bitmap(3);
        //inode table
        for(int i=0;i<superblock.inode_table_size/1024;i++)
        {
            reverse_block_bitmap(i+superblock.inode_table_start/1024);
        }

        //初始化根目录
        root_inode.clear();
        root_inode.id = 0;
        root_inode.file_mode = 1;//目录
        root_inode.file_count = 0;
        root_inode.range_count = 0;
        root_inode.create_time = time(NULL);
        reverse_inode_bitmap(0);
        write_inode_to_basic(0, root_inode);
        cur_inode = root_inode;
    } 
    else
    {
        //存在文件，加载数据
        fp = fopen(BASIC_FILE, "rb+");
        if(fp == NULL)
        {
            cout<<"format: load file system error"<<endl;
            return false;
        }

        if(fseek(fp, 0, SEEK_SET)!=0)
        {
            cout<<"format: fseek error"<<endl;
            return false;
        }
        if(fread(&superblock, sizeof(Superblock), 1, fp)!=1)
        {
            cout<<"format: fread error"<<endl;
            return false;
        }
        cur_inode = read_inode_from_basic(0);
        root_inode = read_inode_from_basic(0);
    }
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
bool File_sys::create_file(string file_name, int file_size)
{
    INode start_node;
    if(file_name[0]=='/')
    {
        //绝对路径
        start_node = root_inode;
    }
    else
    {
        //相对路径
        start_node = cur_inode;
    }


    cout<<"cur_node: "<<cur_inode.id<<endl;
    cout<<"start inode: "<<start_node.id<<endl;


    vector<string> path;
    string t="";
    for(int i=0;i<file_name.size();i++)
    {
        if(file_name[i]=='/')
        {
            if(t!="")
            {
                path.push_back(t);
                t="";
            }
            continue;
        }
        else
        {
            t+=file_name[i];
        }
    }
    if(t=="")
    {
        cout<<"no file name"<<endl;
        return false;
    }
    if(t.size()>=MAX_FILENAME_SIZE)
    {
        cout<<"file name is too long"<<endl;
        return false;
    }
    path.push_back(t);


    cout<<"path: ";
    for(int i=0;i<path.size();i++)
    {
        cout<<path[i]<<" ";
    }
    cout<<endl;


    //通过路径找到文件创建位置
    int find_count = path.size()-1;
    bool found = true;
    INode next_node;
    for(int i=0;i<find_count;i++)
    {
        cout<<i<<endl;
        next_node = find_inode_in_directory(start_node, path[i], found);
        if(next_node.file_mode==0||!found)
        {
            cout<<"no such directory"<<endl;
            return false;
        }
        start_node = next_node;
    }
    //在此之后，start_node就是当前创建的文件的parent目录
    found = true;
    //判断是否存在这个文件
    next_node = find_inode_in_directory(start_node, path[find_count], found);
    if(found)
    {
        cout<<"file already exist"<<endl;
        return false;
    }
    if(start_node.file_count>=superblock.max_file_in_dir)
    {
        cout<<"reach max number of file in the directory"<<endl;
        return false;
    }

    int free_block_count = number_of_usable_block();
    if(file_size>free_block_count)
    {
        cout<<"no enough space"<<endl;
        return false;
    }
    if(file_size>MAX_FILE_SIZE)
    {
        cout<<"size exceed max size"<<endl;
        return false;
    }

    File f;
    f.inode_id = find_usable_inode();
    reverse_inode_bitmap(f.inode_id);
    strcpy(f.filename, path[find_count].c_str());
    
    INode new_node;
    new_node.clear();
    new_node.file_size = file_size;
    new_node.create_time = time(NULL);
    new_node.file_mode = 0;
    new_node.id = f.inode_id;

    write_file_to_dentry(f, start_node);
    //开始将数据写入block
    int f_size = file_size;
    //先写入10个直接block
    for(int i=0;i<10;i++)
    {
        if(f_size==0)
        {
            break;
        }
        new_node.dir_address[i]=find_usable_block();
        reverse_block_bitmap(new_node.dir_address[i]);
        fill_random_string(new_node.dir_address[i]);
        f_size--;
    }
    if(f_size>0)
    {
        //获取一个block存储地址
        new_node.indir_address = find_usable_block();
        reverse_block_bitmap(new_node.indir_address);
        int shift=0;
        for(;f_size>0;f_size--)
        {
            Address address;
            int block_id = find_usable_block();
            reverse_block_bitmap(block_id);
            address.set_block_id(block_id);
            //因为不是新文件，不需要设置inode id，所以置为零就行了
            address.set_inode_id(0);
            fill_random_string(block_id);
            write_address_to_block(address, new_node.indir_address, shift);
            shift++;
        }
    }
    //将inode写入inode表
    write_inode_to_basic(new_node.id, new_node);
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
bool File_sys::delete_file(string file_name)
{
    INode start_node;
    if(file_name[0]=='/')
    {
        //绝对路径
        start_node = root_inode;
    }
    else
    {
        //相对路径
        start_node = cur_inode;
    }
    vector<string> path;
    string t="";
    for(int i=0;i<file_name.size();i++)
    {
        if(file_name[i]=='/')
        {
            std::remove_if(t.begin(), t.end(), isspace);
            if(t!="")
            {
                path.push_back(t);
                t="";
            }
            else
            {
                continue;
            }
        }
        else
        {
            t+=file_name[i];
        }
    }
    if(t=="")
    {
        cout<<"no such name"<<endl;
        return false;
    }
    path.push_back(t);

    int find_count = path.size()-1;
    bool found = true;
    INode next_node;
    for(int i=0;i<find_count;i++)
    {
        next_node = find_inode_in_directory(start_node, path[i], found);
        if(next_node.file_mode==0||!found)
        {
            cout<<"no such directory"<<endl;
            return false;
        }
        start_node = next_node;
    }
    //在此之后，start_node就是当前创建的文件的parent目录
    found = true;
    //判断是否存在这个文件
    next_node = find_inode_in_directory(start_node, path[find_count], found);
    if(!found)
    {
        cout<<"no such file"<<endl;
        return false;
    }
    
    delete_file_from_dentry(start_node, path[find_count]);
    if(next_node.id == -1)
    {
        cout<<"no such file"<<endl;
        return false;
    }
    reverse_inode_bitmap(next_node.id);
    int f_size = next_node.file_size;
    for(int i=0;i<10;i++)
    {
        if(f_size==0)
        {
            break;
        }
        f_size--;
        reverse_block_bitmap(next_node.dir_address[i]);
    }
    if(f_size>0)
    {
        reverse_block_bitmap(next_node.indir_address);
        int shift = 0;
        for(;f_size>0;f_size--)
        {
            Address address;
            if(fseek(fp, next_node.indir_address*superblock.block_size
            +shift*sizeof(Address), SEEK_SET)!=0)
            {
                cout<<"delete_file: fseek error"<<endl;
                return false;
            }
            if(fread(&address, sizeof(Address), 1, fp)!=1)
            {
                cout<<"delete_file: fwrite error"<<endl;
                return false;
            }
            reverse_block_bitmap(address.get_block_id());
            shift++;
        }
    }
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
bool File_sys::create_dir(string dir_name)
{
    INode start_node;
    if(dir_name[0]=='/')
    {
        //绝对路径
        start_node = root_inode;
    }
    else
    {
        //相对路径
        start_node = cur_inode;
    }
    vector<string> path;
    string t="";
    for(int i=0;i<dir_name.size();i++)
    {
        if(dir_name[i]=='/')
        {
            std::remove_if(t.begin(), t.end(), isspace);
            if(t!="")
            {
                path.push_back(t);
                t="";
            }
            else
            {
                continue;
            }
        }
        else
        {
            t+=dir_name[i];
        }
    }
    if(t=="")
    {
        cout<<"no directory name"<<endl;
        return false;
    }
    if(t.size()>=MAX_FILENAME_SIZE)
    {
        cout<<"directory name is too long"<<endl;
        return false;
    }
    path.push_back(t);
    //通过路径找到文件创建位置
    int find_count = path.size()-1;
    bool found = true;
    INode next_node;
    for(int i=0;i<find_count;i++)
    {
        next_node = find_inode_in_directory(start_node, path[i], found);
        if(next_node.file_mode==0||!found)
        {
            cout<<"no such directory"<<endl;
            return false;
        }
        start_node = next_node;
    }
    //在此之后，start_node就是当前创建的文件的parent目录
    found = true;
    //判断是否存在这个文件
    next_node = find_inode_in_directory(start_node, path[find_count], found);
    if(found)
    {
        cout<<"directory already exist"<<endl;
        return false;
    }
    if(start_node.file_count>=superblock.max_file_in_dir)
    {
        cout<<"reach max number of file in the directory"<<endl;
        return false;
    }

    File f;
    f.inode_id = find_usable_inode();
    reverse_inode_bitmap(f.inode_id);
    strcpy(f.filename, path[find_count].c_str());
    write_file_to_dentry(f, start_node);

    INode new_node;
    new_node.id=f.inode_id;
    new_node.file_size=0;
    new_node.create_time=time(NULL);
    new_node.file_mode=1;
    write_inode_to_basic(new_node.id, new_node);
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
bool File_sys::delete_dir(string dir_name)
{
    INode inode;
    if(dir_name[0] == '/') 
    {
        inode = root_inode;
    }
    else 
    {
        inode = cur_inode;
    }
    vector<string> path;
    string temp = "";
    for(int i = 0; i < dir_name.size(); ++i) 
    {
        if(dir_name[i] == '/') 
        {
            if(temp != "") 
            {
                path.push_back(temp);
                temp = "";
            }
            continue;
        }
        temp += dir_name[i];
    }
    if(temp == "") return false;
    path.push_back(temp);

    int cnt = (int)path.size();
    INode next_node;
    bool found = true;
    for(int i = 0; i < cnt-1; ++i) {
        next_node = find_inode_in_directory(inode, path[i], found);
        if(!found||next_node.file_mode != 1)
        {
            cout<<"no dir 1"<<endl;
            return false;
        }
        inode = next_node;
    }
    found = true;
    next_node = find_inode_in_directory(inode, path[cnt-1], found);
    if(!found||next_node.file_mode == 0) 
    {
        cout<<"no dir 2"<<endl;
        return false;
    }

    if(dir_name[0] == '/') 
    {
        vector<string> cur;
        string tempdir = "";
        for(int i = 1; i < curpath.size(); ++i) 
        {
            if(curpath[i] == '/') 
            {
                if(tempdir != "") 
                {
                    cur.push_back(tempdir);
                    tempdir = "";
                }
                continue;
            }
            tempdir += curpath[i];
        }
        if(tempdir != "") 
        {
            cur.push_back(tempdir);
        }
        if(cnt <= cur.size()) {
            found = true;
            for(int i = 0; i < cnt; ++i) {
                if(path[i] != cur[i]) {
                    found = false;
                    break;
                }
            }
            if(found) 
            {
                cout<<"can not delete this dir"<<endl;
                return false;
            }
                
        }
    }
    if(next_node.file_count > 0) 
    {
        cout<<"have file"<<endl;
        return false;
    }
    delete_file_from_dentry(inode, path[cnt-1]);
    inode = next_node;
    reverse_inode_bitmap(inode.id);
    int count = inode.file_count;
    int block_file_num = superblock.block_size/sizeof(File);
    for(int i = 0; i < 10; ++i) {
        if(count <= 0) {
            break;
        }
        count -= block_file_num;
        reverse_block_bitmap(inode.dir_address[i]);
    }
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
bool File_sys::change_dir(string path)
{
    if(path=="/")
    {
        cur_inode = root_inode;
        curpath=ROOT_DIR;
        return true;
    }

    INode start_node;
    if(path[0]=='/')
    {
        //绝对路径
        start_node = root_inode;
    }
    else
    {
        //相对路径
        start_node = cur_inode;
    }
    vector<string> path_vec;
    string t="";
    for(int i=0;i<path.size();i++)
    {
        if(path[i]=='/')
        {
            if(t!="")
            {
                path_vec.push_back(t);
                t="";
            }
            continue;
        }
        else
        {
            t+=path[i];
        }
    }
    if(t=="")
    {
        cout<<"no directory name"<<endl;
        return false;
    }
    path_vec.push_back(t);

    int find_count = path_vec.size()-1;
    bool found = true;
    INode next_node;
    for(int i=0;i<find_count;i++)
    {
        next_node = find_inode_in_directory(start_node, path_vec[i], found);
        if(next_node.file_mode==0||!found)
        {
            cout<<"no such directory"<<endl;
            return false;
        }
        start_node = next_node;
    }
    //在此之后，start_node就是当前创建的文件的parent目录
    found = true;
    //判断是否存在这个文件
    next_node = find_inode_in_directory(start_node, path_vec[find_count], found);
    if(!found)
    {
        cout<<"no such directory"<<endl;
        return false;
    }
    if(next_node.id == -1)
    {
        cout<<"no such file"<<endl;
        return false;
    }
    if(next_node.file_mode==0)
    {
        cout<<"no a directory"<<endl;
        return false;
    }

    cur_inode = next_node;

    if(path[0]=='/')
    {
        curpath=ROOT_DIR;
    }
    for(int i = 0; i < path_vec.size(); i++) {
        curpath+="/";
        curpath+=path_vec[i];
    }
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
void File_sys::dir()
{
    cur_inode = read_inode_from_basic(cur_inode.id);
    int cnt = cur_inode.range_count;
    int FILE_PER_BLOCK = superblock.block_size/sizeof(File);
    cout<<left<<setw(28)<<"Name";
    cout<<left<<setw(10)<<"Type";
    cout<<left<<setw(30)<<"Create time";
    cout<<left<<setw(10)<<"Size(KB)"<<endl;
    for(int i = 0; i < 10; ++i) {
        if(cnt == 0) break;
        for(int j = 0; j < FILE_PER_BLOCK; ++j) {
            if(cnt == 0) break;
            cnt--;
            File file;
            fseek(fp, BLOCK_SIZE*cur_inode.dir_address[i]+sizeof(File)*j, SEEK_SET);
            fread(&file, sizeof(File), 1, fp);
            if(file.inode_id == -1) continue;
            INode inode = read_inode_from_basic(file.inode_id);

            std::cout <<left<< std::setw(28) << file.filename;
            if(inode.file_mode == 1) 
            {
                std::cout <<left<< std::setw(10) << "Dir";
            }
            else
            {
                std::cout <<left<< std::setw(10) << "File";
            } 
            char buffer[40];
            strftime(buffer, 40, "%a %b %d %T %G", localtime(&inode.create_time));
            std::cout <<left<< std::setw(30) << buffer;
            std::cout <<left<< std::setw(10) << inode.file_size<<endl;
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
bool File_sys::cp(string file1, string file2)
{
    //处理file1路径
    INode f1_inode;
    if(file1[0]=='/')
    {
        //绝对路径
        f1_inode = root_inode;
    }
    else
    {
        //相对路径
        f1_inode = cur_inode;
    }
    vector<string> f1_path;
    string t="";
    for(int i=0;i<file1.size();i++)
    {
        if(file1[i]=='/')
        {
            //std::remove_if(t.begin(), t.end(), isspace);
            if(t!="")
            {
                f1_path.push_back(t);
                t="";
            }
            else
            {
                continue;
            }
        }
        else
        {
            t+=file1[i];
        }
    }
    if(t=="")
    {
        cout<<"no directory name"<<endl;
        return false;
    }
    f1_path.push_back(t);
    //通过路径找到文件创建位置
    int f1_find_count = f1_path.size()-1;
    bool found = true;
    INode f1_next_node;
    for(int i=0;i<f1_find_count;i++)
    {
        f1_next_node = find_inode_in_directory(f1_inode, f1_path[i], found);
        if(f1_next_node.file_mode==0||!found)
        {
            cout<<"no such directory"<<endl;
            return false;
        }
        f1_inode = f1_next_node;
    }
    //在此之后，start_node就是当前创建的文件的parent目录
    found = true;
    //判断是否存在这个文件
    f1_next_node = find_inode_in_directory(f1_inode, f1_path[f1_find_count], found);
    
    if(f1_inode.file_size>number_of_usable_block())
    {
        cout<<"space limited"<<endl;
        return false;
    }
    f1_inode = f1_next_node;

    //处理file2路径
    INode f2_inode;
    if(file2[0]=='/')
    {
        //绝对路径
        f2_inode = root_inode;
    }
    else
    {
        //相对路径
        f2_inode = cur_inode;
    }
    vector<string> f2_path;
    t="";
    for(int i=0;i<file2.size();i++)
    {
        if(file2[i]=='/')
        {
            //std::remove_if(t.begin(), t.end(), isspace);
            if(t!="")
            {
                f2_path.push_back(t);
                t="";
            }
            else
            {
                continue;
            }
        }
        else
        {
            t+=file2[i];
        }
    }
    if(t=="")
    {
        cout<<"no directory name"<<endl;
        return false;
    }
    f2_path.push_back(t);
    //通过路径找到文件创建位置
    int f2_find_count = f2_path.size()-1;
    found = true;
    INode f2_next_node;
    for(int i=0;i<f2_find_count;i++)
    {
        f2_next_node = find_inode_in_directory(f2_inode, f2_path[i], found);
        if(f2_next_node.file_mode==0||!found)
        {
            cout<<"no such directory"<<endl;
            return false;
        }
        f2_inode = f2_next_node;
    }
    //在此之后，start_node就是当前创建的文件的parent目录
    found = true;
    //判断是否存在这个文件
    f2_next_node = find_inode_in_directory(f2_inode, f2_path[f2_find_count], found);

    //将file1的数据读取出来
    vector<char> file1_content;
    int file_size_temp=f1_inode.file_size;
    for(int i=0;i<10, file_size_temp>0;i++,file_size_temp--)
    {
        if(fseek(fp, f1_inode.dir_address[i]*superblock.block_size,SEEK_SET)!=0)
        {
            cout<<"cp: fseek error"<<endl;
            return false;
        }
        for(int j=0;j<superblock.block_size;j++)
        {
            char c;
            if(fread(&c, 1, 1, fp)!=1)
            {
                cout<<"cp: fread error"<<endl;
                return false;
            }
            file1_content.push_back(c);
        }
    }
    if(file_size_temp>0)
    {
        int shift=0;
        for(;file_size_temp>0;file_size_temp--)
        {
            Address address;
            if(fseek(fp, f1_inode.indir_address*superblock.block_size+shift*sizeof(Address), SEEK_SET)!=0)
            {
                cout<<"cp: fseek error"<<endl;
                return false;
            }
            if(fread(&address, sizeof(Address), 1, fp)!=1)
            {
                cout<<"cp: fread error"<<endl;
                return false;
            }
            int block_id = address.get_block_id();
            if(fseek(fp, block_id*superblock.block_size, SEEK_SET)!=0)
            {
                cout<<"cp: fseek error"<<endl;
                return false;
            }
            for(int j=0;j<superblock.block_size;j++)
            {
                char c;
                if(fread(&c, 1, 1, fp)!=1)
                {
                    cout<<"cp: fread error"<<endl;
                    return false;
                }
                file1_content.push_back(c);
            }
            shift++;
        }
    }

    //创建file2
    File f;
    f.inode_id = find_usable_inode();
    reverse_inode_bitmap(f.inode_id);
    strcpy(f.filename, f2_path[f2_path.size()-1].c_str());

    INode new_node;
    new_node.file_size = f1_inode.file_size;
    new_node.create_time = time(NULL);
    new_node.file_mode = f1_inode.file_mode;
    new_node.id = f.inode_id;

    write_file_to_dentry(f, f2_inode);
    //开始将数据写入block
    int f2_size_temp = new_node.file_size;
    int char_num=0;
    //先写入10个直接block
    for(int i=0;i<10;i++)
    {
        if(f2_size_temp==0)
        {
            break;
        }
        new_node.dir_address[i]=find_usable_block();
        reverse_block_bitmap(new_node.dir_address[i]);
        if(fseek(fp, new_node.dir_address[i]*superblock.block_size, SEEK_SET)!=0)
        {
            cout<<"cp: fseek error"<<endl;
            return false;
        }
        for(int j=0;j<superblock.block_size;j++)
        {
            char c = file1_content[char_num];
            char_num++;
            if(fwrite(&c, 1, 1, fp)!=1)
            {
                cout<<"cp: fwrite error"<<endl;
                return false;
            }
        }
        f2_size_temp--;
    }
    if(f2_size_temp>0)
    {
        //获取一个block存储地址
        new_node.indir_address = find_usable_block();
        reverse_block_bitmap(new_node.indir_address);
        int shift=0;
        for(;f2_size_temp>0;f2_size_temp--)
        {
            Address address;
            int block_id = find_usable_block();
            reverse_block_bitmap(block_id);
            address.set_block_id(block_id);
            //因为不是新文件，不需要设置inode id，所以置为零就行了
            address.set_inode_id(0);
            fill_random_string(block_id);
            write_address_to_block(address, new_node.indir_address, shift);
            shift++;
        }
    }
    //将inode写入inode表
    write_inode_to_basic(new_node.id, new_node);
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
void File_sys::sum()
{
    cout<<"System Size: "<<superblock.system_size/1024<<"KB"<<endl;
    cout<<"Total number of blocks: "<<superblock.block_num<<endl;
    cout<<"Number of block used: "<<superblock.block_num-number_of_usable_block()<<endl;
    cout<<"Number of block unused: "<<number_of_usable_block()<<endl;
    cout<<"Number of inode unused: "<<number_of_usable_inode()<<endl;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
bool File_sys::cat(string file_name)
{
    //处理file1路径
    INode start_node;
    if(file_name[0]=='/')
    {
        //绝对路径
        start_node = root_inode;
    }
    else
    {
        //相对路径
        start_node = cur_inode;
    }
    vector<string> f1_path;
    string t="";
    for(int i=0;i<file_name.size();i++)
    {
        if(file_name[i]=='/')
        {
            std::remove_if(t.begin(), t.end(), isspace);
            if(t!="")
            {
                f1_path.push_back(t);
                t="";
            }
            else
            {
                continue;
            }
        }
        else
        {
            t+=file_name[i];
        }
    }
    if(t=="")
    {
        cout<<"no directory name"<<endl;
        return false;
    }
    f1_path.push_back(t);
    //通过路径找到文件创建位置
    int f1_find_count = f1_path.size()-1;
    bool found = true;
    INode f1_next_node;
    for(int i=0;i<f1_find_count;i++)
    {
        f1_next_node = find_inode_in_directory(start_node, f1_path[i], found);
        if(f1_next_node.file_mode==0||!found)
        {
            cout<<"no such directory"<<endl;
            return false;
        }
        start_node = f1_next_node;
    }
    //在此之后，start_node就是当前创建的文件的parent目录
    found = true;
    //判断是否存在这个文件
    f1_next_node = find_inode_in_directory(start_node, f1_path[f1_find_count], found);
    if(!found)
    {
        cout<<"file does not exist"<<endl;
        return false;
    }

    //将file1的数据读取出来
    vector<char> file1_content;
    int file_size_temp=f1_next_node.file_size;
    for(int i=0;i<10, file_size_temp>0;i++,file_size_temp--)
    {
        if(fseek(fp, f1_next_node.dir_address[i]*superblock.block_size,SEEK_SET)!=0)
        {
            cout<<"cp: fseek error"<<endl;
            return false;
        }
        for(int j=0;j<superblock.block_size;j++)
        {
            char c;
            if(fread(&c, 1, 1, fp)!=1)
            {
                cout<<"cp: fread error"<<endl;
                return false;
            }
            file1_content.push_back(c);
        }
    }
    if(file_size_temp>0)
    {
        int shift=0;
        for(;file_size_temp>0;file_size_temp--)
        {
            Address address;
            if(fseek(fp, f1_next_node.indir_address*superblock.block_size+shift*sizeof(Address), SEEK_SET)!=0)
            {
                cout<<"cp: fseek error"<<endl;
                return false;
            }
            if(fread(&address, sizeof(Address), 1, fp)!=1)
            {
                cout<<"cp: fread error"<<endl;
                return false;
            }
            int block_id = address.get_block_id();
            if(fseek(fp, block_id*superblock.block_size, SEEK_SET)!=0)
            {
                cout<<"cp: fseek error"<<endl;
                return false;
            }
            for(int j=0;j<superblock.block_size;j++)
            {
                char c;
                if(fread(&c, 1, 1, fp)!=1)
                {
                    cout<<"cp: fread error"<<endl;
                    return false;
                }
                file1_content.push_back(c);
            }
            shift++;
        }
    }
    for(int i=0;i<file1_content.size();i++)
    {
        if(i%100==0)
        {
            cout<<endl;
        }
        cout<<file1_content[i];
    }
    cout<<endl;
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////