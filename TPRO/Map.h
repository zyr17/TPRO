/* 
 * Last Updated at [2017/1/18 17:50] by wuhao
 * version 1.5.0
 * comments:增加exportSubMap功能 @2017/1/18
 *			对shortestPathLength_for_MM_v2/v3增加了一些开关, 修改getCompleteRoute,改了一些bug @2017/1/3
 *			对openOld稍微改了下 @2016/12/13
 *			增加SP for mapmatching功能函数 @2016/12/06
 *			对GeoPoint底层改变进行对应的修改，仍可能存在bug @2016/6/29
 *			增加hasEdgeWithMinLen @2016/6/25
 *			增加drawEdge, drawRoute功能 @2016/6/16
 *			新增可记录路径的最短路功能，已经原有最短路增加不连通的情况 @2016/3/1
 *			新增distM可求得投影点坐标功能 @2016/2/19
 *			修复getNearEdges中触发异常的bug & 修改drawMap函数,增加可选画中间节点功能@2015/3/19
 *			修复getNearEdges函数的bug2 @2015/2/27
 *			修复getNearEdges函数的bug @2015/2/20
 *			增加 loadPolylines for SMMD @2015/2/17
 *			增加读入时候为node建立网格索引功能 @2015/2/13
 *			增加SMMD需要用的功能函数 & 为SMMD增加Edge字段 @2015/2/13
 *			解决distM函数的内存泄漏问题 @2015/2/6
 */

#pragma once
#include "GeoPoint.h"
#include <list>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <queue>
#include "MapDrawer.h"
#include "PointGridIndex.h"
#include <mutex>
#include <thread>
#include <map>
#include <stack>
#include <iomanip>

#define eps 1e-6
#define INF  1e20 //最短路径所用
#define MAXSPEED 50 //最大速度
using namespace std;
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#define max(a,b)	(((a) > (b)) ? (a) : (b))
typedef list<GeoPoint*> Figure; //代表一条路形，每个GeoPoint*代表路形点，首尾节点即路的两个端点
typedef list<GeoPoint*> Traj;
typedef vector<int> Route;
//typedef vector<GeoPoint*> Polyline;
typedef std::pair<double, double> simplePoint; //内部类型，勿改动
using namespace std;


/**********************************************************/
/*test code starts from here*/
extern double mapQueryTime_for_cache;
extern double initialTimeInSP;
/*test code ends*/
/**********************************************************/


struct Theta
{
	double alpha;
	double beta;
	double u;
	double lambda;

	double mu;
	double sigma;
	Theta(); //默认构造函数务必要写,以防release模式下,没有初始化对象直接调用对象导致错误,debug模式自动带初始化所以不会出错
};

struct  Edge 
{
	Figure* figure;  //路形信息
	double lengthM;  //记录路段总长，单位为m
	int startNodeId; //路段起始点id
	int endNodeId;  //路段终止点id
	bool visited;  //辅助字段，外部调用需谨慎改动（如果不知道这个有什么作用的话请不要改动这个域的值）,为了在范围查询时候防止重复返回而设立的字段
	int id;
	bool visitedExtern; //给外部调用的字段，来记录这个edge是否被访问
	int type; //1~8
	double avgSpd_mps; //道路的平均速度，由历史数据统计记录在此字段
	vector<int> adjEdgeIds;

	//Extend for SMMD
	double alpha;
	double beta;
	double u;
	double lambda;
	double prior;
	vector<GeoPoint*> r_hat;
	vector<GeoPoint*> r_fig; //figure的vector形式
	vector<Theta> thetas;
	list<GeoPoint*> trainData;
	double intervalM;

	int getSlotId(GeoPoint* pt);
	void cut(double intervalM);
	Edge();
};

struct AdjNode //邻接表结点
{
	int endPointId;
	int edgeId;
	AdjNode* next;
};

//最短路径长度所用数据结构
struct NODE_DIJKSTRA 
{
	int t; double dist;
	NODE_DIJKSTRA(int i, double dist)
	{
		this->t = i;
		this->dist = dist;
	}
	bool operator < (const NODE_DIJKSTRA &rhs) const 
	{
		return dist > rhs.dist;
	}
};


class Map
{
public:
	friend struct Edge;
	vector<Edge*> edges; //保存所有边的集合，如果边的两个端点至少一个不在范围内则为NULL，【逐个遍历需手动判断NULL】
	vector<GeoPoint*> nodes; //保存所有点的集合,如果点不在范围内则为NULL，【逐个遍历需手动判断NULL】
	vector<AdjNode*> adjList; //邻接表

	void roadtypeSummary(vector<Traj*>& trajs_vec);
	void roadtypeSummary(vector<Route>& routes_vec);
	double turningAngle(int e1, int e2);
	
