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

struct TPROData{//һ��·���ĸ�����Ϣ
	std::vector<int> edges;//���·������
	std::vector<int> pointsData;//���·������
	std::vector<int> popularityData;//���·�����Ŷ�����
	int popularitySum;//������Ŷ����еĺ�
	int id;//·��ID
	double routeLength;//·�����ȣ�����ȥ��
	TPROData(std::vector<int> &inputpoints, std::vector<int> &inputedges, long long inputid, double length);//���캯����points: ·������, edges: ·������, id: ·��ID, length: ·������
	TPROData(std::string &status);//���캯�������뱣����ַ����ָ�״̬
	std::string saveStatus();//��������״̬�������ͨ��string���ء�����edges, pointsData, popularityData, popularitySum, id, routeLength
	bool operator< (TPROData &k) const{//����С�ںţ����ڻ������Ŷ����е�sort
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
	TPRO(Map *map, Area *area, int lonCut, int latCut, int topK);//���캯��������map, area, lonCut:����ʱ�����ȷֵĿ���, latCut:����ʱ��γ�ȷֵĿ���, topK:����ѡȡǰtopK���ŵ�·��
	TPRO(Map *map, Area *area, std::string &status);//���캯��������map, area, status����status�ַ���������ѵ����Ϣ����Ҫ�˹���֤�����ѵ����Ϣ��map, area��Ӧ
	std::vector<std::vector<int>> setTrainData(std::string fileName);//����ѵ���ļ����ļ�Ϊscv��ʽ��һ��һ��·���������С������ļ�������·����vector��ÿ��·����һ��vector��ʾ
	void setTrainData(std::vector<std::vector<int>> &routeData);//����ѵ�����ݣ�routeData:·����vector��ÿ��·����һ��vector��ʾ��·��Ϊ������
	std::vector<int> getPopularitySum();//����ÿ��·�������Ŷȵ�ֵ������ѵ�����������ʱ���˳��
	std::vector<std::vector<int>> getTrainData();//����ѵ�����ݼ���ÿ��·����һ��vector��ʾ����Ϊ������
	std::vector<double> getAnomalousScore(std::vector<std::vector<int>> &routes);//����������·�����ݣ�����ÿ��·�����쳣ֵ��Ϊ0-1֮���һ��ʵ����routes:·����vector��ÿ��·����һ��vector��ʾ��·��Ϊ������
	void getTopKRouteByRoute(std::vector<int> &inputRoute, std::vector<std::vector<int>> &result);//����һ��·�����������·�����ڿ��topK·����vector��inputRoute:����·����������, result:���topK��·����������
	void getTopKRouteByRoute(std::vector<int> &inputRoute, std::vector<std::vector<int>> &result, std::vector <int> &popularityResult);//����һ��·�����������·�����ڿ��topK·����vector��inputRoute:����·����������, result:���topK��·����������, popularityResult:���ÿ��·�����ŶȵĽ��
	int inline getGroupId(int pointId);//�õ�������������
	std::string saveStatus();//��Ŀǰ��ѵ��������档�ᱣ��dLon, dLat, lonNum, latNum, topK, groupData, topKData��map, areaΪָ�벻�ᱣ�档�������Ϊstring����

public:
	Map *map;
	Area *area;
	double dLon, dLat;//ÿ������ľ�γ�ȴ�С��ת�����ֵ����"_geo"��
	int lonNum, latNum;//����γ�Ȼ��ֳɶ��ٿ�
	int topK;//�������Ŷ���ߵ�·��������
	std::vector<std::vector<int>> editDistanceDP;//��༭����Ķ�̬�滮������
	std::map<int, std::map<int, std::vector<TPROData>>> groupData;//�������ÿ�����·����Ϣ��ǰ��άΪ�����յ������ţ�ʹ��map��Ȼ�������д洢TPROData���͵�vector
	std::map<int, std::map<int, std::vector<TPROData>>> topKData;//���ÿ�����TopK��·������Ϣ��ǰ��άΪ�����յ������ţ�ʹ��map��Ȼ�������д洢TPROData���͵�vector
	double editDistance(std::vector<int> &a, std::vector<int> &b);//������·���ı༭���롣����Ϊ������
};

namespace TPROFunc{
	void drawData(std::vector<double> & data, int delta = 20);//���������л������ݷֲ���data:��������, delta:���ֳɼ�������
	void TPROTest(int N, int M, int K, bool testAll = true, bool showRandomMap = false, bool fixedGroup = false);//��ȡ ���� ���˲����ɣ��ǲ�����ĺ��ܡ���Ȼ��������˲����ɣ���������������N:���Ȼ��ֿ���, M:γ�Ȼ��ֿ���, K:topK·������, testAll:�Ƿ��ÿ��·�����쳣ֵ, showRandomMap:�Ƿ񴴽�PNGͼƬ�۲�Ч��, fixedGroup:����ͼƬ��ѡ·���Ƿ��Ϊһ������
	void TPROQuickTest(std::string inputFileName, bool testAll = true, bool showRandomMap = false, bool fixedGroup = false);//��ȡ ���� ���˲����ɣ��ǲ��������һ�����ܡ���Ȼ��Ӳ��̫��������TPROTest��Ҫ����inputFileName:״̬�ļ���λ��, testAll:�Ƿ��ÿ��·�����쳣ֵ, showRandomMap:�Ƿ񴴽�PNGͼƬ�۲�Ч��, fixedGroup:����ͼƬ��ѡ·���Ƿ��Ϊһ������
	void makePNGByRoute(Map *map, TPRO *tpro, std::vector<int> &route, std::string filePath, std::string filePrefix = "");//����һ�Ű������ɹ켣��ͼƬ��map:Map��ָ��, tpro:TPRO���ָ��, route:һ��·���������У�������ͼƬ�л�������·��������·���������topK·��, filePath:�����ļ���λ�ã�Ϊһ���ļ���·��, filePrefix:�����ļ�����ǰ׺��Ĭ�ϻ����ļ��������maxPopularity��anomalousRate��������Ϣ���Ը����ڴ�
	inline void getIntFromString(const char *buffer, int &i);//sscanf�����strlen̫���ˣ���д���ַ������ȡint��
	inline void getDoubleFromString(const char *buffer, double &i);//sscanf�����strlen̫���ˣ���д���ַ������ȡdouble��
}