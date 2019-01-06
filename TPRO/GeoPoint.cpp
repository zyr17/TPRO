/* 
 * Last Updated at [2016/12/23 15:17] by wuhao
 */
#include "GeoPoint.h"
#include <math.h>

double GeoPoint::geoScale = 1.0; // 6371004 * 3.141592965 / 180;  @2016/6/28 现在单位已经是米了，不需要进行scale缩放
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
	//double L = 6381372 * 3.14159265359 * 2;//地球周长  
	//double W = L;// 平面展开后，x轴等于周长  
	//double H = L / 2;// y轴约等于周长一半  
	//double mill = 2.3;// 米勒投影中的一个常数，范围大约在正负2.3之间  
	//double x = lon_geo * 3.14159265359 / 180;// 将经度从度数转换为弧度  
	//double y = lat_geo * 3.14159265359 / 180;// 将纬度从度数转换为弧度  
	//y = 1.25 * log(tan(0.25 * 3.14159265359 + 0.4 * y));// 米勒投影的转换  
	//// 弧度转为实际距离  
	//x = (W / 2) + (W / (2 * 3.14159265359)) * x;
	//y = (H / 2) - (H / (2 * mill)) * y;
	
	//偷懒， lat_geo = 0, lon_geo = 0 定位直角坐标(0, 0)
	double y = 3.14159265359 * R / 180 * lat_geo;
	double x = 3.14159265359 * R * cos(LAT*3.14159265359 / 180) / 180 * lon_geo;
	/*std::printf("cos(%lf * 3,14 / 180) = %lf\n", lat_geo, cos(lat_geo*3.14159265359 / 180));
	printf("3.14159265359 * R * cos(lat_geo*3.14159265359 / 180) / 180 * %lf = %lf\n", lon_geo, x);
	system("pause");*/

	//平移一下，以防过大过小
	GeoPoint pt;
	pt.lat = y - 5000000; //lat_geo 90度时差不多10,000,000
	pt.lon = x - 7000000; //lon_geo 180度, LAT 45度时差不多14,000,000
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
	///由于实际代码运行中可能在运行时直接更改lat, lon字段，导致lat_geo, lon_geo还是旧的位置造成不一致
	///所以在需要输出geo坐标的时候可以使用此函数重新对geo坐标根据当前rect坐标进行更新
	///[TODO]：这个设计太2，只是个临时补丁，今后需要将lat,lon私有化，在更改rect坐标处独立设计一个setter，在setter中同时更新geo坐标
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