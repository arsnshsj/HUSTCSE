#include "DatabaseBuild.h"
#include "Tree.h"

stack<pathinfo> stk;
stack<tree*> sk; 
map<string, tree*> mp;

tree *root=new tree();

int FileNum = 0, DirNum = 0,Empty_DirNum = 0;
int MaxDeep = 0, MaxLength = 0;
time_t tfirst = 1e10,tfinal = 0;
long long sum = 0,sum_size = 0;
string stg;

//判断目录是否为空
bool judge_is_empty(const directory_entry& entry)
{
    return filesystem::is_empty(entry.path());
}

//将file_time_type 格式转换成 time_t格式
template <typename TP> std::time_t to_time_t(TP tp) 
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
        + system_clock::now());
    return system_clock::to_time_t(sctp);
}

//扫描磁盘文件，同时生成sql文件并建立兄弟孩子树
int init()
{
    path str("C:\\Windows");
    if(!exists(str))//若路径不存在
    {
        return 1;
    }
    directory_entry entry(str);

    DatabaseBuild(format("{}", last_write_time(entry)));//先进行数据库初始化
    ofstream file;
    file.open("sql_dir.sql");
    file.close();
    file.open("sql_file.sql");
    file.close();

    //根节点信息初始化
    tree* h = new tree();
    h->deep = 0;
    h->bro = NULL;
    h->id = 0;
    h->preid = NULL;
    h->str = entry.path();
    h->filesize = -1;
    h->filetime = to_time_t(entry.last_write_time());
    root->child = h;
    root->bro = NULL;
    mp[""] = h;

    tree* tp;
     
    ofstream ol;
    stk.push({0,str,0,h});//文件夹入栈
    while(!stk.empty() )//栈不为空
    { 
        try{
            //获取栈顶信息
            path dir_entry = (stk.top()).str;
            int deep = (stk.top()).deep;
            int id = (stk.top()).num;
            tp = (stk.top()).t;
            stk.pop();//出栈
            MaxDeep = max(MaxDeep, deep);//求最大目录深度
            int flag = 0;
            
            //遍历当前栈顶目录下的文件
            for (const auto& entry : directory_iterator(dir_entry, directory_options::skip_permission_denied | directory_options::follow_directory_symlink))
            {
                //初始化新结点
                tree* t = new tree();
                t->bro = NULL;
                t->child = NULL;
                t->str = entry.path();
                t->filetime = to_time_t(entry.last_write_time());
                //使用map容器进行映射，方便后续进行模拟修改时查找节点
                mp[entry.path().string().substr(10)] = t;

                ol<<entry.path().string()<<endl;
                if(!flag)
                {
                    //初始化栈顶目录的孩子
                    tp->child = t;
                    t->deep = tp->deep + 1;
                    t->preid = tp;
                    t->pre = 1;
                    flag = 1;
                }
                else 
                {
                    //初始化栈顶目录下文件的兄弟节点
                    tp->bro = t;
                    t->preid = tp;
                    t->pre = 2;
                    t->deep = tp->deep + 1;
                }

                if (is_directory(entry.path()))
                {   
                    DirNum++;
                    t->id = DirNum;
                    t->filesize = 0;
                    insert_directories(entry.path().filename().string(),
                                      format("{}", last_write_time(entry)),
                                      DirNum,entry.path().string(),id);
                    //生成该文件夹的sql插入语句

                    if(judge_is_empty(entry))
                    {
                        Empty_DirNum++;//空文件夹计数
                    }
                    stk.push({DirNum,entry.path(),deep + 1,t});
                }
                else
                {
                    FileNum++;
                    t->id = FileNum + 8e4;
                    t->filesize = entry.file_size();
                    insert_file(entry.path().filename().string(),
                                format("{}", last_write_time(entry)),
                                FileNum,entry.path().string(),
                                entry.file_size(), id,
                                entry.path().extension().string());
                    //生成该文件的sql插入语句
                }
                if(entry.path().string().length() > MaxLength){
                    MaxLength = entry.path().string().length();
                    stg = entry.path().string();
                }//获取最大路径长度和具体路径
                tp = t;//当前结点更新
            }
            
        }
        catch (const exception& e) {
            //cerr << "\n遍历目录时发生错误: " << e.what() << "\n";
        }
    }
    ol.close();
    cout<< "DirNum: " << DirNum << endl;
    cout<< "include Empty_DirNum: " << Empty_DirNum << endl;
    cout<< "FileNum: " << FileNum << endl;
    cout<< "MaxDeep: " << MaxDeep << endl;
    cout<< "MaxLength: " << MaxLength << " " << stg << endl;
    return 0;
}