	void setArea(Area* area);
	Map(); //默认构造函数,需要手动调用open()函数来初始化
	Map(string folderDir, Area* area, int gridWidth = 0);  //[Depreciated] 在folderDir路径下载入地图文件,并以gridWidth列的粒度来创建索引,gridWidth<=0时为不建立索引
	Map(string folderDir, Area* area, double gridSizeM = 0.0); //在folderDir路径下载入地图文件,并以网格边长为gridSizeM米来创建索引,gridSizeM<=0时为不建立索引
	void open(string folderDir, int gridWidth = 0);  //[Depreciated] 在folderDir路径下载入地图文件,并以gridWidth列的粒度来创建索引,适用于无参构造函数	
	void open(string folderDir, double gridSizeM = 0.0, bool loadWaytype = true);  //在folderDir路径下载入地图文件,并以网格边长为gridSizeM米来创建索引,适用于无参构造函数	,gridSizeM<=0时为不建立索引
	void openOld(string folderDir, int gridWidth = 0); //在folderDir路径下载入「老版格式」地图文件,并以gridWidth列的粒度来创建索引,适用于无参构造函数	
	void ExtractSubMap(string exportPath = "", vector<int>& nodeid_old2new = vector<int>(), vector<int>& edgeid_old2new = vector<int>());
	static void estimateSpd(vector<Traj*>& trajs_vec);
	void get_avg_spd_of_roads(vector<Traj*>& trajs_vec, double defaultSpd_mps = 5.0);
	void check_edge_visited(vector<Route>& routes_vec, bool dumpfile = false, string filePath = "unvisited_edges.csv");
	void dump_unvisitied_edges(string filePath);

	/*基础功能函数*/
	int hasEdge(int startNodeId, int endNodeId) const; //判断startNodeId与endNodeId之间有无边,没有边返回-1，有边返回第一条满足条件的edgeId[注意：首尾点确定的情况下，路网中可能存在多条边]
	int hasEdgeWithMinLen(int startNodeId, int endNodeId) const; //判断startNodeId与endNodeId之间有无边,没有边返回-1，有边返回满足条件的所有边中最短的edgeId
	int insertNode(double lat, double lon); //插入一个新结点,返回新结点id
	int insertEdge(Figure* figure, int startNodeId, int endNodeId); //在当前图中插入边,返回新边id
	int splitEdge(int edgeId, double lat, double lon); //将edge在(lat,lon)点处分开成两段,(lat,lon)作为新结点加入,返回新结点的nodeId
	void getMinMaxLatLon(string nodeFilePath);
	void drawMap(Gdiplus::Color color, MapDrawer& md, bool drawNodeId = true, bool drawInternalPts = false); //画出地图, drawNodeId为true则画出节点编号，drawInternalPts为true则同时画出路形节点
	void drawEdge(Gdiplus::Color color, MapDrawer& md, int edgeId, bool bold = true, bool verbose = true); //画id为edgeId的路段，允许edge为NULL
	void drawRoute(Gdiplus::Color color, MapDrawer& md, vector<int> route, bool bold = true); //route记录需要画的路径，不会检查连接性
	double distM(double lat, double lon, Edge* edge) const; //返回(lat,lon)点到edge的距离，单位为米
	void getNearNodes(double lat, double lon, int k, vector<GeoPoint*>& dest); //返回距离(lat, lon)最近的k个node，存入dest
	void goAlongPath_and_split(GeoPoint startPos, GeoPoint endPos, vector<int>& path, vector<double>& ratios, vector<GeoPoint>& splitPts); //从startPos到endPos，经过path的路程中，根据ratios的配比返回路程中的点，满足每段之间的比值满足ratios,具体看实现中的说明
	GeoPoint goAlongEdge(Edge* edge, double offsetM, double _distToGoM); //从距离edge头offset米的位置沿着路走_distToGoM距离，返回目的地,offsetN,_distToGoM都要合法，不允许超出路
	//GeoPoint goAlongEdge(Edge* edge, GeoPoint* startPos, double _distM); //从startPos沿着路走_distM距离，返回目的地，startPos需要在路上，_distM要合法

	/*Map Matching相关函数*/
	vector<Edge*> getNearEdges(double lat, double lon, double threshold) const; //返回距离(lat, lon)点严格小于threshold米的所有Edge*,会产生内存泄露
	void getNearEdges(double lat, double lon, double threshold, vector<Edge*>& dest); //注意，线程不安全
	void getNearEdges_s(double lat, double lon, double threshold, vector<Edge*>& dest); //线程安全版本
	void getNearEdges(double lat, double lon, int k, vector<Edge*>& dest); //返回离(lat, lon)点距离最近的k条路段，存入dest
	double shortestPathLength(int ID1, int ID2, double dist1, double dist2, double deltaT);//求最短路，后两个参数啥意思我也不知道，不是我的代码
	double shortestPathLength(int nodeId1, int nodeId2, vector<int>& path, bool unreachable_warning = false); //求最短路，将路径（边序列）记录到path，返回最短路的长度，不连通的情况下返回INF,并根据unreachable_warning来决定是否报warning
	double shortestPathLength_for_MM(int nodeId1, int nodeId2, vector<int>& path, double deltaT, double maxSpd_MPS, bool unreachable_warning  = false);
	double shortestPathLength_for_MM_v2(int nodeId1, int nodeId2, vector<int>& path, double deltaT, double maxSpd_MPS, map<int, double>& cache, bool use_prune_strategy = true, bool unreachable_warning = false);
	double shortestPathLength_for_MM_v3(int nodeId1, int nodeId2, vector<int>& path, double deltaT, double maxSpd_MPS, map<int, double>& cache, double* dist, bool* flag, int* prev, bool use_intermediate_cache = true, bool use_prune_strategy = true, bool unreachable_warning = false);
	double distM(double lat, double lon, Edge* edge, double& prjDist) const;//同上，同时记录投影点到edge起点的距离存入prjDist，无投影则记为0
	double distM(double lat, double lon, Edge* edge, GeoPoint& projection) const; //同上，同时记录投影点坐标
	double distMFromTransplantFromSRC(double lat, double lon, Edge* edge, double& prjDist); //移植SRC版本：返回(lat,lon)点到edge的距离，单位为米；同时记录投影点到edge起点的距离存入prjDist
	bool getCompleteRoute(vector<int>& subRoute, vector<int>& route_ans); //根据subRoute返回完整的路径，其中subRoute的连续边可重复，如1,1,2,2,2,3,5,6。返回结果为标准的route, 通过参数route_ans返回，如果路径不连续，函数值返回false

