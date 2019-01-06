/* 
 * Last Updated at [2016/12/23 15:17] by wuhao
 */
#include "GeoPoint.h"
#include <math.h>

double GeoPoint::geoScale = 1.0; // 6371004 * 3.141592965 / 180;  @2016/6/28 ���ڵ�λ�Ѿ������ˣ�����Ҫ����scale����
double GeoPoint::R = 6371004;
double GeoPoint::LAT = 0;


GeoPoint::GeoPoint()
{
	this->lat = 0;
	this->lon = 0;
	this->time = INVALID_TIME;
	this->mmRoadId = INVALID_ROAD_ID;
}

GeoPoint::GeoPoint(double _lat, double _lon, int _time, int _mmRoadId, bool use_geo /* = false */)
{
	if (use_geo)
		loadGeoPos(_lat, _lon, true, POS_ORI);
	else
	{
		this->lat = _lat;
		this->lon = _lon;
	}
	this->time = _time;
	this->mmRoadId = _mmRoadId;
}

GeoPoint::GeoPoint(double _lat, double _lon, int _time, bool use_geo /* = false */)
{
	if (use_geo)
		loadGeoPos(_lat, _lon, true, POS_ORI);
	else
	{
		this->lat = _lat;
		this->lon = _lon;
	}
	this->time = _time;
	this->mmRoadId = INVALID_ROAD_ID;
}

GeoPoint::GeoPoint(double _lat, double _lon, bool use_geo /* = false */)
{
	if (use_geo)
		loadGeoPos(_lat, _lon, true, POS_ORI);
	else
	{
		this->lat = _lat;
		this->lon = _lon;
	}
	this->time = INVALID_TIME;
	this->mmRoadId = INVALID_ROAD_ID;
}

GeoPoint::GeoPoint(std::pair<double, double>& lat_lon_pair, bool use_geo /* = false */)
{
	if (use_geo)
		loadGeoPos(lat_lon_pair.first, lat_lon_pair.second, true, POS_ORI);
	else
	{
		this->lat = lat_lon_pair.first;
		this->lon = lat_lon_pair.second;
	}	
	this->time = INVALID_TIME;
	this->mmRoadId = INVALID_ROAD_ID;
}

GeoPoint GeoPoint::geo2rect(double lat_geo, double lon_geo)
{
	////use MillierConvertion
	//double L = 6381372 * 3.14159265359 * 2;//�����ܳ�  
	//double W = L;// ƽ��չ����x������ܳ�  
	//double H = L / 2;// y��Լ�����ܳ�һ��  
	//double mill = 2.3;// ����ͶӰ�е�һ����������Χ��Լ������2.3֮��  
	//double x = lon_geo * 3.14159265359 / 180;// �����ȴӶ���ת��Ϊ����  
	//double y = lat_geo * 3.14159265359 / 180;// ��γ�ȴӶ���ת��Ϊ����  
	//y = 1.25 * log(tan(0.25 * 3.14159265359 + 0.4 * y));// ����ͶӰ��ת��  
	//// ����תΪʵ�ʾ���  
	//x = (W / 2) + (W / (2 * 3.14159265359)) * x;
	//y = (H / 2) - (H / (2 * mill)) * y;
	
	//͵���� lat_geo = 0, lon_geo = 0 ��λֱ������(0, 0)
	double y = 3.14159265359 * R / 180 * lat_geo;
	double x = 3.14159265359 * R * cos(LAT*3.14159265359 / 180) / 180 * lon_geo;
	/*std::printf("cos(%lf * 3,14 / 180) = %lf\n", lat_geo, cos(lat_geo*3.14159265359 / 180));
	printf("3.14159265359 * R * cos(lat_geo*3.14159265359 / 180) / 180 * %lf = %lf\n", lon_geo, x);
	system("pause");*/

	//ƽ��һ�£��Է������С
	GeoPoint pt;
	pt.lat = y - 5000000; //lat_geo 90��ʱ���10,000,000
	pt.lon = x - 7000000; //lon_geo 180��, LAT 45��ʱ���14,000,000
	pt.lat_geo = lat_geo;
	pt.lon_geo = lon_geo;
	return pt;
}

GeoPoint GeoPoint::rect2geo(double lat, double lon)
{
	GeoPoint pt;
	pt.lat_geo = 180 / 3.14159265359 / R * (lat + 5000000);
	pt.lon_geo = 180 / (3.14159265359 * R * cos(LAT*3.14159265359 / 180)) * (lon + 7000000);
	pt.lat = lat;
	pt.lon = lon;
	return pt;
}

void GeoPoint::loadGeoPos(double _lat_geo, double _lon_geo, bool doTrans /* = true */, PosField overwrite_field /* = POS_ORI */)
{
	this->lat_geo = _lat_geo;
	this->lon_geo = _lon_geo;
	if (doTrans)
	{
		GeoPoint rectPos = geo2rect(_lat_geo, _lon_geo);
		this->lat_ref(overwrite_field) = rectPos.lat;
		this->lon_ref(overwrite_field) = rectPos.lon;
	}
}

