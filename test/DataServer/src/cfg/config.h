#ifndef _GET_CONFIG_H_
#define _GET_CONFIG_H_
 
#include <string>
#include <map>
using namespace std;
 
#define COMMENT_CHAR '#'
 
map<string, string>  ReadConfig(const string & filename);
void PrintConfig(const map<string, string> & m);
#endif