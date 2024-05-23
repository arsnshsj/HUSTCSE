#include<iostream>
#include<filesystem>
#include<stack>
#include<fstream>
#include<format>
#include<chrono>
#include<string>
#include<stdio.h>
#include<map>
#include<string.h>
#include<time.h>

using namespace std;
using namespace std::filesystem;

//树节点结构体
struct tree 
{
    int deep;
    int id,pre;
    path str;
    time_t filetime;
    int filesize;
    tree *child, *bro,*preid;
};
typedef struct tree tree;

//遍历磁盘文件结构体
struct pathinfo 
{
    int num;
    path str;
    int deep;
    tree *t;
};

bool judge_is_empty(const directory_entry& entry);

template <typename TP> std::time_t to_time_t(TP tp);

int init();

void run(ofstream &file, string str);

void get_info(char c);

void get_tree_deep();

void destroy_tree();

void compare(int flag);

void create_point(tree *h, string str, string time, string size);

void run_file(ofstream &file, string str);

void myfile();

void mydir();