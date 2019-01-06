#pragma once
#include <iostream>
#include <list>
#include <vector>
#include <algorithm>
#include "GeoPoint.h"
#include "MapDrawer.h"
using namespace std;

class PointGridIndex
{
public:
	void createIndex(list<GeoPoint*>& pts, Area* area, int gridWidth);
	pair<int, int> getRowCol(GeoPoint* pt);
	void drawGridLine(Gdiplus::Color color, MapDrawer& md);
	void getNearPts(GeoPoint* pt, double thresholdM, vector<GeoPoint*>& dest); //������pt������thresholdM�׵����е����dest
	void getNearPts(GeoPoint* pt, int gridRange, vector<GeoPoint*>& dest); //����pt��������Ϊ���ģ��߳�Ϊ2*gridRange+1���е����е����dest
	void kNN_approx(GeoPoint* pt, int k, double thresholdM, vector<GeoPoint*>& dest);
	void kNN_exact(GeoPoint* pt, int k, vector<GeoPoint*>& dest);

//private:
	list<GeoPoint*>* **grid;
	int gridWidth;
	int gridHeight;
	double gridSizeDeg; //��¼gird�߳���������ʵ�ʾ��루��λΪ�ȣ�
	Area* area;

	void initialization();
	void insertOnePt(GeoPoint* pt);

};