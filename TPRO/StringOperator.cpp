/* 
 * Last Updated at [2016/1/26 14:18] by wuhao
 */
#include "StringOperator.h"

double StringOperator::precision = 1e-6;

double StringOperator::stringToDouble(const string str)
{
	//TODO:异常值检测

	if (str[0] == '-')
	{
		return  -1.0 * stringToPositiveDouble(str, 1, str.size());
	}
	else
	{
		return stringToPositiveDouble(str, 0, str.size());
	}
}


string StringOperator::doubleToString(const double value)
{
	string sign = "";

	double tempValue = value;
	if (tempValue < 0)
	{
		sign = "-";
		tempValue *= -1.0;
	}

	int integerValue = static_cast<int>(tempValue);
	double decimalValue = tempValue - integerValue;

	string integerPart = intToString(integerValue);
	string decimalPart = "";

	double base = 0.1;
	while (decimalValue > precision)
	{
		int times = static_cast<int>(decimalValue / base);
		decimalPart += '0' + times;
		decimalValue -= times * base;
		base /= 10.0;
	}
	if (abs(int(value) - value) < precision)
		return sign + integerPart;
	else
		return sign + integerPart + "." + decimalPart;
}

void StringOperator::setPrecision(const double newPrecision)
{
	precision = newPrecision;
}

string StringOperator::intToString(const int value)
{
	if (value == 0)
	{
		return "0";
	}

	string result = "";
	int tempValue = value;

	if (tempValue < 0)
	{
		result += "-";
		tempValue *= -1;
	}

	stack<int> number;	//存储输入的每一位，用于倒序返回
	while (tempValue > 0)
	{
		number.push(tempValue % 10);
		tempValue /= 10;
	}
	while (!number.empty())
	{
		int topNumber = number.top();
		number.pop();
		result += '0' + topNumber;
	}

	return result;
}

int StringOperator::stringToInt(const string str)
{
	//TODO:异常值检测

	if (str[0] == '-')
	{
		return  -1 * stringToPositiveInt(str, 1, str.size());
	}
	else
	{
		return stringToPositiveInt(str, 0, str.size());
	}
}

int StringOperator::stringToPositiveInt(const string str, const int startIndex, const int endIndex)
{
	int value = 0;
	for (int i = startIndex; i != endIndex; ++i)
	{
		value = value * 10 + (str[i] - '0');
	}
	return value;
}

double StringOperator::stringToPositiveDouble(const string str, const int startIndex, const int endIndex)
{

	int integerValue = 0;
	int pointPos = 0;	//小数点位置
	//计算整数部分的值并求出小数点位置
	for (int i = startIndex; i != endIndex; ++i)
	{
		if (str[i] == '.')
		{
			pointPos = i;
			break;
		}

		integerValue = integerValue * 10 + (str[i] - '0');
	}

	double decimalValue = 0.0;
	//计算小数部分的值
	double base = 10.0;
	for (int i = pointPos + 1; i != endIndex; ++i)
	{
		decimalValue += (str[i] - '0') / base;
		base *= 10;
	}

	return static_cast<double>(integerValue)+decimalValue;
}

void StringOperator::split(const string& src, const string& separator, vector<string>& dest)
{
	std::string str = src;
	std::string substring;
	std::string::size_type start = 0, index;
	do
	{
		index = str.find_first_of(separator, start);
		if (index != std::string::npos)
		{
			substring = str.substr(start, index - start);
			dest.push_back(substring);
			start = str.find_first_not_of(separator, index);
			if (start == std::string::npos) return;
		}
	} while (index != std::string::npos);
	//the last token
	substring = str.substr(start);
	dest.push_back(substring);
}

void StringOperator::split(const string& src, const char& separator, vector<string>& dest)
{
	int index = 0, start = 0;
	while (index != src.size())
	{
		if (src[index] == separator)
		{
			string substring = src.substr(start, index - start);
			dest.push_back(substring);
			while (src[index + 1] == separator)
			{
				dest.push_back("");
				index++;
			}
			index++;
			start = index;
		}
		else
			index++;
	}
	//the last token
	string substring = src.substr(start);
	dest.push_back(substring);

}
