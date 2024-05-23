#include"Tree.h"
#pragma GCC optimize("03")

using namespace std;
using namespace std::filesystem;

int main()
{
    int op;
    cout<<"输入 1 确认开始扫描文件,预计需要时间1分钟。"<<endl;
    cin>>op;
    init();//初始化，建兄弟孩子树，生成sql文件
    cout<<"输入 2 获取兄弟孩子树深度。"<<endl;
    cin>>op;
    get_tree_deep();//获取树的深度

    get_info('1');//第二次统计信息

    ofstream file;
    file.open("1vs2.txt");
    file.close();//清空文件内容
    for(int i =1 ;i <= 3; i++)
    {
        cout<<"输入第 "<<i<<" 次文件修改指令"<<endl;
        myfile();
    }//执行文件操作

    get_info('2');//第二次统计信息
    
    cout<<"观察修改指令完成后的变化"<<endl;
    compare(1);//第一次对比

    cout<<"输入文件夹修改指令"<<endl;
    mydir();//执行文件夹操作
    get_info('3');//第三次统计信息
    cout<<"观察修改指令完成后的变化"<<endl;
    compare(2);//第二次对比
    
    destroy_tree();//销毁树
    return 0;
}
