/* 
 * Last Updated at [2016/12/23 15:!7] by wuhao
 * version 2.1.3
 * comments:修正了rect2geo的一个小bug @2016/12/23
 *			增加speedMps函数 @2016/12/06
 *			修改了Area中的setArea中的TODO，以及将getArea重命名为setArea @2016/11/30
 *			修改了print()函数以及增加了rect2geo，refreshGeo函数 @2016/11/16
 *			对底层改变进行修改 @2016/6/29
 */
#pragma once
#include <iostream>
#include <list>
#include <vector>
#include <math.h>
using namespace std;
#define INVALID_TIME -999
#define INVALID_ROAD_ID -999
#define INF 1e20

enum PosField
{
	POS_ORI,
	POS_RESULT,
	POS_GT,
};

enum MMRoadIdField
{
	MMROADID,
	MMROADID2,
	MMROADID_GT,
};

class GeoPoint
{
public:
	double lat;
	double lon;
	int time;
	int mmRoadId;
	double dist; //扩展字段
	//double lmd; //扩展字段
	int isOutlier; //扩展字段
	//bool isStayPoint; //扩展字段
	//double direction; //扩展字段
	//int clusterId = -1; //扩展字段
	int mmRoadId2; //扩展字段
	double speed; //扩展字段
	double lat_gt; //扩展字段
	double lon_gt; //扩展字段
	double lat_result; //扩展字段
	double lon_result; //扩展字段
	int mmRoadId_gt = -1; //扩展字段
	bool visited = false; //扩展字段 

	double lat_geo;
	double lon_geo;

	//构造函数，use_geo = true表示读入的lat,lon是地理坐标，构造函数会自动转换成直角坐标存入lat,lon字段，将地理坐标存入lat_geo,lon_geo字段。读取文件时use_geo务必为true
	GeoPoint();
	GeoPoint(double lat, double lon, int time, int mmRoadId, bool use_geo = false);
	GeoPoint(double lat, double lon, int time, bool use_geo = false);
	GeoPoint(double lat, double lon, bool use_geo = false);
	GeoPoint(std::pair<double, double>& lat_lon_pair, bool use_geo = false);
	
	void loadGeoPos(double lat_geo, double lon_geo, bool doTrans = true, PosField overwrite_field = POS_ORI); //读取真实坐标，doTrans为true代表自动将真实坐标转化为直角坐标，并写入overwrite_field字段
	static GeoPoint geo2rect(double lat_geo, double lon_geo); //将真实地理坐标转化成直角坐标，直角坐标保存在返回的GeoPoint的lat,lon字段中
	static GeoPoint rect2geo(double lat, double lon); //将直角坐标（lat = y, lon = x）转化为真实地理坐标，地理坐标保存在返回的GeoPoint的lat_geo, lon_geo字段中

	static double geoScale; //已废弃
	static double R; //地球半径
	static double LAT;
	static double distM(double lat1, double lon1, double lat2, double lon2);
	static double distM(GeoPoint& pt1, GeoPoint& pt2);
	static double distM(GeoPoint* pt1, GeoPoint* pt2);
	double distM(double lat1, double lat2);
	double distM(GeoPoint* pt);
	double distM(GeoPoint& pt);
	static double distDeg(double lat1, double lon1, double lat2, double lon2);
	static double distDeg(GeoPoint pt1, GeoPoint pt2);
	static double distDeg(GeoPoint* pt1, GeoPoint* pt2);
	double distDeg(double lat1, double lat2);
	double distDeg(GeoPoint* pt);
	double distDeg(GeoPoint& pt);
	static double speedMps(GeoPoint* pt1, GeoPoint* pt2);
	static double speedMps(GeoPoint& pt1, GeoPoint& pt2);
	static double speedKmh(GeoPoint* pt1, GeoPoint* pt2);
	static double speedKmh(GeoPoint& pt1, GeoPoint& pt2);

	double& lat_ref(PosField pos_field)
	{
		switch (pos_field)
		{
		case POS_ORI: return lat; break;
		case POS_RESULT: return lat_result; break;
		case POS_GT: return lat_gt; break;
		default: break;
		}
	}
	double& lon_ref(PosField pos_field)
	{
		switch (pos_field)
		{
		case POS_ORI: return lon; break;
		case POS_RESULT: return lon_result; break;
		case POS_GT: return lon_gt; break;
		default: break;
		}
	}
	int& mmRoadId_ref(MMRoadIdField road_field)
	{
		switch (road_field)
		{
		case MMROADID: return mmRoadId; break;
		case MMROADID2:	return mmRoadId2; break;
		case MMROADID_GT: return mmRoadId_gt; break;
		default: break;
		}
	}
	
	void refreshGeo(); //根据当前的rect坐标更新geo坐标
	void print(bool printRectCoord = false); //printRectCoord为true时同时打印直角坐标
};

//表示区域的类，MapDrawer与Map引用同一个Area对象以保持区域的一致性
class Area
{
public:
	double minLat;
	double maxLat;
	double minLon;
	double maxLon;

	double minLat_geo;
	double maxLat_geo;
	double minLon_geo;
	double maxLon_geo;