	/*删路相关*/
	void delEdge(int edgeId, bool delBirectionEdges = true); //从地图中删除edgeId，如果第二个参数为true则同时删除反向路
	void drawDeletedEdges(Gdiplus::Color color, MapDrawer& md); //画出删除的路
	void deleteEdgesRandomly(int delNum, double minEdgeLengthM); //在地图里随机删除长度不短于minEdgeLengthM的delNum条Edge(同时会删除反向路)，随机种子需自行在main中初始化
	//增强版本，随机删除长度不短于minEdgeLengthM的delNum条Edge，
	//同时删除的edge需满足到其距离小于aroundThresholdM的路段不超过aroundNumThreshold条（双向路以2计），
	//并将其周围的这些路全部删除，随机种子需自行在main中初始化
	//doOutput为true时将会把所有删除的路段号输出到文件（一行一个id）
	void deleteEdgesRandomlyEx(int delNum, double minEdgeLengthM, double aroundThresholdM, int aroundNumThreshold, bool doOutput = true);
	void deleteEdges(string path); //对于deleteEdgesRandomlyEx输出的所有删除的id号的文件读入后在路网中删除，保证调用此函数后的状态与调用deleteEdgesRandomlyEx()后的状态一样
	vector<Edge*> deletedEdges; //记录被删除掉的边
	void deleteIntersectionType1(int delNum, double minEdgeLengthM, bool doOutput = true);
	void deleteIntersectionType2(int delNum, double minEdgeLengthM, bool doOutput = true);
	void deleteIntersectionType3(int delNum, double minEdgeLengthM, bool doOutput = true);

	/*SMMD相关*/
	void loadPolylines(string filePath); //读入生成的polyline，加入r_hat域中

	PointGridIndex ptIndex;

//private:
	int gridWidth, gridHeight;
	double gridSizeDeg;
	double strictThreshold = 0;
	list<Edge*>* **grid;
	list<GeoPoint*> nodesInArea; //保存在区域内的nodes，用来为nodes建立网格索引用
	
	Area* area;
	//一些常用的area参考值
	//singapore half
	/*double minLat = 1.22;
	double maxLat = 1.5;
	double minLon = 103.620;
	double maxLon = 104.0;*/
	
	/*singapore full
	double minLat = 0.99999;
	double maxLat = 1.6265;
	double minLon = 103.548;
	double maxLon = 104.1155;*/

	//washington full
	//minLat:45.559102, maxLat : 49.108823, minLon : -124.722781, maxLon : -116.846465
	/*double minLat = 45.0;
	double maxLat = 49.5;
	double minLon = -125.0;
	double maxLon = -116.5;*/

	int getRowId(double lat) const;
	int getColId(double lon) const;
	double distM_withThres(double lat, double lon, Edge* edge, double threshold) const; //返回(lat,lon)点到edge的距离上界,提前预判优化版本	
	double calEdgeLength(Figure* figure) const;
	bool inArea(double lat, double lon, bool use_geo) const;
	void createGridIndex();
	void createGridIndexForEdge(Edge *edge);
	void createGridIndexForSegment(Edge *edge, GeoPoint* fromPT, GeoPoint* toPt);
	void insertEdgeIntoGrid(Edge* edge,int row, int col);
	void insertEdge(int edgeId, int startNodeId, int endNodeId);
	
	void split(const string& src, const string& separator, vector<string>& dest);
	void split(const string& src, const char& separator, vector<string>& dest);
	static double cosAngle(GeoPoint* pt1, GeoPoint* pt2, GeoPoint* pt3);
	static double turningAngle(GeoPoint* p1, GeoPoint* p2, GeoPoint* p3);
	void test();
	void findNearEdges(Edge* edge, list<Edge*>& dest, double thresholdM); //返回距离edge长度不超过thresholdM的路段
	double distM(Edge* edge1, Edge* edge2, double threshold = 0);
	void getNearEdges(Edge* edge, double thresholdM, vector<Edge*>& dest);
};

