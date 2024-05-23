#include<iostream>
#include<fstream>
#include<string>

using namespace std;

string convert(const string& path);

void DatabaseBuild(string time);

void insert_directories(string filename, string filetime, 
               int num, string filepath,int pre);

void insert_file(string filename, string filetime, 
               int num, string filepath, int filesize, int pre, string filetype);