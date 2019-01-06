/* 
 * Last Updated at [2016/1/26 14:17] by wuhao
 * Ver 1.1.0 
 * 20160126 ������split����
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
	static double stringToPositiveDouble(const string str, const int startIndex, const int endIndex);	//��str[startIndex...endIndex]�Ӵ�ת��Ϊ��������
	static int stringToPositiveInt(const string str, const int startIndex, const int endIndex);	////��str[startIndex...endIndex]�Ӵ�ת��Ϊ������
	
	static double precision;//������ת�ַ���ʱ�ľ������ã�1e��-n����ʾ��ȷ��С�����nλ
};
#endif