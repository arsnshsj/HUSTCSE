#include<iostream>
#include "DatabaseBuild.h"

using namespace std;

string convert(const string& path) {
    string result;
    for(char c : path) {
        if(c == '\\') {
            result += "\\\\";
        } else {
            result += c;
        }
    }
    return result;
}//将字符串中的单斜杠替换成双斜杠

void DatabaseBuild(string time)
{
     ofstream osql;
     osql.open("sql_.sql");
     osql << "create database FileInfo;" <<endl;
     osql << "use FileInfo;" << endl;

     osql << "create table directories("
          << "id int primary key, "
          << "preid int, "
          << "filename varchar(200),"
          << "path varchar(250), "
          << "filetime varchar(19)"
          << ",foreign key (preid) references directories(id)"
          << ");" << endl;

     osql << "create table files("
           << "id int primary key, "
          << "preid int, "
          << "filename varchar(200),"
          << "path varchar(250), "
          << "filesize int, "
          << "filetime varchar(19),"
          << "filetype varchar(200)"
          << ",foreign key (preid) references directories(id)"
          << ");" << endl;

     osql << "insert into directories " 
          <<"value(" 
          << 0 << ",NULL,'Windows','C:\\\\Windows','"
          << time.substr(0,19) << "');"
          << endl;

     osql.close();
}//数据库初始化

void insert_directories(string filename, string filetime, 
               int num, string filepath,int pre)
{
     ofstream osql;
     osql.open("sql_dir.sql", ios::app);
     osql << "insert into directories " 
          <<"value(" 
          << num << ","
          << pre << ",'"
          << filename << "','"
          << convert(filepath) << "','"
          << filetime.substr(0,19) << "');"
          << endl;
     osql.close();
     return ;   
}//插入目录信息

void insert_file(string filename, string filetime, 
               int num, string filepath, int filesize, int pre, string filetype)
{
     ofstream osql;
     osql.open("sql_file.sql", ios::app);
     osql << "insert into files " 
          <<"value(" 
          << num << ","
          << pre << ",'"
          << filename << "','"
          << convert(filepath) << "',"
          << filesize << ",'"
          << filetime.substr(0,19) << "','"
          << filetype << "');"
          << endl;
     osql.close();
     return ;   
}//插入文件信息