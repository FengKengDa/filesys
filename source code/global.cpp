#define SYS_SIZE 1024*1024*16       //2^24
#define BLOCK_SIZE 1024             //1KB
#define BLOCK_NUM 1024*16           //2^14
#define INODE_SIZE 128
#define ADDRESS_SIZE 24
#define ADDRESS_BLOCK 14            // 16MB/1KB = 2^14 BLOCKS
#define ADDRESS_INODE 10
#define SUPER_BLOCK_SIZE 1024       // 1BLOCK
#define INODE_BITMAP_SIZE 1024      //inode最多有2^10个，一个block1024*8，可以放下
#define BLOCK_BITMAP_SIZE 2*1024    //1024*8*2刚好放下block的bitmap
#define INODE_TABLE_SIZE 128*1024   //max inode = 2^10 = 1024  1block可以包含8个inode
                                    //当inode数量达到最多时，共需要1024/8 = 128个block
#define INODE_BITMAP_START SUPER_BLOCK_SIZE                         
#define BLOCK_BITMAP_START INODE_BITMAP_START+INODE_BITMAP_SIZE     
#define INODE_TABLE_START BLOCK_BITMAP_START+BLOCK_BITMAP_SIZE      
#define MAX_FILE_SIZE 266           //10*1K+1024/4*1K
#define MAX_FILENAME_SIZE 28        //FILE->SIZE = 32  INT = 4  CHAR[]可以设置成32-4=28
#define BASIC_FILE "basic_file.bin"
#define ROOT_DIR "~"