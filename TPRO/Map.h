/* 
 * Last Updated at [2017/1/18 17:50] by wuhao
 * version 1.5.0
 * comments:����exportSubMap���� @2017/1/18
 *			��shortestPathLength_for_MM_v2/v3������һЩ����, �޸�getCompleteRoute,����һЩbug @2017/1/3
 *			��openOld��΢������ @2016/12/13
 *			����SP for mapmatching���ܺ��� @2016/12/06
 *			��GeoPoint�ײ�ı���ж�Ӧ���޸ģ��Կ��ܴ���bug @2016/6/29
 *			����hasEdgeWithMinLen @2016/6/25
 *			����drawEdge, drawRoute���� @2016/6/16
 *			�����ɼ�¼·�������·���ܣ��Ѿ�ԭ�����·���Ӳ���ͨ����� @2016/3/1
 *			����distM�����ͶӰ�����깦�� @2016/2/19
 *			�޸�getNearEdges�д����쳣��bug & �޸�drawMap����,���ӿ�ѡ���м�ڵ㹦��@2015/3/19
 *			�޸�getNearEdges������bug2 @2015/2/27
 *			�޸�getNearEdges������bug @2015/2/20
 *			���� loadPolylines for SMMD @2015/2/17
 *			���Ӷ���ʱ��Ϊnode���������������� @2015/2/13
 *			����SMMD��Ҫ�õĹ��ܺ��� & ΪSMMD����Edge�ֶ� @2015/2/13
 *			���distM�������ڴ�й©���� @2015/2/6
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
#define INF  1e20 //���·������
#define MAXSPEED 50 //����ٶ�
using namespace std;
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#define max(a,b)	(((a) > (b)) ? (a) : (b))
typedef list<GeoPoint*> Figure; //����һ��·�Σ�ÿ��GeoPoint*����·�ε㣬��β�ڵ㼴·�������˵�
typedef list<GeoPoint*> Traj;
typedef vector<int> Route;
//typedef vector<GeoPoint*> Polyline;
typedef std::pair<double, double> simplePoint; //�ڲ����ͣ���Ķ�
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
	Theta(); //Ĭ�Ϲ��캯�����Ҫд,�Է�releaseģʽ��,û�г�ʼ������ֱ�ӵ��ö����´���,debugģʽ�Զ�����ʼ�����Բ������
};

struct  Edge 
{
	Figure* figure;  //·����Ϣ
	double lengthM;  //��¼·���ܳ�����λΪm
	int startNodeId; //·����ʼ��id
	int endNodeId;  //·����ֹ��id
	bool visited;  //�����ֶΣ��ⲿ����������Ķ��������֪�������ʲô���õĻ��벻Ҫ�Ķ�������ֵ��,Ϊ���ڷ�Χ��ѯʱ���ֹ�ظ����ض��������ֶ�
	int id;
	bool visitedExtern; //���ⲿ���õ��ֶΣ�����¼���edge�Ƿ񱻷���
	int type; //1~8
	double avgSpd_mps; //��·��ƽ���ٶȣ�����ʷ����ͳ�Ƽ�¼�ڴ��ֶ�
	vector<int> adjEdgeIds;

	//Extend for SMMD
	double alpha;
	double beta;
	double u;
	double lambda;
	double prior;
	vector<GeoPoint*> r_hat;
	vector<GeoPoint*> r_fig; //figure��vector��ʽ
	vector<Theta> thetas;
	list<GeoPoint*> trainData;
	double intervalM;

	int getSlotId(GeoPoint* pt);
	void cut(double intervalM);
	Edge();
};

struct AdjNode //�ڽӱ���
{
	int endPointId;
	int edgeId;
	AdjNode* next;
};

//���·�������������ݽṹ
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
	vector<Edge*> edges; //�������бߵļ��ϣ�����ߵ������˵�����һ�����ڷ�Χ����ΪNULL��������������ֶ��ж�NULL��
	vector<GeoPoint*> nodes; //�������е�ļ���,����㲻�ڷ�Χ����ΪNULL��������������ֶ��ж�NULL��
	vector<AdjNode*> adjList; //�ڽӱ�

	void roadtypeSummary(vector<Traj*>& trajs_vec);
	void roadtypeSummary(vector<Route>& routes_vec);
	double turningAngle(int e1, int e2);
	
	void setArea(Area* area);
	Map(); //Ĭ�Ϲ��캯��,��Ҫ�ֶ�����open()��������ʼ��
	Map(string folderDir, Area* area, int gridWidth = 0);  //[Depreciated] ��folderDir·���������ͼ�ļ�,����gridWidth�е���������������,gridWidth<=0ʱΪ����������
	Map(string folderDir, Area* area, double gridSizeM = 0.0); //��folderDir·���������ͼ�ļ�,��������߳�ΪgridSizeM������������,gridSizeM<=0ʱΪ����������
	void open(string folderDir, int gridWidth = 0);  //[Depreciated] ��folderDir·���������ͼ�ļ�,����gridWidth�е���������������,�������޲ι��캯��	
	void open(string folderDir, double gridSizeM = 0.0, bool loadWaytype = true);  //��folderDir·���������ͼ�ļ�,��������߳�ΪgridSizeM������������,�������޲ι��캯��	,gridSizeM<=0ʱΪ����������
	void openOld(string folderDir, int gridWidth = 0); //��folderDir·�������롸�ϰ��ʽ����ͼ�ļ�,����gridWidth�е���������������,�������޲ι��캯��	
	void ExtractSubMap(string exportPath = "", vector<int>& nodeid_old2new = vector<int>(), vector<int>& edgeid_old2new = vector<int>());
	static void estimateSpd(vector<Traj*>& trajs_vec);
	void get_avg_spd_of_roads(vector<Traj*>& trajs_vec, double defaultSpd_mps = 5.0);
	void check_edge_visited(vector<Route>& routes_vec, bool dumpfile = false, string filePath = "unvisited_edges.csv");
	void dump_unvisitied_edges(string filePath);

	/*�������ܺ���*/
	int hasEdge(int startNodeId, int endNodeId) const; //�ж�startNodeId��endNodeId֮�����ޱ�,û�б߷���-1���б߷��ص�һ������������edgeId[ע�⣺��β��ȷ��������£�·���п��ܴ��ڶ�����]
	int hasEdgeWithMinLen(int startNodeId, int endNodeId) const; //�ж�startNodeId��endNodeId֮�����ޱ�,û�б߷���-1���б߷����������������б�����̵�edgeId
	int insertNode(double lat, double lon); //����һ���½��,�����½��id
	int insertEdge(Figure* figure, int startNodeId, int endNodeId); //�ڵ�ǰͼ�в����,�����±�id
	int splitEdge(int edgeId, double lat, double lon); //��edge��(lat,lon)�㴦�ֿ�������,(lat,lon)��Ϊ�½�����,�����½���nodeId
	void getMinMaxLatLon(string nodeFilePath);
	void drawMap(Gdiplus::Color color, MapDrawer& md, bool drawNodeId = true, bool drawInternalPts = false); //������ͼ, drawNodeIdΪtrue�򻭳��ڵ��ţ�drawInternalPtsΪtrue��ͬʱ����·�νڵ�
	void drawEdge(Gdiplus::Color color, MapDrawer& md, int edgeId, bool bold = true, bool verbose = true); //��idΪedgeId��·�Σ�����edgeΪNULL
	void drawRoute(Gdiplus::Color color, MapDrawer& md, vector<int> route, bool bold = true); //route��¼��Ҫ����·����������������
	double distM(double lat, double lon, Edge* edge) const; //����(lat,lon)�㵽edge�ľ��룬��λΪ��
	void getNearNodes(double lat, double lon, int k, vector<GeoPoint*>& dest); //���ؾ���(lat, lon)�����k��node������dest
	void goAlongPath_and_split(GeoPoint startPos, GeoPoint endPos, vector<int>& path, vector<double>& ratios, vector<GeoPoint>& splitPts); //��startPos��endPos������path��·���У�����ratios����ȷ���·���еĵ㣬����ÿ��֮��ı�ֵ����ratios,���忴ʵ���е�˵��
	GeoPoint goAlongEdge(Edge* edge, double offsetM, double _distToGoM); //�Ӿ���edgeͷoffset�׵�λ������·��_distToGoM���룬����Ŀ�ĵ�,offsetN,_distToGoM��Ҫ�Ϸ�����������·
	//GeoPoint goAlongEdge(Edge* edge, GeoPoint* startPos, double _distM); //��startPos����·��_distM���룬����Ŀ�ĵأ�startPos��Ҫ��·�ϣ�_distMҪ�Ϸ�

	/*Map Matching��غ���*/
	vector<Edge*> getNearEdges(double lat, double lon, double threshold) const; //���ؾ���(lat, lon)���ϸ�С��threshold�׵�����Edge*,������ڴ�й¶
	void getNearEdges(double lat, double lon, double threshold, vector<Edge*>& dest); //ע�⣬�̲߳���ȫ
	void getNearEdges_s(double lat, double lon, double threshold, vector<Edge*>& dest); //�̰߳�ȫ�汾
	void getNearEdges(double lat, double lon, int k, vector<Edge*>& dest); //������(lat, lon)����������k��·�Σ�����dest
	double shortestPathLength(int ID1, int ID2, double dist1, double dist2, double deltaT);//�����·������������ɶ��˼��Ҳ��֪���������ҵĴ���
	double shortestPathLength(int nodeId1, int nodeId2, vector<int>& path, bool unreachable_warning = false); //�����·����·���������У���¼��path���������·�ĳ��ȣ�����ͨ������·���INF,������unreachable_warning�������Ƿ�warning
	double shortestPathLength_for_MM(int nodeId1, int nodeId2, vector<int>& path, double deltaT, double maxSpd_MPS, bool unreachable_warning  = false);
	double shortestPathLength_for_MM_v2(int nodeId1, int nodeId2, vector<int>& path, double deltaT, double maxSpd_MPS, map<int, double>& cache, bool use_prune_strategy = true, bool unreachable_warning = false);
	double shortestPathLength_for_MM_v3(int nodeId1, int nodeId2, vector<int>& path, double deltaT, double maxSpd_MPS, map<int, double>& cache, double* dist, bool* flag, int* prev, bool use_intermediate_cache = true, bool use_prune_strategy = true, bool unreachable_warning = false);
	double distM(double lat, double lon, Edge* edge, double& prjDist) const;//ͬ�ϣ�ͬʱ��¼ͶӰ�㵽edge���ľ������prjDist����ͶӰ���Ϊ0
	double distM(double lat, double lon, Edge* edge, GeoPoint& projection) const; //ͬ�ϣ�ͬʱ��¼ͶӰ������
	double distMFromTransplantFromSRC(double lat, double lon, Edge* edge, double& prjDist); //��ֲSRC�汾������(lat,lon)�㵽edge�ľ��룬��λΪ�ף�ͬʱ��¼ͶӰ�㵽edge���ľ������prjDist
	bool getCompleteRoute(vector<int>& subRoute, vector<int>& route_ans); //����subRoute����������·��������subRoute�������߿��ظ�����1,1,2,2,2,3,5,6�����ؽ��Ϊ��׼��route, ͨ������route_ans���أ����·��������������ֵ����false

	/*ɾ·���*/
	void delEdge(int edgeId, bool delBirectionEdges = true); //�ӵ�ͼ��ɾ��edgeId������ڶ�������Ϊtrue��ͬʱɾ������·
	void drawDeletedEdges(Gdiplus::Color color, MapDrawer& md); //����ɾ����·
	void deleteEdgesRandomly(int delNum, double minEdgeLengthM); //�ڵ�ͼ�����ɾ�����Ȳ�����minEdgeLengthM��delNum��Edge(ͬʱ��ɾ������·)�����������������main�г�ʼ��
	//��ǿ�汾�����ɾ�����Ȳ�����minEdgeLengthM��delNum��Edge��
	//ͬʱɾ����edge�����㵽�����С��aroundThresholdM��·�β�����aroundNumThreshold����˫��·��2�ƣ���
	//��������Χ����Щ·ȫ��ɾ�������������������main�г�ʼ��
	//doOutputΪtrueʱ���������ɾ����·�κ�������ļ���һ��һ��id��
	void deleteEdgesRandomlyEx(int delNum, double minEdgeLengthM, double aroundThresholdM, int aroundNumThreshold, bool doOutput = true);
	void deleteEdges(string path); //����deleteEdgesRandomlyEx���������ɾ����id�ŵ��ļ��������·����ɾ������֤���ô˺������״̬�����deleteEdgesRandomlyEx()���״̬һ��
	vector<Edge*> deletedEdges; //��¼��ɾ�����ı�
	void deleteIntersectionType1(int delNum, double minEdgeLengthM, bool doOutput = true);
	void deleteIntersectionType2(int delNum, double minEdgeLengthM, bool doOutput = true);
	void deleteIntersectionType3(int delNum, double minEdgeLengthM, bool doOutput = true);

	/*SMMD���*/
	void loadPolylines(string filePath); //�������ɵ�polyline������r_hat����

	PointGridIndex ptIndex;

//private:
	int gridWidth, gridHeight;
	double gridSizeDeg;
	double strictThreshold = 0;
	list<Edge*>* **grid;
	list<GeoPoint*> nodesInArea; //�����������ڵ�nodes������Ϊnodes��������������
	
	Area* area;
	//һЩ���õ�area�ο�ֵ
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
	double distM_withThres(double lat, double lon, Edge* edge, double threshold) const; //����(lat,lon)�㵽edge�ľ����Ͻ�,��ǰԤ���Ż��汾	
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
	void findNearEdges(Edge* edge, list<Edge*>& dest, double thresholdM); //���ؾ���edge���Ȳ�����thresholdM��·��
	double distM(Edge* edge1, Edge* edge2, double threshold = 0);
	void getNearEdges(Edge* edge, double thresholdM, vector<Edge*>& dest);
};