//对文件夹路径统计对应文件信息
void run(ofstream &file, string str)
{
    //当前路径不存在
    if(mp.count(str) == 0)
    {
        file<< str;
        file<< "(don't exsist), NULL" ;
        file<< ", NULL" ;
        file<< ", NULL" ;
        file<< ", NULL"<<endl;
        return ;
    }

    tree *h;
    h = mp[str];
    time_t time_first, time_final;
    long long sum_file_size = 0,sum_file = 0;

    time_final = time_first = h->filetime;

    //文件为空文件
    if(h->child == NULL)
    {
        file<< str;
        char *t1 = ctime(&(time_first));  t1[strlen(t1)-1] = 0;//删除ctime在尾部自带的换行符
        file<< "," << t1;
        char *t2 = ctime(&(time_final));  t2[strlen(t2)-1] = 0;
        file<< "," << t2;
        file<< "," << 0;
        file<< "," << 0<<endl;
        return ;
    }
    h = h->child;
    if(h->id > 8e4){//id大于8e4,说明是文件，统计其信息
        sum_file++;
        sum_file_size+= h->filesize;
    }
    
    time_t time = h->filetime;
    if(time_first > time) time_first = time;
    if(time_final < time) time_final = time;
    while(h->bro){//继续统计路径下的文件
        h = h->bro;
        if(h->id > 8e4)
        {
            sum_file++;
            sum_file_size+= h->filesize;
        }
        time_t time = h->filetime;
        if(time_first > time) time_first = time;
        if(time_final < time) time_final = time;
    }
    file<< str;
    char *t1 = ctime(&(time_first));  t1[strlen(t1)-1] = 0;
    file<< ", " << t1;
    char *t2 = ctime(&(time_final));  t2[strlen(t2)-1] = 0;
    file<< ", " << t2;
    file<< ", " << sum_file;
    file<< ", " << sum_file_size<<endl;
    sum += sum_file;
    sum_size += sum_file_size;
    tfirst = min(tfirst , time_first);
    tfinal = min(tfinal , time_final);
    return ;
}

//统计文件信息
void get_info(char c)
{
    ofstream file;
    switch (c)
    {
        case '1':
        file.open( "1.csv");//第一次文件信息统计的结果，输出到1.csv
        break;
        case '2':
        file.open( "2.csv");
        break;
        case '3':
        file.open( "3.csv");
        break;
        default:
        break;
    }
    file<< "path, time_final, time_first, sum_file, sum_file_size"<< endl;

    ifstream mystat;
    mystat.open("mystat.txt");
    string str;
    getline(mystat,str);//跳过第一行

    tfirst = 1e10,tfinal = 0;
    sum = 0,sum_size = 0;
    while(getline(mystat,str))//逐行读入
    {
        if(str.compare("end of dirs") == 0)break;
        //处理文件路径字符串
        str = str.substr(10);
        str = str.substr(0,str.length() - 1);
        run(file, str);//获取信息
    }
    mystat.close();
    file<< "整体信息统计如下： ";
    char *t1 = ctime(&(tfirst));  t1[strlen(t1)-1] = 0;
    file<< " " << t1;
    char *t2 = ctime(&(tfinal));  t2[strlen(t2)-1] = 0;
    file<< " " << t2;
    file<< " " << sum;
    file<< " " << sum_size<<endl;

    file.close();
    return ;
}

//获取树的深度
void get_tree_deep()
{
    tree *h;
    h = root->child;
    int treedeep = 0;
    sk.push(h);
    while(!sk.empty())//用栈进行递归
    {
        tree *t = sk.top();
        sk.pop();
        treedeep = max(treedeep, t->deep);
        if(t->child) 
        {
            sk.push(t->child);
        }
        if(t->bro) 
        {
            sk.push(t->bro);
        }
    }
    cout<< "treedeep: " << treedeep << endl;
    return ;
}

//销毁树
void destroy_tree()
{
    tree *h;
    h = root->child;
    int treedeep = 0;
    sk.push(h);
    while(!sk.empty())
    {
        tree *t = sk.top();
        sk.pop();
        if(t->child) 
        {
            sk.push(t->child);
        }
        if(t->bro) 
        {
            sk.push(t->bro);
        }
        free(t);
    }
    return ;
}

