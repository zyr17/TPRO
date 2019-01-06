/*
* Last Updated at [2016/12/22 10:27] by wuhao
* version 2.1.1
* comments:	废弃了setArea()的一个版本的使用 @2016/12/22
*			增加画虚线功能,增加热度图功能 @2015/4/30
*/

/*
*	//使用方法
*	MapDrawer md;
*   Area area(minlat, maxlat, minlon, maxlon); //设置一个area(矩形)
*	md.setArea(area); //将画布绑定到area
*	md.setResolution(size); //图片宽度,单位为像素,高度自适应
*	md.newBitmap(); //构建新图片,默认背景为透明
*	md.lockBits(); //画图操作前必须调用这个
*	drawsth(); //所有画图操作应该加在这儿
* 	md.unlockBits(); //所有画图操作完成后加上此句以提交更改
*   md.saveBitmap("out.png"); //另存为png图片
*
*  //zooming使用方法 [已废弃]
*  size = 15000;
*  md.setArea(minLat, maxLat, minLon, maxLon)
*  md.setResolution(size);
*  size = 3000;
*  md.zoomIn(8900, 7150, 400, 300, size); //在15000宽度的图片中聚焦到(8900,7150)为左上角，400*300的小矩形中，并重设图像宽度为3000
*  md.newBitmap();
*  ......
*/
#pragma once
#include <string>
#include <vector>
#include <Windows.h>  
#include <tchar.h>  
#include <stdio.h>  
#include <gdiplus.h>
#include <io.h>
#include <fstream>
#include <iostream>
#include "GeoPoint.h"
//using namespace Gdiplus;
#pragma comment(lib,"gdiplus.lib")
#define STEPLEN 1
#define BLOCKHEIGHT 10
#define BLOCKWIDTH 10
#define DISTANCEINERBLOCK 1
#define MARGINFROMBOUNDRY 10
#define ENDLESSLINE 99999999
#define BLANK 20 //虚线空白间隔，单位像素
#define LINE 20 //虚线中实线间隔，单位像素

namespace MD
{
	static bool minus[5][4] = {
		0, 0, 0, 0,
		0, 0, 0, 0,
		1, 1, 1, 0,
		0, 0, 0, 0,
		0, 0, 0, 0
	};

	static bool num[10][5][4] = {
		1, 1, 1, 0,
		1, 0, 1, 0,
		1, 0, 1, 0,
		1, 0, 1, 0,
		1, 1, 1, 0,

		0, 0, 1, 0,
		0, 0, 1, 0,
		0, 0, 1, 0,
		0, 0, 1, 0,
		0, 0, 1, 0,

		1, 1, 1, 0,
		0, 0, 1, 0,
		1, 1, 1, 0,
		1, 0, 0, 0,
		1, 1, 1, 0,

		1, 1, 1, 0,
		0, 0, 1, 0,
		1, 1, 1, 0,
		0, 0, 1, 0,
		1, 1, 1, 0,

		1, 0, 1, 0,
		1, 0, 1, 0,
		1, 1, 1, 0,
		0, 0, 1, 0,
		0, 0, 1, 0,

		1, 1, 1, 0,
		1, 0, 0, 0,
		1, 1, 1, 0,
		0, 0, 1, 0,
		1, 1, 1, 0,

		1, 1, 1, 0,
		1, 0, 0, 0,
		1, 1, 1, 0,
		1, 0, 1, 0,
		1, 1, 1, 0,

		1, 1, 1, 0,
		0, 0, 1, 0,
		0, 0, 1, 0,
		0, 0, 1, 0,
		0, 0, 1, 0,

		1, 1, 1, 0,
		1, 0, 1, 0,
		1, 1, 1, 0,
		1, 0, 1, 0,
		1, 1, 1, 0,

		1, 1, 1, 0,
		1, 0, 1, 0,
		1, 1, 1, 0,
		0, 0, 1, 0,
		1, 1, 1, 0
	};

	static bool point[5][2] = {
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		1, 0
	};
}
enum Mode
{
	NORMAL, //正常线
	DASHLINE, //虚线
};

class MapDrawer
{
public:
	Area* area;
	int r_width; //画布像素宽度
	int r_height; //画布像素高度

	MapDrawer();
	~MapDrawer();