double GeoPoint::distM(double lat1, double lon1, double lat2, double lon2)
{
	/*double deg2rad = 3.1415926 / 180;
	double X1 = lon1 * deg2rad;
	double X2 = lon2 * deg2rad;
	double Y1 = lat1 * deg2rad;
	double Y2 = lat2 * deg2rad;*/

	//return 6371000 * acos(cos(Y1) * cos(Y2) * cos(X1 - X2) + sin(Y1)*sin(Y2));
	return sqrt((lat1 - lat2) * (lat1 - lat2) + (lon1 - lon2) * (lon1 - lon2));// *GeoPoint::geoScale * cos(lat1 * deg2rad);
}

double GeoPoint::distM(GeoPoint& pt1, GeoPoint& pt2)
{
	return GeoPoint::distM(pt1.lat, pt1.lon, pt2.lat, pt2.lon);
}

double GeoPoint::distM(GeoPoint* pt1, GeoPoint* pt2)
{
	return GeoPoint::distM(pt1->lat, pt1->lon, pt2->lat, pt2->lon);
}

double GeoPoint::distM(double lat, double lon)
{
	return GeoPoint::distM(this->lat, this->lon, lat, lon);
	//double lat1 = this->lat;
	//double lon1 = this->lon;
	//return sqrt((lat1 - lat) * (lat1 - lat) + (lon1 - lon) * (lon1 - lon)) * GeoPoint::geoScale;
}

double GeoPoint::distM(GeoPoint& pt)
{
	return distM(pt.lat, pt.lon);
}

double GeoPoint::distM(GeoPoint* pt)
{
	return distM(pt->lat, pt->lon);
}

double GeoPoint::distDeg(double lat1, double lon1, double lat2, double lon2)
{
	return sqrt((lat1 - lat2) * (lat1 - lat2) + (lon1 - lon2) * (lon1 - lon2));
}

double GeoPoint::distDeg(GeoPoint pt1, GeoPoint pt2)
{
	return GeoPoint::distDeg(pt1.lat, pt1.lon, pt2.lat, pt2.lon);
}

double GeoPoint::distDeg(GeoPoint* pt1, GeoPoint* pt2)
{
	return GeoPoint::distDeg(pt1->lat, pt1->lon, pt2->lat, pt2->lon);
}

double GeoPoint::distDeg(double lat, double lon)
{
	return GeoPoint::distDeg(this->lat, this->lon, lat, lon);
}

double GeoPoint::distDeg(GeoPoint& pt)
{
	return distDeg(this->lat, this->lon, pt.lat, pt.lon);
}

double GeoPoint::distDeg(GeoPoint* pt)
{
	return distDeg(this->lat, this->lon, pt->lat, pt->lon);
}

double GeoPoint::speedMps(GeoPoint* pt1, GeoPoint* pt2)
{
	if (pt1->time - pt2->time == 0)
	{
		puts("[ERROR]@GeoPoint::speedMps(): pt1.time == pt2.time");
		system("pause");
	}
	return distM(pt1, pt2) / abs(pt1->time - pt2->time);
}

double GeoPoint::speedMps(GeoPoint& pt1, GeoPoint& pt2)
{
	if (pt1.time - pt2.time == 0)
	{
		puts("[ERROR]@GeoPoint::speedMps(): pt1.time == pt2.time");
		system("pause");
	}
	return distM(pt1, pt2) / abs(pt1.time - pt2.time);
}

void GeoPoint::refreshGeo()
{
	//////////////////////////////////////////////////////////////////////////
	///����ʵ�ʴ��������п���������ʱֱ�Ӹ���lat, lon�ֶΣ�����lat_geo, lon_geo���Ǿɵ�λ����ɲ�һ��
	///��������Ҫ���geo�����ʱ�����ʹ�ô˺������¶�geo������ݵ�ǰrect������и���
	///[TODO]��������̫2��ֻ�Ǹ���ʱ�����������Ҫ��lat,lon˽�л����ڸ���rect���괦�������һ��setter����setter��ͬʱ����geo����
	//////////////////////////////////////////////////////////////////////////
	GeoPoint tmp = GeoPoint::rect2geo(this->lat, this->lon);
	this->lat_geo = tmp.lat_geo;
	this->lon_geo = tmp.lon_geo;
}

void GeoPoint::print(bool printRectCoord /* = false */)
{
	this->refreshGeo();
	printf("lat = %lf, lon = %lf\n", this->lat_geo, this->lon_geo);
	if (printRectCoord) 
		printf("lat_rect = %lf, lon_rect = %lf\n\n", this->lat, this->lon);
}