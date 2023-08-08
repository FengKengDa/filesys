#include</osFileSys/file_sys.hpp>
using namespace std;

int main()
{
    cout<<"start"<<endl;
    File_sys filesys;
    filesys.print_welcome();
    filesys.format();
    while(true)
    {
        cout<<filesys.curpath<<"$ ";
        string input;
        getline(std::cin,input);
        std::istringstream s(input);
        string st;
        vector<string> para;
        while(s>>st)
        {
            para.push_back(st);
        }
        int pm = para.size();
        if(pm==0)
        {
            continue;
        }
        if(para[0]=="createFile")
        {
            if(pm!=3)
            {
                cout<<"parameter number error"<<endl;
                continue;
            }
            int file_size = atoi(para[2].c_str());
            if(file_size<=0)
            {
                cout<<"file size error"<<endl;
                continue;
            }
            filesys.create_file(para[1], file_size);
        }
        else if(para[0]=="deleteFile")
        {
            if(pm!=2)
            {
                cout<<"parameter number error"<<endl;
                continue;
            }
            filesys.delete_file(para[1]);
        }
        else if(para[0]=="createDir")
        {
            if(pm!=2)
            {
                cout<<"parameter number error"<<endl;
                continue;
            }
            filesys.create_dir(para[1]);
        }
        else if(para[0]=="deleteDir")
        {
            if(pm!=2)
            {
                cout<<"parameter number error"<<endl;
                continue;
            }
            filesys.delete_dir(para[1]);
        }
        else if(para[0]=="changeDir")
        {
            if(pm!=2)
            {
                cout<<"parameter number error"<<endl;
                continue;
            }
            filesys.change_dir(para[1]);
        }
        else if(para[0]=="dir")
        {
            if(pm!=1)
            {
                cout<<"parameter number error"<<endl;
                continue;
            }
            filesys.dir();
        }
        else if(para[0]=="cp")
        {
            if(pm!=3)
            {
                cout<<"parameter number error"<<endl;
                continue;
            }
            filesys.cp(para[1],para[2]);
        }
        else if(para[0]=="sum")
        {
            if(pm!=1)
            {
                cout<<"parameter number error"<<endl;
                continue;
            }
            filesys.sum();
        }
        else if(para[0]=="cat")
        {
            if(pm!=2)
            {
                cout<<"parameter number error"<<endl;
                continue;
            }
            filesys.cat(para[1]);
        }
        else if(para[0]=="exit")
        {
            if(pm!=1)
            {
                cout<<"parameter number error"<<endl;
                continue;
            }
            break;
        }
        else
        {
            cout<<"no such command"<<endl;
        }
    }
    return 0;
}