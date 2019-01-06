/* 
 * Last Updated at [2016/1/26 14:17] by wuhao
 * Ver 1.1.0 
 * 20160126 增加了split函数
 */
#ifndef STRINGOPERATOR_H
#define STRINGOPERATOR_H

#include <string>
#include <stack>
#include <vector>

using namespace std;

class StringOperator
{
public:
	static double stringToDouble(const string str);
	static string doubleToString(const double value);
	static void setPrecision(const double newPrecision);

	static string intToString(const int value);
	static int stringToInt(const string str);

	static void split(const string& src, const string& separator, vector<string>& dest);
	static void split(const string& src, const char& separator, vector<string>& dest);

private:
	static double stringToPositiveDouble(const string str, const int startIndex, const int endIndex);	//将str[startIndex...endIndex]子串转换为正浮点数
	static int stringToPositiveInt(const string str, const int startIndex, const int endIndex);	////将str[startIndex...endIndex]子串转换为正整数
	
	static double precision;//浮点数转字符串时的精度设置，1e（-n）表示精确到小数点后n位
};
#endif