	//绘画函数具有鲁棒性，无需判断是否在范围内
	void drawPoint(Gdiplus::Color color, int x, int y); //在屏幕(x,y)画一个像素点
	void drawPoint(Gdiplus::Color color, double lat, double lon); //在地里坐标(lat,lon)对应的图上画一个像素点
	void drawBigPoint(Gdiplus::Color color, int x, int y); //在屏幕(x,y)画一个十字点
	void drawBigPoint(Gdiplus::Color color, double lat, double lon); //在地里坐标(lat,lon)对应的图上画一个十字点
	void drawLine(Gdiplus::Color color, int x1, int y1, int x2, int y2, Mode mode = Mode::NORMAL);
	void drawLine(Gdiplus::Color color, double lat1, double lon1, double lat2, double lon2, Mode mode = Mode::NORMAL);
	void drawBoldLine(Gdiplus::Color color, int x1, int y1, int x2, int y2, Mode mode = Mode::NORMAL);
	void drawBoldLine(Gdiplus::Color color, double lat1, double lon1, double lat2, double lon2, Mode mode = Mode::NORMAL); //画一条粗线
	void drawMap(Gdiplus::Color color, std::string mapFilePath); //画地图，mapFilePath为地图文件路径，需OSM标准格式
	static Gdiplus::Color randomColor(); //随机生成一种颜色

	void newBitmap(); //新建画布，画图前需调用，需事先设定好area和resolution
	void lockBits(); //画图前必须先调用这个，需在newBitmap()操作之后调用
	void unlockBits(); //所有画图操作结束后必须调用这个，以提交更改
	void saveBitmap(std::string fileName); //保存为png图片，fileName为文件路径
	void setArea(double _minLat, double _maxLat, double _minLon, double _maxLon); //设置轨迹有效区域，在此范围内的轨迹都会被画在画布上
	void setArea(Area* area);
	void setResolution(int width); //设置画布宽度，高度由经纬度区域按比例自动计算	
	void setResolution(int width, int height); //自定义画布，不推荐使用
	bool inArea(double lat, double lon); //判断轨迹经纬度是否在指定画图区域内
	bool inArea(int x, int y); //判断点(x,y)是否在画布范围内
	void zoomIn(int upperLeft_x, int upperLeft_y, int width, int height, int newR_width); //【只适合用于北半球！】放大区域，左上角像素坐标和矩形长宽为参数输入，newR_width为新画布宽度

	Gdiplus::Point geoToScreen(double lat, double lon); //将地理坐标转换成屏幕坐标,返回类型为Gdiplus::Point
	std::pair<double, double> screenToGeo(int screenX, int screenY); //将屏幕坐标转换成地理坐标,返回pair的first为lat,second为lon

	//new
	void drawInt(Gdiplus::Color color, int x, int y, int value);  //在屏幕坐标(x,y)画一个整数，值为value
	void drawDouble(Gdiplus::Color color, int x, int y, double Value, int precision = 6); //在屏幕坐标(x,y)画一个double，值为value，默认进度为6位

	//////////////////////////////////////////////// new begin //////////////////////////////////////////////////////////////////
	static Gdiplus::Color getColor(double value, double minvalue = 0, double maxvalue = 255);
	static Gdiplus::Color getColor(int value, int minvalue = 0, int maxvalue = 255);
	void printSample(double minvalue, double maxvalue, double startId, double endId, double stepLen = STEPLEN, int blockHeight = BLOCKHEIGHT, int blockWidth = BLOCKWIDTH, int blockMargin = DISTANCEINERBLOCK, int margin = MARGINFROMBOUNDRY);
	void printSample(int minvalue, int maxvalue, int startId, int endId, int stepLen = STEPLEN, int blockHeight = BLOCKHEIGHT, int blockWidth = BLOCKWIDTH, int blockMargin = DISTANCEINERBLOCK, int margin = MARGINFROMBOUNDRY);
	void drawBoldSquare(Gdiplus::Color color, double lat1, double lon1, double lat2, double lon2);
	//////////////////////////////////////////////// new end //////////////////////////////////////////////////////////////////

private:
	ULONG_PTR gdiplusToken;
	Gdiplus::Bitmap* bm;
	Gdiplus::BitmapData* bmData;

	void MapDrawer::bresenhamDrawLine_x(Gdiplus::Color color, int x1, int y1, int x2, int y2, int line = ENDLESSLINE, int blank = 0);
	void MapDrawer::bresenhamDrawLine_y(Gdiplus::Color color, int x1, int y1, int x2, int y2, int line = ENDLESSLINE, int blank = 0);
	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
	wchar_t* CharToWchar(const char* c);
};
/*
class Character
{
public:
static void drawChar(char c, MapDrawer& md); //use switch...case...
private:
static const bool num[10][3][5]; //0~9数字
static const bool point[3][5]; //小数点
static const bool minus[3][5]; //负号
static const bool space[3][5]; //空格
};
*/