//统计文件信息前后对比
void compare(int flag)
{
    ifstream txt1,txt2;
    ofstream file;
    txt1.open("1.csv");
    if(flag == 1)// 第一次文件统计信息和第二次文件统计信息作对比
        txt2.open("2.csv"),file.open("1vs2.csv", ios::app);
    else // 第一次文件统计信息和第三次文件统计信息作对比
        txt2.open("3.csv"),file.open("1vs3.csv", ios::app);

    string str1,str2;
    // 跳过第一行
    getline(txt1,str1);
    getline(txt2,str2);

    int cnt = 0;
    // 逐行读入
    while(getline(txt1,str1) && getline(txt2,str2)){
        cnt++;
        file<<cnt<<endl;
        // 进行对比
        if(str1.compare(str2) == 0)
        {
            file<< "   no different"<<endl<<endl; 
            continue;
        }
            
        file<<"   different:"<<endl;
        file<<"    before:"<<str1<<endl;
        file<<"     after:"<<str2<<endl<<endl;
    }
    txt1.close();
    txt2.close();
    file.close();
    return ;
}

// 创建新结点
void create_point(tree *h, string str, string time, string size)
{
    tree *p = new (tree);
    h->bro = p;
    p->pre = 1;              p->preid = h;
    p->deep = h->deep + 1;   p->bro = NULL;
    p->child = NULL;         p->filesize = stoi(size);
    p->id = ++FileNum;       
    p->filetime = (time_t)stoi(time);
    mp[str.substr(10)] = p;
    // 结点信息初始化
    return ;
}

// 对文件路径统计对应文件信息
void run_file(ofstream &file, string str)   
{
    if(mp.count(str) == 0)
    {
        file<<" file doesn't exist."<< endl;
        return ;
    }

    tree *h = mp[str];
    char *t = ctime(&(h->filetime));
    t[strlen(t)-1] = 0;
    file<< (h->str).string() << " "<< t<< " "<< h->filesize<<endl;
    return ;
}

// 文件修改操作
void myfile()
{
    string str,pt,op,time,size,s;
    cin>>str;
    pt = str.substr(0,str.find(','));
    str = str.substr(str.find(',') + 1);
    op = str.substr(0,str.find(','));
    str = str.substr(str.find(',') + 1);
    time = str.substr(0,str.find(','));
    size = str.substr(str.find(',') + 1);
    // 处理读入的命令
    tree *h;

    ofstream file;
    file.open("1vs2.txt", ios::app);
    file<<"befor "<<endl; 
    s = pt.substr(10);
    s = s.substr(0,s.find_last_of("\\"));
    run_file(file, s);

    // 具体操作
    switch (op[0])
    {
        // 修改
    case 'M':
        pt = pt.substr(10);
        h = mp[pt];
        h->filesize = stoi(size);
        h->filetime = (time_t)stoi(time);
        break;

        // 增加
    case 'A':
        str = pt.substr(0,pt.find_last_of("\\"));
        str = str.substr(10);
        h = mp[str];
        //找到尾部结点
        if(h->child)
        {
            h = h->child;
            while(h->bro)
            {
                h = h->bro;
            }
        }
        create_point(h,pt,time,size);
        pt = pt.substr(10);
        break;

        // 删除
    case 'D':
        pt = pt.substr(10);
        pt = pt.substr(0,pt.find_last_of("\\"));
        if(mp.count(pt) == 0)break;
        h = mp[pt];
        // 更新连接关系
        if(h->bro)
        {
            if(h->pre == 1)
                h->preid->child = h->bro;
            else 
                h->preid->bro = h->bro;
        } 
        else 
        {
            if(h->pre == 1)
                h->preid->child = NULL;
            else 
                h->preid->bro = NULL;
        }
        mp.erase(pt);
        free(h);
        break;

    default:
        break;
    }
    file<<"after "<<endl;
    run_file(file, pt);
    file.close();
    return ;
}

// 文件夹修改操作
void mydir()
{
    string str,pt,op,time,size;
    cin>>str;
    pt = str.substr(0,str.find(','));
    // 处理读入的命令
    pt = pt.substr(10);
    pt = pt.substr(0,pt.length() - 1);
    tree *h;
    h = mp[pt];

    ofstream file;
    file.open("1vs3.txt");
    file<<"befor "<<endl;
    run(file, pt);

    // 更新结点关系
    if(h->bro)
    {
        if(h->pre == 1)
            h->preid->child = h->bro;
        else 
            h->preid->bro = h->bro;
    } 
    else 
    {
        if(h->pre == 1)
            h->preid->child = NULL;
        else 
            h->preid->bro = NULL;
    }

    mp.erase(pt);
    file<<"after "<<endl;
    run(file, pt);


    if(h->child == NULL)
    {
        free(h);
        return ;
    }

    // 级联删除
    sk.push(h->child);// 将以第一个孩子为根节点的树删除
    while(!sk.empty())
    {
        tree *t = sk.top();
        sk.pop();
        
        pt = (t->str).string();
        pt = pt.substr(10);

        if(t->child) 
        {
            sk.push(t->child);
        }
        if(t->bro) 
        {
            sk.push(t->bro);
        }
        mp.erase(pt);
        free(t);
    }

    file.close();
    return ;
}
