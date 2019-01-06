/* 
 * Last Updated at [2016/12/23 15:!7] by wuhao
 * version 2.1.3
 * comments:������rect2geo��һ��Сbug @2016/12/23
 *			����speedMps���� @2016/12/06
 *			�޸���Area�е�setArea�е�TODO���Լ���getArea������ΪsetArea @2016/11/30
 *			�޸���print()�����Լ�������rect2geo��refreshGeo���� @2016/11/16
 *			�Եײ�ı�����޸� @2016/6/29
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
	double dist; //��չ�ֶ�
	//double lmd; //��չ�ֶ�
	int isOutlier; //��չ�ֶ�
	//bool isStayPoint; //��չ�ֶ�
	//double direction; //��չ�ֶ�
	//int clusterId = -1; //��չ�ֶ�
	int mmRoadId2; //��չ�ֶ�
	double speed; //��չ�ֶ�
	double lat_gt; //��չ�ֶ�
	double lon_gt; //��չ�ֶ�
	double lat_result; //��չ�ֶ�
	double lon_result; //��չ�ֶ�
	int mmRoadId_gt = -1; //��չ�ֶ�
	bool visited = false; //��չ�ֶ� 

	double lat_geo;
	double lon_geo;

	//���캯����use_geo = true��ʾ�����lat,lon�ǵ������꣬���캯�����Զ�ת����ֱ���������lat,lon�ֶΣ��������������lat_geo,lon_geo�ֶΡ���ȡ�ļ�ʱuse_geo���Ϊtrue
	GeoPoint();
	GeoPoint(double lat, double lon, int time, int mmRoadId, bool use_geo = false);
	GeoPoint(double lat, double lon, int time, bool use_geo = false);
	GeoPoint(double lat, double lon, bool use_geo = false);
	GeoPoint(std::pair<double, double>& lat_lon_pair, bool use_geo = false);
	
	void loadGeoPos(double lat_geo, double lon_geo, bool doTrans = true, PosField overwrite_field = POS_ORI); //��ȡ��ʵ���꣬doTransΪtrue�����Զ�����ʵ����ת��Ϊֱ�����꣬��д��overwrite_field�ֶ�
	static GeoPoint geo2rect(double lat_geo, double lon_geo); //����ʵ��������ת����ֱ�����ֱ꣬�����걣���ڷ��ص�GeoPoint��lat,lon�ֶ���
	static GeoPoint rect2geo(double lat, double lon); //��ֱ�����꣨lat = y, lon = x��ת��Ϊ��ʵ�������꣬�������걣���ڷ��ص�GeoPoint��lat_geo, lon_geo�ֶ���

	static double geoScale; //�ѷ���
	static double R; //����뾶
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
	
	void refreshGeo(); //���ݵ�ǰ��rect�������geo����
	void print(bool printRectCoord = false); //printRectCoordΪtrueʱͬʱ��ӡֱ������
};

//��ʾ������࣬MapDrawer��Map����ͬһ��Area�����Ա��������һ����
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
		///use_geoΪtrueʱ������Ĳ���Ϊ�������꣬�������Զ�ת����ֱ�����ꣻfalseʱ������Ĳ���Ϊֱ������
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
		///Ϊpts�㼯������һ�����ʵ�area
		///list�汾
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
			double d = 0.0002; //�߽����
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
			double d = 500; //�߽����@2016/6/28 ����������ֱ������ϵ����λ����
			this->setArea(minLat - d, maxLat + d, minLon - d, maxLon + d, false);
		}		
		this->print(); //��ӡ��Ļ�����¼���´β����ٴζ�����
	}

	void setArea(std::vector<GeoPoint*>& pts, bool use_geo)
	{
		//////////////////////////////////////////////////////////////////////////
		///Ϊpts�㼯������һ�����ʵ�area
		///vector�汾
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
			double d = 0.0002; //�߽����
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
			double d = 500; //�߽����@2016/6/28 ����������ֱ������ϵ����λ����
			this->setArea(minLat - d, maxLat + d, minLon - d, maxLon + d, false);
		}
		this->print(); //��ӡ��Ļ�����¼���´β����ٴζ�����
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
		//����(lat,lon)�Ƿ��ڸ�������
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