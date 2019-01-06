#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <map>
#include <set>
#include <cstdlib>
#include "GeoPoint.h"
#include "Map.h"

struct TPROData{//一条路径的各种信息
	std::vector<int> edges;//存放路径边序
	std::vector<int> pointsData;//存放路径点序
	std::vector<int> popularityData;//存放路径热门度序列
	int popularitySum;//存放热门度序列的和
	int id;//路径ID
	double routeLength;//路径长度，用于去重
	TPROData(std::vector<int> &inputpoints, std::vector<int> &inputedges, long long inputid, double length);//构造函数。points: 路径点序, edges: 路径边序, id: 路径ID, length: 路径长度
	TPROData(std::string &status);//构造函数。传入保存的字符串恢复状态
	std::string saveStatus();//保存数据状态。将结果通过string返回。保存edges, pointsData, popularityData, popularitySum, id, routeLength
	bool operator< (TPROData &k) const{//重载小于号，用于基于热门度序列的sort
		for (int i = 0;; i++){
			if (i >= popularityData.size() || i >= k.popularityData.size()) break;
			if (popularityData[i] > k.popularityData[i]) return 1;
			if (popularityData[i] < k.popularityData[i]) return 0;
		}
		if (popularityData.size() < k.popularityData.size()) return 1;
		return 0;
	}
};

struct TPRO{
	//std::vector<std::vector<int>> data;
	TPRO(Map *map, Area *area, int lonCut, int latCut, int topK);//构造函数。输入map, area, lonCut:分组时将经度分的块数, latCut:分组时将纬度分的块数, topK:最终选取前topK热门的路径
	TPRO(Map *map, Area *area, std::string &status);//构造函数。输入map, area, status。从status字符串内载入训练信息。需要人工保证保存的训练信息和map, area对应
	std::vector<std::vector<int>> setTrainData(std::string fileName);//输入训练文件，文件为scv格式，一行一个路径，边序列。返回文件包含的路径的vector，每条路径用一个vector表示
	void setTrainData(std::vector<std::vector<int>> &routeData);//输入训练数据，routeData:路径的vector，每条路径用一个vector表示，路径为边序列
	std::vector<int> getPopularitySum();//给出每条路径的热门度的值，按照训练数据输入的时候的顺序
	std::vector<std::vector<int>> getTrainData();//给出训练数据集，每条路径用一个vector表示，内为边序列
	std::vector<double> getAnomalousScore(std::vector<std::vector<int>> &routes);//输入若干条路径数据，返回每条路径的异常值，为0-1之间的一个实数。routes:路径的vector，每条路径用一个vector表示，路径为边序列
	void getTopKRouteByRoute(std::vector<int> &inputRoute, std::vector<std::vector<int>> &result);//输入一条路径，输出这条路径所在块的topK路径的vector。inputRoute:输入路径，边序列, result:输出topK的路径，边序列
	void getTopKRouteByRoute(std::vector<int> &inputRoute, std::vector<std::vector<int>> &result, std::vector <int> &popularityResult);//输入一条路径，输出这条路径所在块的topK路径的vector。inputRoute:输入路径，边序列, result:输出topK的路径，边序列, popularityResult:输出每条路径热门度的结果
	int inline getGroupId(int pointId);//得到点所属的区域。
	std::string saveStatus();//将目前的训练结果保存。会保存dLon, dLat, lonNum, latNum, topK, groupData, topKData。map, area为指针不会保存。将结果作为string返回

public:
	Map *map;
	Area *area;
	double dLon, dLat;//每块区域的经纬度大小。转换后的值（非"_geo"）
	int lonNum, latNum;//将经纬度划分成多少块
	int topK;//保留热门度最高的路径的条数
	std::vector<std::vector<int>> editDistanceDP;//求编辑距离的动态规划用数组
	std::map<int, std::map<int, std::vector<TPROData>>> groupData;//分组后存放每个组的路径信息。前两维为起点和终点的区域号，使用map。然后在其中存储TPROData类型的vector
	std::map<int, std::map<int, std::vector<TPROData>>> topKData;//存放每个组的TopK条路径的信息。前两维为起点和终点的区域号，使用map。然后在其中存储TPROData类型的vector
	double editDistance(std::vector<int> &a, std::vector<int> &b);//求两条路径的编辑距离。输入为点序列
};

namespace TPROFunc{
	void drawData(std::vector<double> & data, int delta = 20);//在命令行中绘制数据分布。data:数据内容, delta:划分成几个部分
	void TPROTest(int N, int M, int K, bool testAll = true, bool showRandomMap = false, bool fixedGroup = false);//读取 处理 求解瞬间完成，是测试里的豪杰。（然而并不能瞬间完成，甚至还会编译错误）N:经度划分块数, M:纬度划分块数, K:topK路径条数, testAll:是否对每条路径求异常值, showRandomMap:是否创建PNG图片观测效果, fixedGroup:创建图片所选路径是否均为一个集合
	void TPROQuickTest(std::string inputFileName, bool testAll = true, bool showRandomMap = false, bool fixedGroup = false);//读取 处理 求解瞬间完成，是测试里的另一个豪杰。（然而硬盘太烂甚至比TPROTest还要慢）inputFileName:状态文件的位置, testAll:是否对每条路径求异常值, showRandomMap:是否创建PNG图片观测效果, fixedGroup:创建图片所选路径是否均为一个集合
	void makePNGByRoute(Map *map, TPRO *tpro, std::vector<int> &route, std::string filePath, std::string filePrefix = "");//绘制一张包含若干轨迹的图片。map:Map的指针, tpro:TPRO类的指针, route:一条路径，边序列，将会在图片中画上这条路径和这条路径所在组的topK路径, filePath:保存文件的位置，为一个文件夹路径, filePrefix:保存文件名的前缀。默认会在文件名后添加maxPopularity和anomalousRate，其余信息可以附加在此
	inline void getIntFromString(const char *buffer, int &i);//sscanf会调用strlen太慢了，手写从字符串里读取int型
	inline void getDoubleFromString(const char *buffer, double &i);//sscanf会调用strlen太慢了，手写从字符串里读取double型
}