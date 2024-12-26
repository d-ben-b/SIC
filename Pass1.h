#ifndef PASS1_H
#define PASS1_H

#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <string>

using namespace std;

// 全域變數
extern vector<pair<string, pair<string, pair<string, string>>>> arr;
extern string program_name, str, starting_address;
extern map<string, string> labels;
extern vector<string> object_code;

// 函數宣告
void input();
int hexToDec(string str);
string decToHex(int num);
string add(string str, string adder, int flag);
void addressing();
void generate_object_code();

#endif