	void setArea(double _minLat, double _maxLat, double _minLon, double _maxLon, bool use_geo)
	{
		//////////////////////////////////////////////////////////////////////////
		///use_geo为true时，输入的参数为地理坐标，函数会自动转化成直角坐标；false时，输入的参数为直角坐标
		//////////////////////////////////////////////////////////////////////////
		if (use_geo)
		{
			GeoPoint minLatLon_rect = GeoPoint::geo2rect(_minLat, _minLon);
			GeoPoint maxLatLon_rect = GeoPoint::geo2rect(_maxLat, _maxLon);
			this->minLat = minLatLon_rect.lat;
			this->minLon = minLatLon_rect.lon;
			this->maxLat = maxLatLon_rect.lat;
			this->maxLon = maxLatLon_rect.lon;

			this->minLat_geo = _minLat;
			this->minLon_geo = _minLon;
			this->maxLat_geo = _maxLat;
			this->maxLon_geo = _maxLon;
		}
		else
		{
			this->minLat = _minLat;
			this->maxLat = _maxLat;
			this->minLon = _minLon;
			this->maxLon = _maxLon;

			printf("minlat = %lf, minlon = %lf\n", _minLat, _minLon);
			printf("maxlat = %lf, naxlon = %lf\n", _maxLat, _maxLon);
			GeoPoint minLatLon_geo = GeoPoint::rect2geo(_minLat, _minLon);
			GeoPoint maxLatLon_geo = GeoPoint::rect2geo(_maxLat, _maxLon);
			minLatLon_geo.print();
			maxLatLon_geo.print();
			system("pause");
			this->minLat_geo = minLatLon_geo.lat_geo;
			this->minLon_geo = minLatLon_geo.lon_geo;
			this->maxLat_geo = maxLatLon_geo.lat_geo;
			this->maxLon_geo = maxLatLon_geo.lon_geo;
		}
	}

	void setArea(std::list<GeoPoint*>& pts, bool use_geo)
	{
		//////////////////////////////////////////////////////////////////////////
		///为pts点集合设置一个合适的area
		///list版本
		//////////////////////////////////////////////////////////////////////////
		if (pts.size() < 2)
		{
			std::cout << "pts'size is too small" << std::endl;
			return;
		}
		double minLat = INF;
		double maxLat = -INF;
		double minLon = INF;
		double maxLon = -INF;

		if (use_geo)
		{
			for each (GeoPoint* pt in pts)
			{
				if (pt->lat_geo < minLat) minLat = pt->lat_geo;
				if (pt->lat_geo > maxLat) maxLat = pt->lat_geo;
				if (pt->lon_geo < minLon) minLon = pt->lon_geo;
				if (pt->lon_geo > maxLon) maxLon = pt->lon_geo;
			}
			double d = 0.0002; //边界宽松
			this->setArea(minLat - d, maxLat + d, minLon - d, maxLon + d, true);
		}
		else
		{
			for each (GeoPoint* pt in pts)
			{
				if (pt->lat < minLat) minLat = pt->lat;
				if (pt->lat > maxLat) maxLat = pt->lat;
				if (pt->lon < minLon) minLon = pt->lon;
				if (pt->lon > maxLon) maxLon = pt->lon;
			}
			double d = 500; //边界宽松@2016/6/28 现在坐标是直角坐标系，单位是米
			this->setArea(minLat - d, maxLat + d, minLon - d, maxLon + d, false);
		}		
		this->print(); //打印屏幕方便记录，下次不用再次读数据
	}

	void setArea(std::vector<GeoPoint*>& pts, bool use_geo)
	{
		//////////////////////////////////////////////////////////////////////////
		///为pts点集合设置一个合适的area
		///vector版本
		//////////////////////////////////////////////////////////////////////////
		if (pts.size() < 2)
		{
			std::cout << "pts'size is too small" << std::endl;
			return;
		}
		double minLat = INF;
		double maxLat = -INF;
		double minLon = INF;
		double maxLon = -INF;

		if (use_geo)
		{
			for each (GeoPoint* pt in pts)
			{
				if (pt->lat_geo < minLat) minLat = pt->lat_geo;
				if (pt->lat_geo > maxLat) maxLat = pt->lat_geo;
				if (pt->lon_geo < minLon) minLon = pt->lon_geo;
				if (pt->lon_geo > maxLon) maxLon = pt->lon_geo;
			}
			double d = 0.0002; //边界宽松
			this->setArea(minLat - d, maxLat + d, minLon - d, maxLon + d, true);
		}
		else
		{
			for each (GeoPoint* pt in pts)
			{
				if (pt->lat < minLat) minLat = pt->lat;
				if (pt->lat > maxLat) maxLat = pt->lat;
				if (pt->lon < minLon) minLon = pt->lon;
				if (pt->lon > maxLon) maxLon = pt->lon;
			}
			double d = 500; //边界宽松@2016/6/28 现在坐标是直角坐标系，单位是米
			this->setArea(minLat - d, maxLat + d, minLon - d, maxLon + d, false);
		}
		this->print(); //打印屏幕方便记录，下次不用再次读数据
	}

	Area()
	{
		minLat = 0;
		maxLat = 0;
		minLon = 0;
		maxLon = 0;
	}

	Area(double _minLat, double _maxLat, double _minLon, double _maxLon, bool use_geo = true)
	{
		setArea(_minLat, _maxLat, _minLon, _maxLon, use_geo);
	}

	bool inArea(double lat, double lon, bool use_geo = false)
	{
		//返回(lat,lon)是否在该区间内
		if (!use_geo)
			return (lat > minLat && lat < maxLat && lon > minLon && lon < maxLon);
		else
		{
			//GeoPoint rectPos = GeoPoint::geo2rect(lat, lon);
			//return (rectPos.lat > minLat && rectPos.lat < maxLat && rectPos.lon > minLon && rectPos.lon < maxLon);
			return (lat > minLat_geo && lat < maxLat_geo && lon > minLon_geo && lon < maxLon_geo);
		}
	}

	void print()
	{
		printf("area: minLat = %lf, maxLat = %lf, minLon = %lf, maxLon = %lf\n", minLat, maxLat, minLon, maxLon);
		printf("      minLat_geo = %lf, maxLat_geo = %lf, minLon_geo = %lf, maxLon_geo = %lf\n", minLat_geo, maxLat_geo, minLon_geo, maxLon_geo);
	}
};