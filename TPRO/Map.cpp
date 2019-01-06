/* 
 * Last Updated at [2017/1/18 17:50] by wuhao
 */
#include "Map.h"

vector<string> WAY_TYPE = { "service", "residential", "unclassified", "tertiary", "secondary", "primary", "trunk", "motorway" };
bool smallerInX(simplePoint& pt1, simplePoint& pt2);
bool smallerInDist(pair<Edge*, double>& c1, pair<Edge*, double>& c2);

//////////////////////////////////////////////////////////////////////////
///for class Edge
//////////////////////////////////////////////////////////////////////////
Theta::Theta()
{
	this->mu = 0;
	this->sigma = 10;
}

Edge::Edge()
{
	r_hat = vector<GeoPoint*>();
	thetas = vector<Theta>();
	trainData = list<GeoPoint*>();
	visitedExtern = false;
}

void Edge::cut(double intervalM)
{
	//////////////////////////////////////////////////////////////////////////
	///为Edge初始化，按照intervalM长度一段段切开，每一段对应一个参数theta，每段最后一段不够长的独立算一段
	//////////////////////////////////////////////////////////////////////////
	this->intervalM = intervalM;
	int count = 0;
	for (int i = 0; i < r_hat.size() - 1; i++)
	{
		count += (int)(GeoPoint::distM(r_hat[i + 1], r_hat[i]) / intervalM) + 1;
	}
	for (int i = 0; i < count; i++)
	{
		thetas.push_back(Theta());
	}
	if (thetas.size() == 0)
	{
		cout << "theta = 0" << endl;
		system("pause");
	}
	
}

int Edge::getSlotId(GeoPoint* pt)
{
	//////////////////////////////////////////////////////////////////////////
	///找出pt对应的theta位置
	//////////////////////////////////////////////////////////////////////////
	//找投影位置	
	double minDist = INF;
	int a = 0;
	//遍历端点距离
	for (int i = 0; i < r_hat.size(); i++)
	{
		double tmpDist = GeoPoint::distM(pt, r_hat[i]);
		if (tmpDist < minDist)
		{
			minDist = tmpDist;
			if (i != 0)
				a = i - 1; //condition (2)
			else
				a = 0; //condition (3)
		}
	}
	//遍历投影距离
	for (int i = 0; i < r_hat.size() - 1; i++)
	{

		/**********************************************************/
		/*test code starts from here*/
		if (i + 1 >= r_hat.size())
		{
			printf("i+1 = %d, size = %d\n", i + 1, r_hat.size());
			system("pause");
		}
		/*test code ends*/
		/**********************************************************/

		if (Map::cosAngle(pt, r_hat[i], r_hat[i + 1]) <= 0 && Map::cosAngle(pt, r_hat[i + 1], r_hat[i]) <= 0) //确认x投影落在r_hat[i~i+1]上
		{
			double A = (r_hat[i + 1]->lat - r_hat[i]->lat);
			double B = -(r_hat[i + 1]->lon - r_hat[i]->lon);
			double C = r_hat[i]->lat * (r_hat[i + 1]->lon - r_hat[i]->lon)
				- r_hat[i]->lon * (r_hat[i + 1]->lat - r_hat[i]->lat);

			double tmpDist = abs(A * pt->lon + B * pt->lat + C) / sqrt(A * A + B * B);
			tmpDist *= GeoPoint::geoScale;
			if (minDist > tmpDist)
			{
				minDist = tmpDist;
				a = i; //condition (1)
			}
		}
	}
	
	//先从头开始走，把前面的先算好
	int count = 0;
	for (int i = 0; i < a; i++)
		count += (int)(GeoPoint::distM(r_hat[i + 1], r_hat[i]) / intervalM) + 1;
	//计算到头端的距离
	double cos_angle = Map::cosAngle(pt, r_hat[a], r_hat[a + 1]);
	double dist = cos_angle * GeoPoint::distM(pt, r_hat[a]);  //距离头(r_hat[a])的距离, 可能为负，当pt在r_hat[0]左边时
	if (dist < 0)
		dist = 0;
	//如果算出来的dist因为精度关系比dist(r_hat[i], r_hat[i+1])略多，则就将错就错把它算到下一个去，小概率事件，不影响大局
	//注意到如果是i+1正好是最后一个结点，同时dist还是超过的话，这时将会thetas越界，需要特判
	count += (int)(dist / intervalM) + 1;
	if (count > thetas.size())
		count = thetas.size();
	return count - 1;
}


//////////////////////////////////////////////////////////////////////////
///public part
//////////////////////////////////////////////////////////////////////////
Map::Map()
{
	area = new Area();
}

Map::Map(string folderDir, Area* area, int gridWidth /* = 0 */)
{
	cout << "[Warning]@Map::Map(string, Area*, int): This function is depreciated, please use Map::Map(string, Area*, double) instead." << endl;
	this->setArea(area);
	this->open(folderDir, gridWidth);
}

Map::Map(string folderDir, Area* area, double gridSizeM /* = 0.0 */)
{
	this->setArea(area);
	this->open(folderDir, gridSizeM);
}

void Map::ExtractSubMap(string exportPath /* = "" */, vector<int>& nodeid_old2new /* = vector<int> */, vector<int>& edgeid_old2new /* = vector<int>*/)
{
	//assign new id to nodes
	vector<int> nodeid_new2old, edgeid_new2old;
	//vector<int> nodeid_old2new, edgeid_old2new;
	nodeid_old2new.clear();
	edgeid_old2new.clear();
	for (int i = 0; i < nodes.size(); i++)
	{
		if (nodes[i] == NULL)
			nodeid_old2new.push_back(-1);
		else
		{
			nodeid_new2old.push_back(i);
			nodeid_old2new.push_back(nodeid_new2old.size() - 1);
		}
	}
	for (int i = 0; i < edges.size(); i++)
	{
		if (edges[i] == NULL)
			edgeid_old2new.push_back(-1);
		else
		{
			edgeid_new2old.push_back(i);
			edgeid_old2new.push_back(edgeid_new2old.size() - 1);
		}
		
	}
	if (exportPath == "")
		return;
	ofstream nodeOFS(exportPath + "nodeOSM.txt");
	ofstream edgeOFS(exportPath + "edgeOSM.txt");
	ofstream wayTypeOFS(exportPath + "wayTypeOSM.txt");
	if (!nodeOFS)
	{
		cout << "output " << exportPath + "nodeOSM.txt" << " error!" << endl;
		system("pause");
	}
	nodeOFS << fixed << showpoint << setprecision(8);
	edgeOFS << fixed << showpoint << setprecision(8);
	
	//export node
	for (int newId = 0; newId < nodeid_new2old.size(); newId++)
	{
		nodeOFS << newId << '\t' << nodes[nodeid_new2old[newId]]->lat_geo << "\t" << nodes[nodeid_new2old[newId]]->lon_geo << endl;
	}
	//export edge
	for (int newId = 0; newId < edgeid_new2old.size(); newId++)
	{
		int oldId = edgeid_new2old[newId];
		edgeOFS << newId << "\t" << nodeid_old2new[edges[oldId]->startNodeId] << "\t" << nodeid_old2new[edges[oldId]->endNodeId] << "\t" << edges[oldId]->figure->size();
		for (auto& pt : *edges[oldId]->figure)
		{
			edgeOFS << "\t" << pt->lat_geo << "\t" << pt->lon_geo;
		}
		edgeOFS << endl;
		wayTypeOFS << newId << "\t" << WAY_TYPE[edges[oldId]->type] << "\t" << edges[oldId]->type << endl;
	}
	nodeOFS.close();
	edgeOFS.close();
	wayTypeOFS.close();
	printf("输出子地图至%s, 共%d个的点，%d条边\n", exportPath.c_str(), nodeid_new2old.size(), edgeid_new2old.size());
}

void Map::openOld(string folderDir, int gridWidth /* = 0*/)
{
	/*文件目录结构为
	* folderDir
	* |-WA_Nodes.txt
	* |-WA_EdgeGeometry.txt
	* |-WA_Edges.txt
	*/
	//////////////////////////////////////////////////////////////////////////
	///排除规则：当edge的两个端点都在area外则不加入（node和edge对应位置放NULL）
	/////[注意]同样两个端点可能存在两条不同的edge!!!!
	//////////////////////////////////////////////////////////////////////////

	this->gridWidth = gridWidth;
	int count = 0;

	//////////////////////////////////////////////////////////////////////////
	//读取WA_Nodes.txt
	//格式：nodeId lat lon
	//////////////////////////////////////////////////////////////////////////
	ifstream nodeIfs(folderDir + "WA_Nodes.txt");
	if (!nodeIfs)
	{
		cout << "open " + folderDir + "WA_Nodes.txt" + " error!\n";
		system("pause");
		exit(0);
	}
	while (nodeIfs)
	{
		double lat, lon;
		int nodeId;
		GeoPoint* pt;
		nodeIfs >> nodeId >> lat >> lon;
		if (nodeIfs.fail())
			break;
		if (inArea(lat, lon, true))
		{
			pt = new GeoPoint(lat, lon, true);
			nodesInArea.push_back(pt);
		}
		else
		{
			//pt = NULL;
			pt = new GeoPoint(lat, lon, true);
			count++;
		}
		nodes.push_back(pt);
	}
	printf("nodes count = %d\n", nodes.size());
	printf("nodes not in area count = %d\n", count);
	nodeIfs.close();

	//////////////////////////////////////////////////////////////////////////
	//读取WA_EdgeGeometry.txt
	//格式：edgeId^^Highway^1^起始端点纬度^起始端点经度[^中间点1纬度^中间点1经度^中间点2纬度^中间点2经度.....]^结束端点纬度^结束端点经度    
	//////////////////////////////////////////////////////////////////////////
	count = 0;
	std::ifstream geometryIfs(folderDir + "WA_EdgeGeometry.txt");
	if (!geometryIfs)
	{
		cout << "open " + folderDir + "WA_EdgeGeometry.txt" + " error!\n";
		system("pause");
		exit(0);
	}
	std::string strLine;
	bool continueFlag = false;
	int firstShapeIndex = 4;
	while (getline(geometryIfs, strLine))
	{
		if (geometryIfs.fail())
			break;
		std::vector<std::string> substrs;
		/*singapore ver*/
		split(strLine, '^', substrs);
		/*for (int i = 0; i < substrs.size(); i++)
		{
			cout << substrs[i] << endl;
		}
		system("pause");*/
		int edgeId = atoi(substrs[0].c_str());
		double startLat = atof(substrs[firstShapeIndex].c_str());
		//printf("startlat = %lf\n", startLat);
		double startLon = atof(substrs[firstShapeIndex + 1].c_str());
		double endLat = atof(substrs[substrs.size() - 2].c_str());
		double endLon = atof(substrs[substrs.size() - 1].c_str());
		if (!inArea(startLat, startLon, true) && !inArea(endLat, endLon, true))
		{
			//	printf("start(%lf,%lf), end(%lf,%lf)\n", startLat, startLon, endLat, endLon);
			//	printf("minlat = %lf, maxlat = %lf\n", minLat, maxLat);
			//	system("pause");
			edges.push_back(NULL);
			count++;
			continue;
		}
		Figure* figure = new Figure();
		for (int i = firstShapeIndex; i < substrs.size() - 1; i += 2)
		{
			double lat, lon;
			lat = atof(substrs[i].c_str());
			lon = atof(substrs[i + 1].c_str());
			//	if (inArea(lat, lon))
			//	{
			GeoPoint* pt = new GeoPoint(lat, lon, true);
			figure->push_back(pt);
			//	}
			//else
			//{
			//	continueFlag = true;
			//	edges.push_back(NULL);
			//	count++;
			//	break;
			//}

		}
		///[TODO]:continueFlag是干啥的忘记了
		if (continueFlag)
		{
			delete figure;
			continueFlag = false;
			continue;
		}
		Edge* edge = new Edge();
		edge->id = edgeId;
		edge->visited = false;
		edge->figure = figure;
		edge->lengthM = calEdgeLength(figure);
		edges.push_back(edge);
	}
	printf("edges count = %d\n", edges.size());
	printf("not in area edges count = %d\n", count);
	geometryIfs.close();

	//////////////////////////////////////////////////////////////////////////
	//读取WA_Edges.txt
	//格式：edgeId startNodeId endNodeId 1
	//////////////////////////////////////////////////////////////////////////
	//初始化邻接表
	count = 0;
	int edgesCount = 0;
	for (int i = 0; i < nodes.size(); i++)
	{
		AdjNode* head = new AdjNode();
		head->endPointId = i;
		head->next = NULL;
		adjList.push_back(head);
	}
	std::ifstream edgeIfs(folderDir + "WA_Edges.txt");
	if (!edgeIfs)
	{
		cout << "open " + folderDir + "WA_Edges.txt" + " error!\n";
		system("pause");
		exit(0);
	}
	while (edgeIfs)
	{
		int edgeId, startNodeId, endNodeId, dummy;
		edgeIfs >> edgeId >> startNodeId >> endNodeId >> dummy;
		if (edgeIfs.fail())
			break;
		if (nodes[startNodeId] != NULL && nodes[endNodeId] != NULL && edges[edgeId] != NULL)
		{
			insertEdge(edgeId, startNodeId, endNodeId);
		}
		else
			count++;
	}
	edgeIfs.close();

	printf(">> reading map finished\n");
	createGridIndex();
	printf(">> creating grid index finished\n");
}

void Map::open(string folderDir, int gridWidth /* = 0 */)
{
	/*文件目录结构为,文件名要对应
	* folderDir
	* |-nodeOSM.txt
	* |-edgeOSM.txt
	*/
	//////////////////////////////////////////////////////////////////////////
	///排除规则：当edge的两个端点都在area外则不加入（node和edge对应位置放NULL）
	///node不管在不在area内全部保留
	//////////////////////////////////////////////////////////////////////////
	cout << "[Warning]@Map::open(string, int): This function is depreciated, please use Map::open(string , double) instead." << endl;

	this->gridWidth = gridWidth;
	int count = 0;
	//////////////////////////////////////////////////////////////////////////
	//读取nodeOSM.txt
	//格式：nodeId \t lat \t lon \n
	//////////////////////////////////////////////////////////////////////////
	ifstream nodeIfs(folderDir + "nodeOSM.txt");
	if (!nodeIfs)
	{
		cout << "open " + folderDir + "nodeOSM.txt" + " error!\n";
		system("pause");
		exit(0);
	}
	while (nodeIfs)
	{
		double lat, lon;
		int nodeId;
		GeoPoint* pt;
		nodeIfs >> nodeId >> lat >> lon;
		if (nodeIfs.fail())
			break;
		if (inArea(lat, lon, true))
		{
			pt = new GeoPoint(lat, lon, true);
			nodesInArea.push_back(pt);
		}
		else
		{
			//pt = NULL; //[ATTENTION]node在区域外不想保留用此句(edge构造时会产生bug，须自行处理)
			pt = new GeoPoint(lat, lon, true);
			count++;
		}
		nodes.push_back(pt);
	}
	printf("nodes count = %d\n", nodes.size());
	printf("nodes not in area count = %d\n", count);
	nodeIfs.close();

	//////////////////////////////////////////////////////////////////////////
	//读取edgeOSM.txt
	//格式：edgeOSM.txt: edgeId \t startNodeId \t endNodeId \t figureNodeCount \t figure1Lat \t figure1Lon \t ... figureNLat \t figureNLon \n  
	//////////////////////////////////////////////////////////////////////////
	count = 0;
	std::ifstream edgeIfs(folderDir + "edgeOSM.txt");
	if (!edgeIfs)
	{
		cout << "open " + folderDir + "WedgeOSM.txt" + " error!\n";
		system("pause");
		exit(0);
	}
	//初始化邻接表
	for (int i = 0; i < nodes.size(); i++)
	{
		AdjNode* head = new AdjNode();
		head->endPointId = i;
		head->next = NULL;
		adjList.push_back(head);
	}
	while (edgeIfs)
	{
		int edgeId, startNodeId, endNodeId, figureCount;
		edgeIfs >> edgeId >> startNodeId >> endNodeId >> figureCount;

		Figure* figure = new Figure();
		for (int i = 0; i < figureCount; i++)
		{
			double lat, lon;
			edgeIfs >> lat >> lon;
			figure->push_back(new GeoPoint(lat, lon, true));
		}
		if (edgeIfs.fail())
			break;
		if (!inArea(nodes[startNodeId]->lat, nodes[startNodeId]->lon, false) && !inArea(nodes[endNodeId]->lat, nodes[endNodeId]->lon, false))
		{
			edges.push_back(NULL);
			count++;
			continue;
		}
		Edge* edge = new Edge();
		edge->id = edgeId;
		edge->visited = false;
		edge->figure = figure;
		edge->lengthM = calEdgeLength(figure);
		edges.push_back(edge);
		insertEdge(edgeId, startNodeId, endNodeId);
	}

	printf("edges count = %d\n", edges.size());
	printf("not in area edges count = %d\n", count);

	edgeIfs.close();
	printf(">> reading map finished\n");
	createGridIndex();
	printf(">> creating grid index finished\n");
}

void Map::open(string folderDir, double gridSizeM /* = 0.0 */, bool loadWaytype /* = true*/)
{
	/*文件目录结构为,文件名要对应
	* folderDir
	* |-nodeOSM.txt
	* |-edgeOSM.txt
	*/
	//////////////////////////////////////////////////////////////////////////
	///排除规则：当edge的两个端点都在area外则不加入（node和edge对应位置放NULL）
	///node不管在不在area内全部保留
	//////////////////////////////////////////////////////////////////////////
	if (folderDir.back() != '\\')
		folderDir = folderDir + "\\";
	this->gridWidth = GeoPoint::distM(GeoPoint(area->maxLat_geo, area->maxLon_geo, true), GeoPoint(area->maxLat_geo, area->minLon_geo, true)) / gridSizeM;
	int count = 0; // not in area counter
	//////////////////////////////////////////////////////////////////////////
	//读取nodeOSM.txt
	//格式：nodeId \t lat \t lon \n
	//////////////////////////////////////////////////////////////////////////
	ifstream nodeIfs(folderDir + "nodeOSM.txt");
	if (!nodeIfs)
	{
		cout << "open " + folderDir + "nodeOSM.txt" + " error!\n";
		system("pause");
		exit(0);
	}
	while (nodeIfs)
	{
		double lat, lon;
		int nodeId;
		GeoPoint* pt;
		nodeIfs >> nodeId >> lat >> lon;
		if (nodeIfs.fail())
			break;
		if (inArea(lat, lon, true))
		{
			pt = new GeoPoint(lat, lon, true);
			nodesInArea.push_back(pt);
		}
		else
		{
			pt = NULL; //[ATTENTION]node在区域外不想保留用此句(edge构造时会产生bug，须自行处理)
			//pt = new GeoPoint(lat, lon, true);
			count++;
		}
		nodes.push_back(pt);
	}
	printf("nodes count = %d\n", nodes.size());
	printf("nodes in area = %d, not in area = %d\n", nodes.size() - count, count);
	nodeIfs.close();

	//////////////////////////////////////////////////////////////////////////
	//读取edgeOSM.txt
	//格式：edgeOSM.txt: edgeId \t startNodeId \t endNodeId \t figureNodeCount \t figure1Lat \t figure1Lon \t ... figureNLat \t figureNLon \n  
	//////////////////////////////////////////////////////////////////////////
	count = 0;
	std::ifstream edgeIfs(folderDir + "edgeOSM.txt");
	if (!edgeIfs)
	{
		cout << "open " + folderDir + "edgeOSM.txt" + " error!\n";
		system("pause");
		exit(0);
	}
	//初始化邻接表
	for (int i = 0; i < nodes.size(); i++)
	{
		AdjNode* head = new AdjNode();
		head->endPointId = i;
		head->next = NULL;
		adjList.push_back(head);
	}
	while (edgeIfs)
	{
		int edgeId, startNodeId, endNodeId, figureCount;
		edgeIfs >> edgeId >> startNodeId >> endNodeId >> figureCount;

		Figure* figure = new Figure();
		for (int i = 0; i < figureCount; i++)
		{
			double lat, lon;
			edgeIfs >> lat >> lon;
			figure->push_back(new GeoPoint(lat, lon, true));
		}
		if (edgeIfs.fail())
			break;
		//if (!inArea(nodes[startNodeId]->lat, nodes[startNodeId]->lon, false) && !inArea(nodes[endNodeId]->lat, nodes[endNodeId]->lon, false))
		//if (!inArea(figure->front()->lat, figure->front()->lon, false) && !inArea(figure->back()->lat, figure->back()->lon, false)) //确保首尾点在area内
		bool inAreaFlag = true;
		for (auto pt : *figure) // 确保每个点都在area内
		{
			if (!inArea(pt->lat, pt->lon, false))
			{
				inAreaFlag = false;
				break;
			}
		}
		if (!inAreaFlag)
		{
			edges.push_back(NULL);
			count++;
			continue;
		}
		Edge* edge = new Edge();
		edge->id = edgeId;
		edge->visited = false;
		edge->figure = figure;
		edge->lengthM = calEdgeLength(figure);
		edges.push_back(edge);
		insertEdge(edgeId, startNodeId, endNodeId);
	}

	printf("edges count = %d\n", edges.size());
	printf("edges in area = %d, not in area = %d\n",edges.size() - count, count);
	edgeIfs.close();

	if (loadWaytype)
	{
		ifstream wayTypeIfs(folderDir + "wayTypeOSM.txt");
		cout << "load waytype from " << folderDir + "wayTypeOSM.txt" << endl;
		while (wayTypeIfs)
		{
			int roadId, wayType;
			string wayTypeStr;
			wayTypeIfs >> roadId >> wayTypeStr >> wayType;
			if (edges[roadId] != NULL)
				edges[roadId]->type = wayType;
		}
		wayTypeIfs.close();
	}
	
	//add new version of adjEdges
	for (auto& edge : edges)
	{
		if (edge == NULL)
			continue;
		AdjNode* adjNode = adjList[edge->endNodeId];
		adjNode = adjNode->next;
		while (adjNode != NULL)
		{
			edge->adjEdgeIds.push_back(adjNode->edgeId);
			adjNode = adjNode->next;
		}
	}
	
	printf(">> reading map finished\n");
	createGridIndex();
	printf(">> creating grid index finished\n");
}

void Map::setArea(Area* area)
{
	//[ATTENTION][MEMORY_LEAK]这里并没有将原area的内存给回收，可能会造成内存泄露
	this->area = area;
}

vector<Edge*> Map::getNearEdges(double lat, double lon, double threshold) const
{
	//////////////////////////////////////////////////////////////////////////
	///返回(lat, lon)周围距离小于threshold米的所有路段
	///[注意]会产生内存泄露
	//////////////////////////////////////////////////////////////////////////
	vector<Edge*> result;
	vector<Edge*> fail;
	int gridSearchRange = int(threshold / (gridSizeDeg * GeoPoint::geoScale)) + 1;
	int rowPt = getRowId(lat);
	int colPt = getColId(lon);
	int row1 = rowPt - gridSearchRange;
	int col1 = colPt - gridSearchRange;
	int row2 = rowPt + gridSearchRange;
	int col2 = colPt + gridSearchRange;
	if (row1 < 0) row1 = 0;
	if (row2 >= gridHeight) row2 = gridHeight - 1;
	if (col1 < 0) col1 = 0;
	if (col2 >= gridWidth) col2 = gridWidth - 1;
	//cout << "gridrange = " << gridSearchRange << endl;
	for (int row = row1; row <= row2; row++)
	{
		for (int col = col1; col <= col2; col++)
		{
			for (list<Edge*>::iterator iter = grid[row][col]->begin(); iter != grid[row][col]->end(); iter++)
			{
				//if (grid[row][col]->size() != 0)
				//	cout << "grid count = " << grid[row][col]->size() << endl;
				if (!((*iter)->visited))
				{
					(*iter)->visited = true;
					double dist = distM_withThres(lat, lon, (*iter), threshold);
					if (dist < threshold)
						result.push_back((*iter));
					else
						fail.push_back((*iter));
				}
			}
		}
	}
	for (int i = 0; i < result.size(); i++)
	{
		result[i]->visited = false;
	}
	for (int i = 0; i < fail.size(); i++)
	{
		fail[i]->visited = false;
	}
	return result;
}

void Map::getNearEdges(double lat, double lon, double threshold, vector<Edge*>& dest)
{
	//////////////////////////////////////////////////////////////////////////
	///返回(lat, lon)周围距离小于threshold米的所有路段
	///PS:该函数线程不安全，多线程不要用
	//////////////////////////////////////////////////////////////////////////
	if (!inArea(lat, lon, false))
	{
		printf("[异常](%lf, %lf)不在区域内 in func Map::getNearEdges(double lat, double lon, double threshold, vector<Edge*>& dest)\n", lat, lon);
		area->print();
	}
	dest.clear();
	vector<Edge*> fail;
	int gridSearchRange = int(threshold / (gridSizeDeg * GeoPoint::geoScale)) + 1;
	int rowPt = getRowId(lat);
	int colPt = getColId(lon);
	int row1 = rowPt - gridSearchRange;
	int col1 = colPt - gridSearchRange;
	int row2 = rowPt + gridSearchRange;
	int col2 = colPt + gridSearchRange;
	if (row1 < 0) row1 = 0;
	if (row2 >= gridHeight) row2 = gridHeight - 1;
	if (col1 < 0) col1 = 0;
	if (col2 >= gridWidth) col2 = gridWidth - 1;
	//cout << "gridrange = " << gridSearchRange << endl;
	for (int row = row1; row <= row2; row++)
	{
		for (int col = col1; col <= col2; col++)
		{
			for (list<Edge*>::iterator iter = grid[row][col]->begin(); iter != grid[row][col]->end(); iter++)
			{
				//if (grid[row][col]->size() != 0)
				//	cout << "grid count = " << grid[row][col]->size() << endl;
				if (!((*iter)->visited))
				{
					//mutex mutexLock;
					//mutexLock.lock();
					(*iter)->visited = true;
					//mutexLock.unlock();

					double dist = distM_withThres(lat, lon, (*iter), threshold);
					if (dist < threshold)
						dest.push_back((*iter));
					else
						fail.push_back((*iter));
				}
			}
		}
	}
	for (int i = 0; i < dest.size(); i++)
	{
		//mutex mutexLock;
		//mutexLock.lock();
		dest[i]->visited = false;
		//mutexLock.unlock();
	}
	for (int i = 0; i < fail.size(); i++)
	{
		//mutex mutexLock;
		//mutexLock.lock();
		fail[i]->visited = false;
		//mutexLock.unlock();
	}
}

void Map::getNearEdges(double lat, double lon, int k, vector<Edge*>& dest)
{
	//////////////////////////////////////////////////////////////////////////
	///找出离(lat, lon)距离最近的k个边，按照从近到远的距离存入dest中
	///搜索方法： 先以查询点所在的格子为中心，gridSearchRange为搜索半径进行搜索。
	///然后以每次搜索半径增加gridExtendStep的幅度向外扩散搜索，直到candidates集合中元素个数
	///大于等于k停止搜索。
	//////////////////////////////////////////////////////////////////////////
	dest.clear();
	vector<pair<Edge*, double> > candidates;
	//初步搜索
	int gridSearchRange = 3;
	int rowPt = getRowId(lat);
	int colPt = getColId(lon);
	int row1 = rowPt - gridSearchRange;
	int col1 = colPt - gridSearchRange;
	int row2 = rowPt + gridSearchRange;
	int col2 = colPt + gridSearchRange;
	if (row1 < 0) row1 = 0;
	if (row1 >= gridHeight)
	{
		cout << "越界@Map::getNearEdges" << endl;
		system("pause");
	}
	if (row2 >= gridHeight) row2 = gridHeight - 1;
	if (col1 < 0) col1 = 0;
	if (col1 >= gridWidth)
	{
		cout << "越界@Map::getNearEdges" << endl;
		system("pause");
	}
	if (col2 >= gridWidth) col2 = gridWidth - 1;
	for (int row = row1; row <= row2; row++)
	{
		for (int col = col1; col <= col2; col++)
		{
			for (list<Edge*>::iterator iter = grid[row][col]->begin(); iter != grid[row][col]->end(); iter++)
			{
				if (!((*iter)->visited))
				{
					(*iter)->visited = true;
					double dist = distM(lat, lon, (*iter));
					candidates.push_back(make_pair((*iter), dist));
				}
			}
		}
	}

	int gridExtendStep = 1; //扩展步进，每次向外扩1格	
	int newRow1, newRow2, newCol1, newCol2; //记录新的搜索范围
	int preRow1 = row1;
	int preRow2 = row2;
	int preCol1 = col1;
	int preCol2 = col2;
	while (candidates.size() < k)
	{
		newRow1 = preRow1 - gridExtendStep;
		newRow2 = preRow2 + gridExtendStep;
		newCol1 = preCol1 - gridExtendStep;
		newCol2 = preCol2 + gridExtendStep;
		if (newRow1 < 0) newRow1 = 0;
		if (newRow2 >= gridHeight) newRow2 = gridHeight - 1;
		if (newCol1 < 0) newCol1 = 0;
		if (newCol2 >= gridWidth) newCol2 = gridWidth - 1;
		if (newRow1 >= gridHeight || newCol1 >= gridWidth)
		{
			cout << "异常 @getNearEdges" << endl;
			system("pause");
		}
		for (int row = newRow1; row <= newRow2; row++)
		{
			for (int col = newCol1; col <= newCol2; col++)
			{
				if (row >= preRow1 && row <= preRow2 && col >= preCol1 && col <= preCol2) //已经搜索过的就不用搜了
					continue;
				if (row >= gridHeight || col >= gridWidth)
				{
					printf("row = %d, col = %d, grid: %d * %d\n", row, col, gridHeight, gridWidth);
					system("pause");
				}
				if (grid[row][col] == NULL)
				{
					cout << "grid[row][col] = NULL !" << endl;
					system("pause");
				}
				for (list<Edge*>::iterator iter = grid[row][col]->begin(); iter != grid[row][col]->end(); iter++)
				{
					if (*iter == NULL)
					{
						cout << "NULL";
						system("pause");
					}
					if (!((*iter)->visited))
					{
						(*iter)->visited = true;
						double dist = distM(lat, lon, (*iter));
						candidates.push_back(make_pair((*iter), dist));
					}
				}

			}
		}
		preRow1 = newRow1;
		preRow2 = newRow2;
		preCol1 = newCol1;
		preCol2 = newCol2;
		if (newRow1 == 0 && newRow2 == gridHeight - 1 && newCol1 == 0 && newCol2 == gridWidth - 1)//如果搜索全图还没满足，就中断搜索
			break;
	}
	sort(candidates.begin(), candidates.end(), smallerInDist);
	for (int i = 0; i < k; i++)
	{
		dest.push_back(candidates[i].first);
	}

	//还原所有边的visitFlag
	for (int i = 0; i < candidates.size(); i++)
	{
		candidates[i].first->visited = false;
	}
	return;

}

void Map::getNearEdges_s(double lat, double lon, double threshold, vector<Edge*>& dest)
{
	//////////////////////////////////////////////////////////////////////////
	///返回(lat, lon)周围距离小于threshold米的所有路段
	///该函数是线程安全的
	//////////////////////////////////////////////////////////////////////////
	if (!inArea(lat, lon, false))
	{
		printf("[异常](%lf, %lf)不在区域内 in func Map::getNearEdges_s(double lat, double lon, double threshold, vector<Edge*>& dest)\n", lat, lon);
		area->print();
	}
	dest.clear();
	vector<Edge*> fail;
	vector<bool> visited(edges.size(), false);
	int gridSearchRange = int(threshold / (gridSizeDeg * GeoPoint::geoScale)) + 1;
	int rowPt = getRowId(lat);
	int colPt = getColId(lon);
	int row1 = rowPt - gridSearchRange;
	int col1 = colPt - gridSearchRange;
	int row2 = rowPt + gridSearchRange;
	int col2 = colPt + gridSearchRange;
	if (row1 < 0) row1 = 0;
	if (row2 >= gridHeight) row2 = gridHeight - 1;
	if (col1 < 0) col1 = 0;
	if (col2 >= gridWidth) col2 = gridWidth - 1;
	//cout << "gridrange = " << gridSearchRange << endl;
	for (int row = row1; row <= row2; row++)
	{
		for (int col = col1; col <= col2; col++)
		{
			for (list<Edge*>::iterator iter = grid[row][col]->begin(); iter != grid[row][col]->end(); iter++)
			{
				//if (grid[row][col]->size() != 0)
				//	cout << "grid count = " << grid[row][col]->size() << endl;
				if (!visited[(*iter)->id])
				{
					visited[(*iter)->id] = true;
					double dist = distM_withThres(lat, lon, (*iter), threshold);
					if (dist < threshold)
						dest.push_back((*iter));
					else
						fail.push_back((*iter));
				}
			}
		}
	}
}

/*void Map::getNearNodes(double lat, double lon, int k, vector<GeoPoint*>& dest)
{
	//////////////////////////////////////////////////////////////////////////
	///找出离(lat, lon)距离最近的k个顶点，按照从近到远的距离存入dest中
	//////////////////////////////////////////////////////////////////////////
	dest.clear();
	ptIndex.kNN(&GeoPoint(lat, lon), k, INF, dest);

}*/

double Map::shortestPathLength(int ID1, int ID2, double dist1, double dist2, double deltaT)
{
	int maxNodeNum = nodes.size();
	vector<double> dist = vector<double>(maxNodeNum);
	vector<bool> flag = vector<bool>(maxNodeNum);
	for (int i = 0; i < maxNodeNum; i++) {
		dist[i] = INF;
		flag[i] = false;
	}
	dist[ID1] = 0;
	priority_queue<NODE_DIJKSTRA> Q;
	NODE_DIJKSTRA tmp(ID1, 0);
	Q.push(tmp);
	while (!Q.empty()) {
		NODE_DIJKSTRA x = Q.top();
		Q.pop();
		int top_nodeId = x.t;
		if (x.dist > deltaT*MAXSPEED){
			return INF;
		}
		if (flag[top_nodeId]) {
			continue;
		}
		flag[top_nodeId] = true;
		if (top_nodeId == ID2) {
			break;
		}
		for (AdjNode* i = adjList[top_nodeId]->next; i != NULL; i = i->next) {
			if (dist[i->endPointId] > dist[top_nodeId] + edges[i->edgeId]->lengthM) {
				dist[i->endPointId] = dist[top_nodeId] + edges[i->edgeId]->lengthM;
				NODE_DIJKSTRA tmp(i->endPointId, dist[i->endPointId]);
				Q.push(tmp);
			}
		}
	}
	double resultLength = dist[ID2];
	if (!flag[ID2])
	{
		//printf("Warning@shortestPathLength(): 两点不可达, resultLength = %lf\n", resultLength);
		//system("pause");
	}
	return resultLength;

}

double Map::shortestPathLength(int nodeId1, int nodeId2, vector<int>& path, bool unreachable_warning /* = false*/)
{
	//////////////////////////////////////////////////////////////////////////
	///计算nodeId1到nodeId2之间的最短路，边序列存在path数组中
	///不连通返回INF，根据unreachable_warning开关决定是否报warning
	//////////////////////////////////////////////////////////////////////////
	if (path.size()!= 0)
	{
		cout << "Warning@Map::shortestPathLength(): path数组内有东西" << endl;
	}
	path.clear();
	if (nodeId1 == nodeId2)
	{
		return 0;
	}	
	int tmpId = hasEdge(nodeId1, nodeId2);
	if (tmpId >= 0)
	{
		path.push_back(tmpId);
		return edges[tmpId]->lengthM;
	}
	int maxNodeNum = nodes.size();
	vector<double> dist = vector<double>(maxNodeNum);
	vector<bool> flag = vector<bool>(maxNodeNum);
	vector<int> prev = vector<int>(maxNodeNum); //记录前驱节点编号
	for (int i = 0; i < maxNodeNum; i++) {
		dist[i] = INF;
		flag[i] = false;
		prev[i] = -1;
	}
	dist[nodeId1] = 0;
	priority_queue<NODE_DIJKSTRA> Q;
	NODE_DIJKSTRA tmp(nodeId1, 0);
	Q.push(tmp);
	while (!Q.empty()) {
		NODE_DIJKSTRA x = Q.top();
		Q.pop();
		int top_nodeId = x.t;
		if (flag[top_nodeId]) {
			continue;
		}
		flag[top_nodeId] = true;
		if (top_nodeId == nodeId2) {
			break;
		}
		for (AdjNode* i = adjList[top_nodeId]->next; i != NULL; i = i->next) {
			if (dist[i->endPointId] > dist[top_nodeId] + edges[i->edgeId]->lengthM) {
				dist[i->endPointId] = dist[top_nodeId] + edges[i->edgeId]->lengthM;
				NODE_DIJKSTRA tmp(i->endPointId, dist[i->endPointId]);
				prev[i->endPointId] = top_nodeId;
				Q.push(tmp);
			}
		}
	}
	double resultLength = dist[nodeId2];
	if (!flag[nodeId2])
	{
		if (unreachable_warning)
		{
			printf("Warning@shortestPathLength(): (node %d, node %d)两点不可达, resultLength = %lf\n", nodeId1, nodeId2, resultLength);
			system("pause");
		}
		return resultLength;
	}
	//构造路径
	vector<int> revPath; //反向节点路径
	int curNodeId = nodeId2;
	while (curNodeId != nodeId1)
	{
		revPath.push_back(curNodeId);
		curNodeId = prev[curNodeId];
	}
	revPath.push_back(nodeId1);
	
	/**********************************************************/
	/*test code starts from here
	cout << "revPathNode = ";
	for (int i = 0; i < revPath.size(); i++)
	{
		cout << revPath[i] << ",";
	}
	cout << endl;
	/*test code ends*/
	/**********************************************************/
	
	//转化成边序列
	for (int i = revPath.size() - 1; i >= 1; i--)
	{
		path.push_back(hasEdgeWithMinLen(revPath[i], revPath[i - 1]));
		//cout << "path.push_back " << hasEdge(revPath[i], revPath[i - 1]) << endl;
	}

	
	/**********************************************************/
	/*test code starts from here
	cout << "path @ SP = ";
	for (int i = 0; i < path.size(); i++)
	{
		cout << path[i] << ",";
	}
	cout << endl;
	test code ends*/
	/**********************************************************/
	

	//check
	double totLen = 0.0;
	for (int i = 0; i < path.size(); i++)
	{
		totLen += edges[path[i]]->lengthM;
	}
	if (fabs(totLen - resultLength) > eps)
	{
		cout << "Error@shortestPathLength():边序列计算有误" << endl;
		printf("totLen = %lf, resultLen = %lf\n", totLen, resultLength);
		//system("pause");
	}
	return resultLength;
}

double Map::shortestPathLength_for_MM(int nodeId1, int nodeId2, vector<int>& path, double deltaT, double maxSpd_MPS, bool unreachable_warning /* = false*/)
{
	//////////////////////////////////////////////////////////////////////////
	///计算nodeId1到nodeId2之间的最短路，边序列存在path数组中
	///不连通返回INF，根据unreachable_warning开关决定是否报warning
	///该版本为map matching服务，deltaT需要自行输入，表示相邻两个采样点的时间间隔，单位秒
	///maxSpd_MPS表示常识的最大车速，单位米每秒
	///算法为A*算法
	//////////////////////////////////////////////////////////////////////////
	if (path.size() != 0)
	{
		cout << "Warning@Map::shortestPathLength(): path数组内有东西" << endl;
	}
	path.clear(); 
	if (nodeId1 == nodeId2)
	{
		return 0;
	}
	int tmpId = hasEdgeWithMinLen(nodeId1, nodeId2);
	if (tmpId >= 0)
	{
		path.push_back(tmpId);
		return edges[tmpId]->lengthM;
	}
	//lets start
	int maxNodeNum = nodes.size();
	vector<double> dist = vector<double>(maxNodeNum);
	vector<bool> flag = vector<bool>(maxNodeNum);
	vector<int> prev = vector<int>(maxNodeNum); //记录前驱节点编号
	for (int i = 0; i < maxNodeNum; i++) {
		dist[i] = INF;
		flag[i] = false;
		prev[i] = -1;
	}
	dist[nodeId1] = 0;
	priority_queue<NODE_DIJKSTRA> Q;
	//NODE_DIJKSTRA tmp(nodeId1, 0);
	NODE_DIJKSTRA tmp(nodeId1, GeoPoint::distM(nodes[nodeId1], nodes[nodeId2]));// 
	Q.push(tmp);
	while (!Q.empty()) {
		NODE_DIJKSTRA x = Q.top();
		Q.pop();
		int top_nodeId = x.t;
		if (x.dist > deltaT*maxSpd_MPS){ //剪枝
			return INF;
		}
		if (flag[top_nodeId]) {
			continue;
		}
		flag[top_nodeId] = true;
		if (top_nodeId == nodeId2) {
			break;
		}
		for (AdjNode* i = adjList[top_nodeId]->next; i != NULL; i = i->next) {
			if (dist[i->endPointId] > dist[top_nodeId] + edges[i->edgeId]->lengthM) {
				dist[i->endPointId] = dist[top_nodeId] + edges[i->edgeId]->lengthM;
				//NODE_DIJKSTRA  tmp(i->endPointId, dist[i->endPointId]);
				NODE_DIJKSTRA tmp(i->endPointId, dist[i->endPointId] + GeoPoint::distM(nodes[i->endPointId], nodes[nodeId2]));
				prev[i->endPointId] = top_nodeId;
				Q.push(tmp);
			}
		}
	}
	double resultLength = dist[nodeId2];
	if (!flag[nodeId2])
	{
		if (unreachable_warning)
		{
			printf("Warning@shortestPathLength(): (node %d, node %d)两点不可达, resultLength = %lf\n", nodeId1, nodeId2, resultLength);
			system("pause");
		}
		return resultLength;
	}
	//构造路径
	vector<int> revPath; //反向节点路径
	int curNodeId = nodeId2;
	while (curNodeId != nodeId1)
	{
		revPath.push_back(curNodeId);
		curNodeId = prev[curNodeId];
	}
	revPath.push_back(nodeId1);

	/**********************************************************/
	/*test code starts from here
	cout << "revPathNode = ";
	for (int i = 0; i < revPath.size(); i++)
	{
	cout << revPath[i] << ",";
	}
	cout << endl;
	/*test code ends*/
	/**********************************************************/

	//转化成边序列
	for (int i = revPath.size() - 1; i >= 1; i--)
	{
		path.push_back(hasEdgeWithMinLen(revPath[i], revPath[i - 1]));
		//cout << "path.push_back " << hasEdge(revPath[i], revPath[i - 1]) << endl;
	}


	/**********************************************************/
	/*test code starts from here
	cout << "path @ SP = ";
	for (int i = 0; i < path.size(); i++)
	{
	cout << path[i] << ",";
	}
	cout << endl;
	test code ends*/
	/**********************************************************/


	//check
	double totLen = 0.0;
	for (int i = 0; i < path.size(); i++)
	{
		totLen += edges[path[i]]->lengthM;
	}
	if (fabs(totLen - resultLength) > eps)
	{
		cout << "Error@shortestPathLength():边序列计算有误" << endl;
		printf("totLen = %lf, resultLen = %lf\n", totLen, resultLength);
		//system("pause");
	}
	return resultLength;
}

double Map::shortestPathLength_for_MM_v2(int nodeId1, int nodeId2, vector<int>& path, double deltaT, double maxSpd_MPS, map<int, double>& cache, bool use_prune_strategy /* = true */, bool unreachable_warning /* = false*/)
{
	//////////////////////////////////////////////////////////////////////////
	///计算nodeId1到nodeId2之间的最短路，边序列存在path数组中
	///不连通返回INF，根据unreachable_warning开关决定是否报warning
	///该版本为map matching服务，deltaT需要自行输入，表示相邻两个采样点的时间间隔，单位秒
	///maxSpd_MPS表示常识的最大车速，单位米每秒
	///算法为A*算法
	///v2版本，在计算最短路过程中会将中间经过的节点cache
	//////////////////////////////////////////////////////////////////////////
	
	if (path.size() != 0)
	{
		cout << "Warning@Map::shortestPathLength(): path数组内有东西" << endl;
	}
	path.clear();
	if (nodeId1 == nodeId2)
	{
		return 0;
	}
	int tmpId = hasEdgeWithMinLen(nodeId1, nodeId2);
	if (tmpId >= 0)
	{
		path.push_back(tmpId);
		return edges[tmpId]->lengthM;
	}
	//Time t1 = Timer::getTime();
	//lets start
	int maxNodeNum = nodes.size();
	/*double dist[200000];
	bool flag[200000];
	int prev[200000];
	memset(dist, INF, sizeof(dist));
	memset(flag, false, sizeof(flag));
	memset(prev, -1, sizeof(prev));*/
	vector<double> dist = vector<double>(maxNodeNum, INF);
	vector<bool> flag = vector<bool>(maxNodeNum, false);
	vector<int> prev = vector<int>(maxNodeNum, -1); //记录前驱节点编号

	//Time t2 = Timer::getTime();
	//initialTimeInSP += (t2 - t1);
	/*for (int i = 0; i < maxNodeNum; i++) {
		dist[i] = INF;
		flag[i] = false;
		prev[i] = -1;
	}*/
	dist[nodeId1] = 0;
	priority_queue<NODE_DIJKSTRA> Q;
	//NODE_DIJKSTRA tmp(nodeId1, 0); //Dijkstra
	NODE_DIJKSTRA tmp(nodeId1, GeoPoint::distM(nodes[nodeId1], nodes[nodeId2]));// A*
	Q.push(tmp);
	while (!Q.empty()) 
	{
		NODE_DIJKSTRA x = Q.top();
		Q.pop();
		int top_nodeId = x.t;
		if (use_prune_strategy && x.dist > deltaT*maxSpd_MPS){ //剪枝
			return INF;
		}
		if (flag[top_nodeId]) {
			continue;
		}
		flag[top_nodeId] = true;

		//caching
		//Time t1 = Timer::getTime();
		map<int, double>::iterator queryIter = cache.find(top_nodeId);
		if (queryIter == cache.end())
			cache.insert(make_pair(top_nodeId, dist[top_nodeId]));
		//Time t2 = Timer::getTime();
		//mapQueryTime_for_cache += (t2 - t1);

		if (top_nodeId == nodeId2) {
			break;
		}
		for (AdjNode* i = adjList[top_nodeId]->next; i != NULL; i = i->next) 
		{
			if (dist[i->endPointId] > dist[top_nodeId] + edges[i->edgeId]->lengthM) {
				dist[i->endPointId] = dist[top_nodeId] + edges[i->edgeId]->lengthM;
				//NODE_DIJKSTRA  tmp(i->endPointId, dist[i->endPointId]); //Dijkstra
				NODE_DIJKSTRA tmp(i->endPointId, dist[i->endPointId] + GeoPoint::distM(nodes[i->endPointId], nodes[nodeId2])); //A*
				prev[i->endPointId] = top_nodeId;
				Q.push(tmp);
			}
			/*if (i->endPointId == nodeId2)
			{
				//return dist[nodeId2];
				break;
			}*/
		}
	}
	double resultLength = dist[nodeId2];
	if (!flag[nodeId2])
	{
		if (unreachable_warning)
		{
			printf("Warning@shortestPathLength(): (node %d, node %d)两点不可达, resultLength = %lf\n", nodeId1, nodeId2, resultLength);
			system("pause");
		}
		return resultLength;
	}
	//构造路径
	vector<int> revPath; //反向节点路径
	int curNodeId = nodeId2;
	while (curNodeId != nodeId1)
	{
		revPath.push_back(curNodeId);
		curNodeId = prev[curNodeId];
	}
	revPath.push_back(nodeId1);

	/**********************************************************/
	/*test code starts from here
	cout << "revPathNode = ";
	for (int i = 0; i < revPath.size(); i++)
	{
	cout << revPath[i] << ",";
	}
	cout << endl;
	/*test code ends*/
	/**********************************************************/

	//转化成边序列
	for (int i = revPath.size() - 1; i >= 1; i--)
	{
		path.push_back(hasEdgeWithMinLen(revPath[i], revPath[i - 1]));
		//cout << "path.push_back " << hasEdge(revPath[i], revPath[i - 1]) << endl;
	}


	/**********************************************************/
	/*test code starts from here
	cout << "path @ SP = ";
	for (int i = 0; i < path.size(); i++)
	{
	cout << path[i] << ",";
	}
	cout << endl;
	test code ends*/
	/**********************************************************/


	//check
	double totLen = 0.0;
	for (int i = 0; i < path.size(); i++)
	{
		totLen += edges[path[i]]->lengthM;
	}
	if (fabs(totLen - resultLength) > eps)
	{
		cout << "Error@shortestPathLength():边序列计算有误" << endl;
		printf("totLen = %lf, resultLen = %lf\n", totLen, resultLength);
		//system("pause");
	}
	return resultLength;
}

void restoreStates(vector<int>& changedList, double* dist, bool* flag, int* prev)
{
	//Time t1 = Timer::getTime();
	for (int i = 0; i < changedList.size(); i++)
	{
		dist[changedList[i]] = INF;
		flag[changedList[i]] = false;
		prev[changedList[i]] = -1;
	}
	//Time t2 = Timer::getTime();
	//initialTimeInSP += (t2 - t1);
	//check
	/*for (int i = 0; i < 100000; i++)
	{
		if (dist[i] < INF || flag[i] || prev[i] != -1)
		{
			printf("dist: %lf, flag: %d, prev: %d", dist[i], flag[i], prev[i]);
			system("pause");
		}
	}*/
}
void restoreStates(stack<int>& changedList, double* dist, bool* flag, int* prev)
{
	//Time t1 = Timer::getTime();
	while (!changedList.empty())
	{
		int id = changedList.top();
		changedList.pop();
		dist[id] = INF;
		flag[id] = false;
		prev[id] = -1;
	}
	//Time t2 = Timer::getTime();
	//initialTimeInSP += (t2 - t1);
	//check
	/*for (int i = 0; i < 100000; i++)
	{
	if (dist[i] < INF || flag[i] || prev[i] != -1)
	{
	printf("dist: %lf, flag: %d, prev: %d", dist[i], flag[i], prev[i]);
	system("pause");
	}
	}*/
}

double Map::shortestPathLength_for_MM_v3(int nodeId1, int nodeId2, vector<int>& path, double deltaT, double maxSpd_MPS, map<int, double>& cache, double* dist, bool* flag, int* prev, bool use_intermediate_cache /* = true */, bool use_prune_strategy /* = true */, bool unreachable_warning /* = false*/)
{
	//////////////////////////////////////////////////////////////////////////
	///计算nodeId1到nodeId2之间的最短路，边序列存在path数组中
	///不连通返回INF，根据unreachable_warning开关决定是否报warning
	///该版本为map matching服务，deltaT需要自行输入，表示相邻两个采样点的时间间隔，单位秒
	///maxSpd_MPS表示常识的最大车速，单位米每秒
	///算法为A*算法
	///v3版本，在v2版本的基础上每次调用不创建dist、flag、prev数组，而是使用传进来的参数，运行完后会将改动重置，速度比v2快若干倍
	///use_prune_strategy = true 则会进行剪枝策略：if x.dist > deltaT*maxSpd_MPS return INF
	///use_intermediate_cache = true 则将会在计算最短路过程中涉及到的其他点的dist一起记录至cache
	//////////////////////////////////////////////////////////////////////////
	vector<int> changedList;
	//stack<int> changedList;
	if (path.size() != 0)
	{
		cout << "Warning@Map::shortestPathLength(): path数组内有东西" << endl;
	}
	path.clear();
	if (nodeId1 == nodeId2)
	{
		return 0;
	}
	int tmpId = hasEdgeWithMinLen(nodeId1, nodeId2);
	if (tmpId >= 0)
	{
		path.push_back(tmpId);
		return edges[tmpId]->lengthM;
	}

	dist[nodeId1] = 0;
	changedList.push_back(nodeId1);
	//changedList.push(nodeId1);
	priority_queue<NODE_DIJKSTRA> Q;
	//NODE_DIJKSTRA tmp(nodeId1, 0); //Dijkstra
	NODE_DIJKSTRA tmp(nodeId1, GeoPoint::distM(nodes[nodeId1], nodes[nodeId2]));// A*
	Q.push(tmp);
	while (!Q.empty())
	{
		NODE_DIJKSTRA x = Q.top();
		Q.pop();
		int top_nodeId = x.t;
		if (use_prune_strategy && x.dist > deltaT*maxSpd_MPS){ //剪枝
			restoreStates(changedList, dist, flag, prev);
			return INF;
		}
		if (flag[top_nodeId]) {
			continue;
		}
		flag[top_nodeId] = true;

		//caching intermediate dists
		if (use_intermediate_cache)
		{
			//Time t1 = Timer::getTime();
			map<int, double>::iterator queryIter = cache.find(top_nodeId);
			if (queryIter == cache.end())
				cache.insert(make_pair(top_nodeId, dist[top_nodeId]));
			//Time t2 = Timer::getTime();
			//mapQueryTime_for_cache += (t2 - t1);
		}
		
		if (top_nodeId == nodeId2) {
			break;
		}
		for (AdjNode* i = adjList[top_nodeId]->next; i != NULL; i = i->next)
		{
			if (dist[i->endPointId] > dist[top_nodeId] + edges[i->edgeId]->lengthM) {
				dist[i->endPointId] = dist[top_nodeId] + edges[i->edgeId]->lengthM;
				//NODE_DIJKSTRA  tmp(i->endPointId, dist[i->endPointId]); //Dijkstra
				NODE_DIJKSTRA tmp(i->endPointId, dist[i->endPointId] + GeoPoint::distM(nodes[i->endPointId], nodes[nodeId2])); //A*
				prev[i->endPointId] = top_nodeId;
				Q.push(tmp);

				changedList.push_back(i->endPointId);
				//changedList.push(i->endPointId);
			}
			/*if (i->endPointId == nodeId2)
			{
			//return dist[nodeId2];
			break;
			}*/
		}
	}
	double resultLength = dist[nodeId2];
	if (!flag[nodeId2])
	{
		if (unreachable_warning)
		{
			printf("Warning@shortestPathLength(): (node %d, node %d)两点不可达, resultLength = %lf\n", nodeId1, nodeId2, resultLength);
			system("pause");
		}
		restoreStates(changedList, dist, flag, prev);
		return resultLength;
	}
	//构造路径
	vector<int> revPath; //反向节点路径
	int curNodeId = nodeId2;
	while (curNodeId != nodeId1)
	{
		revPath.push_back(curNodeId);
		curNodeId = prev[curNodeId];
	}
	revPath.push_back(nodeId1);

	//转化成边序列
	for (int i = revPath.size() - 1; i >= 1; i--)
	{
		path.push_back(hasEdgeWithMinLen(revPath[i], revPath[i - 1]));
		//cout << "path.push_back " << hasEdge(revPath[i], revPath[i - 1]) << endl;
	}

	//check
	double totLen = 0.0;
	for (int i = 0; i < path.size(); i++)
	{
		totLen += edges[path[i]]->lengthM;
	}
	if (fabs(totLen - resultLength) > eps)
	{
		cout << "Error@shortestPathLength():边序列计算有误" << endl;
		printf("totLen = %lf, resultLen = %lf\n", totLen, resultLength);
		//system("pause");
	}
	restoreStates(changedList, dist, flag, prev);
	return resultLength;
}

double tortuosity(Edge* edge)
{
	//////////////////////////////////////////////////////////////////////////
	///
	//////////////////////////////////////////////////////////////////////////
	double totLen = 0;
	return edge->lengthM / GeoPoint::distM(edge->figure->front(), edge->figure->back());
}

void Map::goAlongPath_and_split(GeoPoint startPos, GeoPoint endPos, vector<int>& path, vector<double>& ratios, vector<GeoPoint>& splitPts)
{
	//////////////////////////////////////////////////////////////////////////
	///从startPos到endPos，经过path的路程中，根据ratios的配比返回路程中的点，满足每段之间的比值满足ratios
	///其中startPos和endPos必须在path.front()和path.back()上
	///path为边序列，保存的是经过的EdgeID
	///ratios是一个和为1的数组，为路程每段长度的配比 [输入可以和不为1，会自动归一化]
	///splitPts为结果数组，记录除了起始点外的中间所有切开的断点坐标
	//////////////////////////////////////////////////////////////////////////
	//printf("startPos to edge %d = %lf; ", edges[path.front()]->id, distM(startPos.lat, startPos.lon, edges[path.front()])); //should be 0
	//printf("endPos to edge %d = %lf\n", edges[path.back()]->id, distM(endPos.lat, endPos.lon, edges[path.back()])); //should be 0

	//Normalization for ratios
	double tot_ratios = 0;
	for (int i = 0; i < ratios.size(); i++)
	{
		tot_ratios += ratios[i];
	}
	for (int i = 0; i < ratios.size(); i++)
	{
		ratios[i] /= tot_ratios;
	}
	//puts("phase 1 pass");
	//check startPos/endPos是否落在相应路段上，同时顺带计算到路段首的距离
	double prjDist1_M, prjDist2_M;
	//startPos.print();
	//endPos.print();
	//puts("phase 2 pass");
	//cout << "path.front = " << path.front() << endl;
	//puts("phase 3 pass");
	//cout << edges[path.front()]->id << endl;
	//puts("phase 4 pass");
	if (abs(distM(startPos.lat, startPos.lon, edges[path.front()], prjDist1_M)) > eps)
	{
		puts("Error@Map::goAlongPath_and_split(),startPosition不是Edge的投影点");
		cout << distM(startPos.lat, startPos.lon, edges[path.front()], prjDist1_M) << endl;
		system("pause");
	}
	if (abs(distM(endPos.lat, endPos.lon, edges[path.back()], prjDist2_M)) > eps)
	{
		puts("Error@Map::goAlongPath_and_split(),endPosition不是Edge的投影点");
		cout << distM(endPos.lat, endPos.lon, edges[path.back()], prjDist2_M) << endl;
		system("pause");
	}
	//puts("phase 5 pass");
	////计算总共需要走的路程长度（米）
	double tot_distToGoM = 0;
	if (path.size() == 1) //在一条路上的情况
	{
		tot_distToGoM = prjDist2_M - prjDist1_M;
	}
	else
	{
		double headDist = edges[path[0]]->lengthM - prjDist1_M;
		double tailDist = prjDist2_M;
		tot_distToGoM += (headDist + tailDist);
		if (path.size() > 2) //path是三条及以上道路相接而成的需要计算中间道路长度
		{
			for (int i = 1; i <= path.size() - 2; i++)
			{
				tot_distToGoM += edges[path[i]]->lengthM;
			}
		}		
	}
	//puts("phase 6 pass");
	////开始遍历
	int cur_index_in_path = 0; //记录当前位于的path数组的位置
	Edge* cur_edge = edges[path[cur_index_in_path]]; //记录当前位于的Edge
	double offset_in_curEdge = prjDist1_M; //记录当前位置距离当前Edge起点的距离

	for (int i = 0; i < ratios.size() - 1; i++) //最后一个不需要处理
	{
		double distToGoM = ratios[i] * tot_distToGoM; //当前需要走多少路到下一个断点
		if (distToGoM <= (cur_edge->lengthM - offset_in_curEdge)) //如果下一个断点也在同一条路上
		{
			splitPts.push_back(goAlongEdge(cur_edge, offset_in_curEdge, distToGoM));
			/**********************************************************/
			/*test code starts from here*/
			GeoPoint pt = goAlongEdge(cur_edge, offset_in_curEdge, distToGoM);
			if (!area->inArea(pt.lat, pt.lon, false))
			{
				pt.print();
				system("pause");
			}
			/*test code ends*/
			/**********************************************************/
			offset_in_curEdge += distToGoM;
		}
		else
		{
			distToGoM -= (cur_edge->lengthM - offset_in_curEdge); //将当前这段路走完
			cur_index_in_path++;
			cur_edge = edges[path[cur_index_in_path]];
			offset_in_curEdge = 0.0;
			int _count = 0; //记录循环次数
			while (1)
			{
				if (_count > 100000) //防止不明原因造成的死循环
				{
					puts("Error@Map::goAlongPath_and_split():在while中死循环");
					system("pause");
				}
				if (distToGoM > cur_edge->lengthM) //端点不在当前路上
				{
					//前往下一条路
					distToGoM -= cur_edge->lengthM;
					cur_index_in_path++;
					cur_edge = edges[path[cur_index_in_path]];
					offset_in_curEdge = 0.0;
				}
				else //端点在当前路上
				{
					splitPts.push_back(goAlongEdge(cur_edge, offset_in_curEdge, distToGoM)); //这儿的offset_in_curEdge其实就是0
					offset_in_curEdge = distToGoM; //只更新offset，其他不变
					break;
				}
				_count++;
			}
		}
	}
	//puts("function done");
}

GeoPoint Map::goAlongEdge(Edge* edge, double offsetM, double _distToGoM)
{
	//////////////////////////////////////////////////////////////////////////
	///从距离edge的头部offsetM的地方开始沿着edge走_distM米，返回目的地的位置
	///edge为空报Warning返回一个(0,0)
	///offsetM需要小于路长
	///offset+_distToGoM不能超过路长
	//////////////////////////////////////////////////////////////////////////
	
	if (edge == NULL)
	{
		puts("Warning@Map::goAlongEdge():edge == NULL");
		system("pause");
		return GeoPoint(0,0);
	}
	if (offsetM > edge->lengthM)
	{
		puts("Error@Map::goAlongEdge():起始距离超出路段长度");
		system("pause");
	}
	double distToGoM = offsetM + _distToGoM;
	
	if (abs(distToGoM - edge->lengthM) < eps) //如果要走到底，那么直接返回（不这么做，下面的while会出bug）
	{
		return GeoPoint(*nodes[edge->endNodeId]);
	}
	
	if (distToGoM > edge->lengthM)
	{
		puts("Error@Map::goAlongEdge():目的地超出了路段");
		system("pause");
	}
	
	
	Figure::iterator iter = edge->figure->begin();
	Figure::iterator nextIter = edge->figure->begin();
	nextIter++;
	GeoPoint dest;
	int _count = 0;
	while (nextIter != edge->figure->end())
	{
		if (_count > 100000) //防止不明原因造成的死循环
		{
			puts("Error@Map::goAlongEdge():在while中死循环");
			system("pause");
		}
	
		double segDist = GeoPoint::distM(*iter, *nextIter);
		if (distToGoM - segDist > 0)
		{
			distToGoM -= segDist;
		}
		else
		{
			double x1 = (*iter)->lon;
			double y1 = (*iter)->lat;
			double x2 = (*nextIter)->lon;
			double y2 = (*nextIter)->lat;
			double tmpLon = x1 + (x2 - x1) * distToGoM / segDist;
			double tmpLat = y1 + (y2 - y1) * distToGoM / segDist;
			dest.lat = tmpLat;
			dest.lon = tmpLon;
			break;
		}
		iter++;
		nextIter++;
		_count++;
	}
	return dest;
}

double Map::distM(double lat, double lon, Edge* edge) const
{
	//////////////////////////////////////////////////////////////////////////
	///返回点(lat, lon)到边edge的精确距离
	///距离定义为：min(点到可投影边的投影距离，点到所有形状点的欧氏距离)
	//////////////////////////////////////////////////////////////////////////
	double minDist = INF;
	if (edge == NULL)
	{
		cout << "Error@Map::distM(double lat, double lon, Edge* edge), edge is NULL" << endl;
		system("pause");
	}
		
	//遍历端点距离
	for (Figure::iterator iter = edge->figure->begin(); iter != edge->figure->end(); iter++)
	{
		double tmpDist = GeoPoint::distM(lat, lon, (*iter)->lat, (*iter)->lon);
		if (tmpDist < minDist)
			minDist = tmpDist;
	}
	//遍历投影距离
	Figure::iterator iter = edge->figure->begin();
	Figure::iterator nextIter = edge->figure->begin();
	nextIter++;
	while (nextIter != edge->figure->end())
	{
		//有投影
		GeoPoint pt(lat, lon);
		if (cosAngle(&pt, (*iter), (*nextIter)) <= 0 && cosAngle(&pt, (*nextIter), (*iter)) <= 0)
		{
			double A = ((*nextIter)->lat - (*iter)->lat);
			double B = -((*nextIter)->lon - (*iter)->lon);
			double C = (*iter)->lat * ((*nextIter)->lon - (*iter)->lon)
				- (*iter)->lon * ((*nextIter)->lat - (*iter)->lat);
			double tmpDist = abs(A * pt.lon + B * pt.lat + C) / sqrt(A * A + B * B);
			tmpDist *= GeoPoint::geoScale;
			if (minDist > tmpDist)
				minDist = tmpDist;
		}
		iter++;
		nextIter++;
	}
	return minDist;
}

double Map::distM(double lat, double lon, Edge* edge, double& prjDist) const
{
	//////////////////////////////////////////////////////////////////////////
	///返回点(lat, lon)到边edge的精确距离
	///距离定义为：min(点到可投影边的投影距离，点到所有形状点的欧氏距离)
	///如果有投影的话，prjDist记录投影点到Edge起点的距离
	//////////////////////////////////////////////////////////////////////////
	if (edge == NULL)
	{
		cout << "Error@Map::distM(double lat, double lon, Edge* edge, double& prjDist), edge is NULL" << endl;
		system("pause");
	}
	
	/**********************************************************/
	/*test code starts from here*/
	/*if (tortuosity(edge) < 1.00005)
	{
		//printf("exact: prjDist = %lf, vertDist = %lf\n", tempTotalPrjDist, minDist);
		double verticalDist;
		GeoPoint pt(lat, lon);
		GeoPoint* node_s = edge->figure->front();
		GeoPoint* node_e = edge->figure->back();
		if (cosAngle(&pt, node_s, node_e) <= 0 && cosAngle(&pt, node_e, node_s) <= 0)
		{
			double A = (node_e->lat - node_s->lat);
			double B = -(node_e->lon - node_s->lon);
			double C = node_s->lat * (node_e->lon - node_s->lon) - node_s->lon * (node_e->lat - node_s->lat);
			verticalDist = abs(A * pt.lon + B * pt.lat + C) / sqrt(A * A + B * B);
			double tmp = GeoPoint::distM(&pt, node_s);
			prjDist = sqrt(tmp * tmp - verticalDist * verticalDist);
			//return verticalDist;
		}
		else
		{
			double dist2s = GeoPoint::distM(&pt, node_s);
			double dist2e = GeoPoint::distM(&pt, node_e);
			if (dist2s < dist2e)
			{
				prjDist = 0;
				verticalDist = dist2s;
				//return dist2s;
			}
			else
			{
				prjDist = edge->lengthM;
				double verticalDist = dist2e;
				//return dist2e;
			}
		}
		//printf("approx: prjDist = %lf, vertDist = %lf\n", prjDist, verticalDist);
		//system("pause");
		return verticalDist;
	}*/
	/*test code ends*/
	/**********************************************************/

	Figure::iterator iter = edge->figure->begin();
	Figure::iterator nextIter = edge->figure->begin();
	nextIter++;
	prjDist = 0;
	double frontSegmentDist = 0;
	double tempTotalPrjDist = 0;
	double minDist = INF;
	//遍历端点距离
	while (nextIter != edge->figure->end())
	{
		double tmpDist = GeoPoint::distM(lat, lon, (*iter)->lat, (*iter)->lon);
		if (tmpDist < minDist)
		{
			minDist = tmpDist;
			tempTotalPrjDist = frontSegmentDist;
		}
		frontSegmentDist += GeoPoint::distM((*iter), (*nextIter));
		iter++;
		nextIter++;
	}
	//补最后一个点
	double tmpDist = GeoPoint::distM(lat, lon, (*iter)->lat, (*iter)->lon);
	if (tmpDist < minDist)
	{
		minDist = tmpDist;
		tempTotalPrjDist = frontSegmentDist;
	}
	//遍历投影距离
	frontSegmentDist = 0;
	iter = edge->figure->begin();
	nextIter = edge->figure->begin();
	nextIter++;
	while (nextIter != edge->figure->end())
	{
		//有投影
		GeoPoint pt(lat, lon);
		if (cosAngle(&pt, (*iter), (*nextIter)) <= 0 && cosAngle(&pt, (*nextIter), (*iter)) <= 0)
		{
			double A = ((*nextIter)->lat - (*iter)->lat);
			double B = -((*nextIter)->lon - (*iter)->lon);
			double C = (*iter)->lat * ((*nextIter)->lon - (*iter)->lon)
				- (*iter)->lon * ((*nextIter)->lat - (*iter)->lat);
			double tmpDist = abs(A * pt.lon + B * pt.lat + C) / sqrt(A * A + B * B);
			tmpDist *= GeoPoint::geoScale;
			if (minDist > tmpDist)
			{
				minDist = tmpDist;
				double tmpPjDist = GeoPoint::distM(&pt, (*iter));
				tmpPjDist *= -cosAngle(&pt, (*iter), (*nextIter));
				tempTotalPrjDist = frontSegmentDist + tmpPjDist;
			}
		}
		frontSegmentDist += GeoPoint::distM((*iter), (*nextIter));
		iter++;
		nextIter++;
	}
	prjDist = tempTotalPrjDist;


	
	return minDist;
}

double Map::distM(double lat, double lon, Edge* edge, GeoPoint& projection) const
{
	//////////////////////////////////////////////////////////////////////////
	///返回点(lat, lon)到边edge的精确距离
	///距离定义为：min(点到可投影边的投影距离，点到所有形状点的欧氏距离)
	///如果有投影的话，projection记录投影点坐标
	//////////////////////////////////////////////////////////////////////////
	double prjDist = 0;
	if (edge == NULL)
	{
		cout << "Error@Map::distM(double lat, double lon, Edge* edge, GeoPoint& projection), edge is NULL" << endl;
		system("pause");
	}
	Figure::iterator iter = edge->figure->begin();
	Figure::iterator nextIter = edge->figure->begin();
	nextIter++;
	//prjDist = 0;
	double frontSegmentDist = 0;
	double tempTotalPrjDist = 0;
	double minDist = INF;
	//遍历端点距离
	while (nextIter != edge->figure->end())
	{
		double tmpDist = GeoPoint::distM(lat, lon, (*iter)->lat, (*iter)->lon);
		if (tmpDist < minDist)
		{
			minDist = tmpDist;
			tempTotalPrjDist = frontSegmentDist;
			projection.lat = (*iter)->lat;
			projection.lon = (*iter)->lon;
		}
		frontSegmentDist += GeoPoint::distM((*iter), (*nextIter));
		iter++;
		nextIter++;
	}
	//补最后一个点
	double tmpDist = GeoPoint::distM(lat, lon, (*iter)->lat, (*iter)->lon);
	if (tmpDist < minDist)
	{
		minDist = tmpDist;
		tempTotalPrjDist = frontSegmentDist;
		projection.lat = (*iter)->lat;
		projection.lon = (*iter)->lon;
	}
	//遍历投影距离
	frontSegmentDist = 0;
	iter = edge->figure->begin();
	nextIter = edge->figure->begin();
	nextIter++;
	while (nextIter != edge->figure->end())
	{
		//有投影
		GeoPoint pt(lat, lon);
		if (cosAngle(&pt, (*iter), (*nextIter)) <= 0 && cosAngle(&pt, (*nextIter), (*iter)) <= 0)
		{
			double A = ((*nextIter)->lat - (*iter)->lat);
			double B = -((*nextIter)->lon - (*iter)->lon);
			double C = (*iter)->lat * ((*nextIter)->lon - (*iter)->lon)
				- (*iter)->lon * ((*nextIter)->lat - (*iter)->lat);
			double tmpDist = abs(A * pt.lon + B * pt.lat + C) / sqrt(A * A + B * B);
			tmpDist *= GeoPoint::geoScale;
			if (minDist > tmpDist)
			{
				minDist = tmpDist;
				double tmpPjDist = GeoPoint::distM(&pt, (*iter));
				tmpPjDist *= -cosAngle(&pt, (*iter), (*nextIter));
				tempTotalPrjDist = frontSegmentDist + tmpPjDist;

				//projection
				double C_ = A * pt.lat - B * pt.lon;
				projection.lat = (A*C_ - B*C) / (A*A + B*B);
				projection.lon = (-A*C - B*C_) / (A*A + B*B);

				//check correctness
				if (abs(tmpDist - GeoPoint::distM(lat, lon, projection.lat, projection.lon)) > 1e-5)
				{
					cout << "[异常] 投影计算有误" << endl;
					system("pause");
				}
			}
		}
		frontSegmentDist += GeoPoint::distM((*iter), (*nextIter));
		iter++;
		nextIter++;
	}
	prjDist = tempTotalPrjDist;
	return minDist;
}

double Map::distMFromTransplantFromSRC(double lat, double lon, Edge* edge, double& prjDist)
{
	//////////////////////////////////////////////////////////////////////////
	///移植SRC版本：返回(lat,lon)点到edge的距离，单位为米；同时记录投影点到edge起点的距离存入prjDist
	//////////////////////////////////////////////////////////////////////////
	double tmpSideLen = 0;
	double result = 1e80, tmp = 0;
	double x = -1, y = -1;
	for (Figure::iterator figIter = edge->figure->begin(); figIter != edge->figure->end(); figIter++){
		if (x != -1 && y != -1){
			double x2 = (*figIter)->lat;
			double y2 = (*figIter)->lon;
			double dist = GeoPoint::distM(x, y, lat, lon); //circle Distance(x, y, nodeX, nodeY);
			if (dist<result){
				result = dist;
				tmpSideLen = tmp;
			}
			double vecX1 = x2 - x;
			double vecY1 = y2 - y;
			double vecX2 = lat - x;
			double vecY2 = lon - y;
			double vecX3 = lat - x2;
			double vecY3 = lon - y2;
			if (vecX1*vecX2 + vecY1*vecY2>0 && -vecX1*vecX3 - vecY1*vecY3 > 0 && (vecX1 != 0 || vecY1 != 0)){
				double rate = ((lat - x2)*vecX1 + (lon - y2)*vecY1) / (-vecX1*vecX1 - vecY1*vecY1);
				double nearX = rate*x + (1 - rate)*x2, nearY = rate*y + (1 - rate)*y2;
				double dist = GeoPoint::distM(nearX, nearY, lat, lon);
				if (dist < result){
					result = dist;
					tmpSideLen = tmp + GeoPoint::distM(x, y, nearX, nearY);
				}
			}
			tmp += GeoPoint::distM(x, y, x2, y2);
		}
		x = (*figIter)->lat;
		y = (*figIter)->lon;
	}
	prjDist = tmpSideLen;
	return result;
}

bool Map::getCompleteRoute(vector<int>& subRoute, vector<int>& route_ans)
{
	//////////////////////////////////////////////////////////////////////////
	///subRoute: 可以有重复，可以不连通。典型用法：对点序列MM后得到的边序列即可用作subRoute
	///route_ans: 所求得的边序列，满足同一条路段不会连续出现多次，路段与路段之间是相邻的
	///return: false 如果路径不可达
	//////////////////////////////////////////////////////////////////////////
	bool reachableFlag = true;
	//压入第一个edge
	route_ans.push_back(subRoute[0]);
	int prev_edge = subRoute[0];
	for (int i = 1; i < subRoute.size(); i++)
	{
		if (subRoute[i] == prev_edge) //前后两个edge为同一条路
			continue;
		int prevEndNodeId = edges[prev_edge]->endNodeId;
		int currentFromId = edges[subRoute[i]]->startNodeId;
		if (prevEndNodeId == currentFromId) //前后两个edge相邻
		{
			route_ans.push_back(subRoute[i]);
		}
		else //前后两个edge不相邻
		{
			vector<int> SProute;
			double SP_dist = shortestPathLength(prevEndNodeId, currentFromId, SProute);
			if (SP_dist >= 1e10)
			{
				reachableFlag = false;
			}
			else
			{
				for (int sp_idx = 0; sp_idx < SProute.size(); sp_idx++)
				{
					route_ans.push_back(SProute[sp_idx]);
				}
			}
			route_ans.push_back(subRoute[i]); //别忘了把当前路段push进去
		}
		prev_edge = subRoute[i];
	}
	return reachableFlag;
}

int Map::hasEdge(int startNodeId, int endNodeId) const
{
	AdjNode* current = adjList[startNodeId]->next;
	while (current != NULL)
	{
		if (current->endPointId == endNodeId)
		{
			return current->edgeId;
		}
		else
			current = current->next;
	}
	return -1;
}

int Map::hasEdgeWithMinLen(int startNodeId, int endNodeId) const
{
	double minLen = INF;
	AdjNode* current = adjList[startNodeId]->next;
	int result = -1;
	while (current != NULL)
	{
		if (current->endPointId == endNodeId)
		{
			if (edges[current->edgeId]->lengthM <= minLen)
			{
				result = current->edgeId;
				minLen = edges[current->edgeId]->lengthM;
			}
		}
		current = current->next;
	}
	return result;
}

int Map::insertNode(double lat, double lon)
{
	//////////////////////////////////////////////////////////////////////////
	///插入一个新结点(lat, lon),并同时在邻接表中也对应插入一个邻接表结点,返回新结点的id
	//////////////////////////////////////////////////////////////////////////
	//if (!inArea(lat, lon))
	//	return -1;
	GeoPoint* pt = new GeoPoint(lat, lon);
	nodes.push_back(pt);
	AdjNode* adjNode = new AdjNode();
	adjNode->endPointId = adjList.size();
	adjNode->next = NULL;
	adjList.push_back(adjNode);
	return nodes.size() - 1;
}

int Map::insertEdge(Figure* figure, int startNodeId, int endNodeId)
{
	//////////////////////////////////////////////////////////////////////////
	///以figure为路形构造一条新边插入地图,并插入网格索引
	///[注意]不可在没有建立网格索引时调用!
	//////////////////////////////////////////////////////////////////////////
	Edge* newEdge = new Edge();
	newEdge->figure = figure;
	newEdge->startNodeId = startNodeId;
	newEdge->endNodeId = endNodeId;
	newEdge->lengthM = calEdgeLength(figure);
	newEdge->id = edges.size();
	edges.push_back(newEdge);
	AdjNode* current = adjList[startNodeId];
	insertEdge(newEdge->id, startNodeId, endNodeId); //加入连通关系
	createGridIndexForEdge(newEdge); //加入网格索引
	return newEdge->id;
}

int Map::splitEdge(int edgeId, double lat, double lon)
{
	//////////////////////////////////////////////////////////////////////////
	///将edgeId号路在(lat, lon)点切割成两段路,(lat, lon)作为intersection
	///切割保证是安全的,无副作用的
	//////////////////////////////////////////////////////////////////////////
	Edge* edge = edges[edgeId];
	Figure* figure = edge->figure;
	pair<int, int> result;
	bool bidirection = false;
	Edge* edgeR = NULL; //记录双向道时的反向对应路段
	//找到切割点
	Figure* subFigure1 = new Figure(); //记录切割后的前半段路段
	Figure* subFigure2 = new Figure(); //记录切割后的后半段路段
	Figure::iterator iter = figure->begin();
	Figure::iterator nextIter = figure->begin();
	nextIter++;
	int newNodeId;
	while (nextIter != edge->figure->end())
	{
		GeoPoint* pt = (*iter);
		GeoPoint* nextPt = (*nextIter);
		subFigure1->push_back(pt);
		//有投影
		double A = (nextPt->lat - pt->lat);
		double B = -(nextPt->lon - pt->lon);
		double C = pt->lat * (nextPt->lon - pt->lon)
			- pt->lon * (nextPt->lat - pt->lat);
		if (abs(A * lon + B * lat + C) < eps)
		{
			newNodeId = insertNode(lat, lon);
			subFigure1->push_back(nodes[newNodeId]);
			subFigure2->push_back(nodes[newNodeId]);
			//分析该路是否为双向路
			//变量名带R的代表reverse
			AdjNode* currentAdjNode = adjList[edge->endNodeId]->next;
			bidirection = false;
			while (currentAdjNode != NULL && bidirection == false)
			{
				if (currentAdjNode->endPointId == edge->startNodeId)
				{
					edgeR = edges[currentAdjNode->edgeId];
					Figure::iterator iterR = edgeR->figure->begin();
					Figure::iterator nextIterR = edgeR->figure->begin();
					nextIterR++;
					while (nextIterR != edgeR->figure->end())
					{
						GeoPoint* ptR = (*iterR);
						GeoPoint* nextPtR = (*nextIterR);
						if (abs(ptR->lat - nextPt->lat) < eps && abs(ptR->lon - nextPt->lon) < eps
							&& abs(nextPtR->lat - pt->lat) < eps && abs(nextPtR->lon - pt->lon) < eps)
						{
							bidirection = true;
							break;
						}
						iterR++;
						nextIterR++;
					}
				}
				currentAdjNode = currentAdjNode->next;
			}
			iter++;
			break;
		}
		iter++;
		nextIter++;
	}
	if (nextIter == figure->end()) //切割点不在路上则报错退出
	{
		cout << "error: split point is not on the edge" << endl;
		system("pause");
		exit(0);
	}
	//将后半段压入subFigure2
	while (iter != figure->end())
	{
		subFigure2->push_back(*iter);
		iter++;
	}
	//将新边加入,修改连接关系
	//将subFigure2作为新边加入地图
	Edge* edge2 = edges[insertEdge(subFigure2, newNodeId, edge->endNodeId)];
	//将subFigure1替代原来的edge
	delete edge->figure;
	edge->figure = subFigure1;
	edge->lengthM = calEdgeLength(subFigure1);
	edge->endNodeId = newNodeId;
	//修改前半段的连通关系
	AdjNode* current = adjList[edge->startNodeId]->next;
	while (current->edgeId != edge->id)
	{
		current = current->next;
	}
	current->endPointId = newNodeId;
	//处理双向道情况
	if (bidirection)
	{
		Figure* subFigure1R = new Figure();
		Figure* subFigure2R = new Figure();
		for (Figure::iterator iter = subFigure1->begin(); iter != subFigure1->end(); iter++)
		{
			subFigure1R->push_front(*iter);
		}
		for (Figure::iterator iter = subFigure2->begin(); iter != subFigure2->end(); iter++)
		{
			subFigure2R->push_front(*iter);
		}
		//将subFigure2R替代edgeR
		delete edgeR->figure;
		edgeR->figure = subFigure2R;
		edgeR->lengthM = calEdgeLength(subFigure2R);
		//重新创建edge2R
		insertEdge(subFigure2R, edge2->endNodeId, edge2->startNodeId);
	}
	return newNodeId;
}

void Map::delEdge(int edgeId, bool delBirectionEdges /* = true */)
{
	//邻接表中的这条边的邻接节点会被确确实实地删除
	//【注意】可能会发生内存泄露，原edge没有被del掉
	//【注意注意！】TODO：索引没有把路删除，解决办法只是将删掉的路的visited字段改成true临时应付了下而已
	if (edges[edgeId] == NULL)
		return;
	int startNodeId = edges[edgeId]->startNodeId;
	int endNodeId = edges[edgeId]->endNodeId;
	if (edges[edgeId])
		edges[edgeId]->visited = true; //防止被索引找到
	edges[edgeId] = NULL;
	AdjNode* currentAdjNode = adjList[startNodeId];
	while (currentAdjNode->next != NULL)
	{
		if (currentAdjNode->next->endPointId == endNodeId && currentAdjNode->next->edgeId == edgeId) //[注意]同样两个端点可能存在两条不同的edge!!!!
		{
			AdjNode* delNode = currentAdjNode->next;
			currentAdjNode->next = currentAdjNode->next->next;
			delete delNode;
			break;
		}
		currentAdjNode = currentAdjNode->next;
	}

	if (delBirectionEdges) //删除反向边
	{
		int reverseEdgeId = hasEdge(endNodeId, startNodeId);
		if (reverseEdgeId == -1) //没有反向边
			return;
		if (edges[reverseEdgeId])
			edges[reverseEdgeId]->visited = true; //防止被索引找到
		edges[reverseEdgeId] = NULL;
		AdjNode* currentAdjNode = adjList[endNodeId];
		while (currentAdjNode->next != NULL)
		{
			if (currentAdjNode->next->endPointId == startNodeId)
			{
				AdjNode* delNode = currentAdjNode->next;
				currentAdjNode->next = currentAdjNode->next->next;
				delete delNode;
				break;
			}
			currentAdjNode = currentAdjNode->next;
		}
	}
}

//Gdiplus::Color randomColor();

void Map::getMinMaxLatLon(string nodeFilePath)
{
	//////////////////////////////////////////////////////////////////////////
	///[TODO] 这个函数怎么看上去是错的....
	//////////////////////////////////////////////////////////////////////////
	ifstream nodeIfs(nodeFilePath);
	if (!nodeIfs)
	{
		cout << "open " + nodeFilePath + " error!\n";
		system("pause");
		exit(0);
	}
	double minLat = INF;
	double maxLat = -INF;
	double minLon = INF;
	double maxLon = -INF;
	while (nodeIfs)
	{
		double lat, lon;
		int nodeId;
		nodeIfs >> nodeId >> lat >> lon;
		if (nodeIfs.fail())
			break;
		if (lon < area->minLon) area->minLon = lon;
		if (lon > area->maxLon) area->maxLon = lon;
		if (lat < area->minLat) area->minLat = lat;
		if (lat > area->maxLat) area->maxLat = lat;
	}
	printf("minLat:%lf, maxLat:%lf, minLon:%lf, maxLon:%lf\n", area->minLat, area->maxLat, area->minLon, area->maxLon);
	nodeIfs.close();
}

void Map::drawMap(Gdiplus::Color color, MapDrawer& md, bool drawNodeId, bool drawInternalPts /* = false */)
{
	for (int i = 0; i < edges.size(); i++)
	{
		if (edges[i] == NULL)
			continue;
		Figure::iterator ptIter = edges[i]->figure->begin(), nextPtIter = ptIter;
		nextPtIter++;
		//draw edgeId
		int deltaX = 2 * ((double)rand() / (double)RAND_MAX - 0.5) * 5;
		int deltaY = 2 * ((double)rand() / (double)RAND_MAX - 0.5) * 5;

		int deltaX_node = 5;
		int deltaY_node = 5;

		Gdiplus::Color rndColor = MapDrawer::randomColor();
		Gdiplus::Point screenCoord = md.geoToScreen(edges[i]->figure->front()->lat, edges[i]->figure->front()->lon);
		Gdiplus::Point screenCoord_node = md.geoToScreen(edges[i]->figure->front()->lat, edges[i]->figure->front()->lon);
		if (drawNodeId)
			md.drawInt(Gdiplus::Color::Black, screenCoord.X + deltaX_node, screenCoord.Y + deltaY_node, edges[i]->startNodeId);
		//md.drawInt(rndColor, screenCoord.X + deltaX, screenCoord.Y + deltaY, edges[i]->id);
		while (1)
		{
			if (nextPtIter == edges[i]->figure->end())
				break;
			md.drawLine(color, (*ptIter)->lat, (*ptIter)->lon, (*nextPtIter)->lat, (*nextPtIter)->lon);
			//md.drawBoldLine(rndColor, (*ptIter)->lat, (*ptIter)->lon, (*nextPtIter)->lat, (*nextPtIter)->lon);
			if (drawInternalPts)
			{
				md.drawBigPoint(Gdiplus::Color::Black, (*ptIter)->lat, (*ptIter)->lon);
				md.drawBigPoint(Gdiplus::Color::Black, (*nextPtIter)->lat, (*nextPtIter)->lon);
			}
			ptIter++;
			nextPtIter++;
		}
		md.drawBigPoint(Gdiplus::Color::Black, edges[i]->figure->front()->lat, edges[i]->figure->front()->lon);
		md.drawBigPoint(Gdiplus::Color::Black, edges[i]->figure->back()->lat, edges[i]->figure->back()->lon);
	}
}

void Map::drawEdge(Gdiplus::Color color, MapDrawer& md, int edgeId, bool bold /* = true */, bool verbose /* = true */)
{
	if ((edgeId < 0 || edgeId >= edges.size() || edges[edgeId] == NULL))
	{
		if (verbose)
			puts("Warning@drawEdge: edge is NULL");
		return;
	}
	Figure::iterator ptIter = edges[edgeId]->figure->begin(), nextPtIter = ptIter;
	nextPtIter++;
	while (1)
	{
		if (nextPtIter == edges[edgeId]->figure->end())
			break;
		if (bold)
			md.drawBoldLine(color, (*ptIter)->lat, (*ptIter)->lon, (*nextPtIter)->lat, (*nextPtIter)->lon);
		else
			md.drawLine(color, (*ptIter)->lat, (*ptIter)->lon, (*nextPtIter)->lat, (*nextPtIter)->lon);
		//md.drawBigPoint(Gdiplus::Color::Black, (*ptIter)->lat, (*ptIter)->lon);
		//md.drawBigPoint(Gdiplus::Color::Black, (*nextPtIter)->lat, (*nextPtIter)->lon);
		ptIter++;
		nextPtIter++;
	}
}

void Map::drawRoute(Gdiplus::Color color, MapDrawer& md, vector<int> route, bool bold)
{
	for (int i = 0; i < route.size(); i++)
	{
		drawEdge(color, md, route[i], bold);
	}
}

void Map::drawDeletedEdges(Gdiplus::Color color, MapDrawer& md)
{
	for (int i = 0; i < deletedEdges.size(); i++)
	{
		if (deletedEdges[i] == NULL || deletedEdges[i]->figure == NULL)
		{
			cout << "NULL";
			system("pause");
		}

		Figure::iterator ptIter = deletedEdges[i]->figure->begin(), nextPtIter = ptIter;
		nextPtIter++;
		while (1)
		{
			if (nextPtIter == deletedEdges[i]->figure->end())
				break;
			md.drawBoldLine(color, (*ptIter)->lat, (*ptIter)->lon, (*nextPtIter)->lat, (*nextPtIter)->lon);

			/**********************************************************/
			/*test code starts from here*/
			//if (deletedEdges[i]->visited == true)
			//{
			//	md.drawBoldLine(Gdiplus::Color::Aqua, (*ptIter)->lat, (*ptIter)->lon, (*nextPtIter)->lat, (*nextPtIter)->lon);
			//}

			/*test code ends*/
			/**********************************************************/


			//md.drawBigPoint(Gdiplus::Color::Black, (*ptIter)->lat, (*ptIter)->lon);
			//md.drawBigPoint(Gdiplus::Color::Black, (*nextPtIter)->lat, (*nextPtIter)->lon);
			ptIter++;
			nextPtIter++;
		}
	}
}

void Map::deleteEdgesRandomly(int delNum, double minEdgeLengthM)
{
	//////////////////////////////////////////////////////////////////////////
	///随机删除delNum条路，路的长度需超过minEdgeLengthM（单位为m）
	///随机种子需要自己在main函数里写
	//////////////////////////////////////////////////////////////////////////

	int victimId;
	int count = 0;
	while (1)
	{
		if (count == delNum)
			break;
		if (edges[victimId = int(((double)rand()) / RAND_MAX * (edges.size() - 1))] == NULL)
			continue;
		if (edges[victimId]->lengthM < minEdgeLengthM)
			continue;

		deletedEdges.push_back(edges[victimId]);
		cout << "delete " << victimId << endl;
		int reverseVictimId = hasEdge(edges[victimId]->endNodeId, edges[victimId]->startNodeId);
		if (reverseVictimId != -1)
			deletedEdges.push_back(edges[reverseVictimId]);
		delEdge(victimId);
		count++;
	}
	cout << ">> randomly deleted " << delNum << " edges" << endl;
}

void Map::deleteEdgesRandomlyEx(int delNum, double minEdgeLengthM, double aroundThresholdM, int aroundNumThreshold, bool doOutput /* = true */)
{
	int victimId;
	int count = 0;
	while (1)
	{
		if (count >= delNum)
			break;
		if (edges[victimId = int(((double)rand()) / RAND_MAX * (edges.size() - 1))] == NULL)
			continue;
		if (edges[victimId]->lengthM < minEdgeLengthM)
			continue;
		vector<Edge*> nearEdges;
		getNearEdges(edges[victimId], aroundThresholdM, nearEdges);
		if (nearEdges.size() >= aroundNumThreshold)
			continue;
		deletedEdges.push_back(edges[victimId]);
		edges[victimId]->visited = true; //
		cout << count << ": delete " << victimId << endl;
		int reverseVictimId = hasEdge(edges[victimId]->endNodeId, edges[victimId]->startNodeId);
		if (reverseVictimId != -1)
		{
			deletedEdges.push_back(edges[reverseVictimId]);
			edges[reverseVictimId]->visited = true; //
			cout << "delete reverse " << reverseVictimId << endl;
		}
		delEdge(victimId);
		count++;
		//删除所有附近的路
		for each (Edge* nearEdge in nearEdges)
		{
			if (edges[nearEdge->id] == NULL)
				continue;
			if (nearEdge->id == reverseVictimId)
				continue;

			deletedEdges.push_back(nearEdge);
			nearEdge->visited = true; //
			cout << count << ":\tdelete " << nearEdge->id << endl;
			int reverseNearEdgeId = hasEdge(nearEdge->endNodeId, nearEdge->startNodeId);
			if (reverseNearEdgeId != -1 && edges[reverseNearEdgeId] != NULL)
			{
				deletedEdges.push_back(edges[reverseNearEdgeId]);
				edges[reverseNearEdgeId]->visited = true; //
				cout << "\tdelete reverse " << reverseNearEdgeId << endl;
			}
			delEdge(nearEdge->id);
			count++;
			if (count >= delNum)
				break;
		}
	}
	cout << ">> randomly deleted " << count << " edges" << endl;
	if (doOutput)
	{
		ofstream ofs("deletedEdges.txt");
		if (!ofs)
		{
			cout << "open deletedEdges.txt error" << endl;
		}
		cout << deletedEdges.size() << endl;
		//system("pause");
		for (int i = 0; i < deletedEdges.size(); i++)
		{
			//printf("%d: %d\n", i, deletedEdges[i]->id);
			//cout << this->deletedEdges.size() << endl;
			ofs << deletedEdges[i]->id << endl;
		}
		ofs.close();
	}
}

void Map::deleteIntersectionType1(int delNum, double minEdgeLengthM, bool doOutput /* = true */)
{
	//////////////////////////////////////////////////////////////////////////
	///删除类型1的路：即删除一条路，共要删除delNum条
	///其中删除路的长度需要满足超过minEdgeLengthM
	///删除过程中挑选周围比较空旷的路删
	//////////////////////////////////////////////////////////////////////////
	cout << "start deleting edges" << endl;
	int victimId;
	int count = 0;
	while (1)
	{
		if (count >= delNum)
			break;
		if (edges[victimId = int(((double)rand()) / RAND_MAX * (edges.size() - 1))] == NULL)
			continue;
		if (edges[victimId]->lengthM < minEdgeLengthM)
			continue;
		if (edges[victimId]->startNodeId == edges[victimId]->endNodeId)
			continue;


		//检测这条候选路段是不是周围比较空旷
		vector<Edge*> nearEdges;
		getNearEdges(edges[victimId], 50.0, nearEdges);
		double minNearEdgeDistM = 99999.0;
		bool hasNearedgeAlreayDeleted = false;
		for each (Edge* nearEdge in nearEdges)
		{
			//cal mindist <nearEdge, victimEdge>
			double dist = distM(edges[victimId], nearEdge);
			if (dist > eps && dist < minNearEdgeDistM)
			{
				minNearEdgeDistM = dist;
			}
			if (nearEdge->visitedExtern == true && dist <= 1) //邻接边已经被删则跳过
			{
				//system("pause");
				hasNearedgeAlreayDeleted = true;
				break;
			}
		}
		if (hasNearedgeAlreayDeleted || minNearEdgeDistM < 50.0) //如果附近50m内有其他路则不考虑这个路段
			continue;

		deletedEdges.push_back(edges[victimId]);
		//edges[victimId]->visited = true; //
		edges[victimId]->visitedExtern = true;
		cout << count + 1 << ": delete " << victimId << endl;
		int reverseVictimId = hasEdge(edges[victimId]->endNodeId, edges[victimId]->startNodeId);
		if (reverseVictimId != -1 && edges[reverseVictimId] != NULL)
		{
			deletedEdges.push_back(edges[reverseVictimId]);
			//edges[reverseVictimId]->visited = true; //
			edges[reverseVictimId]->visitedExtern = true;
			cout << "delete reverse " << reverseVictimId << endl;
		}
		//delEdge(victimId);
		count++;
	}

	for (int i = 0; i < deletedEdges.size(); i++)
	{
		delEdge(deletedEdges[i]->id);
	}

	cout << ">> randomly deleted " << count << " edges" << endl;
	if (doOutput)
	{
		ofstream ofs("deletedEdgesType1.txt");
		if (!ofs)
		{
			cout << "open deletedEdgesType1.txt error" << endl;
		}
		cout << deletedEdges.size() << endl;
		for (int i = 0; i < deletedEdges.size(); i++)
		{
			//printf("%d: %d\n", i, deletedEdges[i]->id);
			//cout << this->deletedEdges.size() << endl;
			ofs << deletedEdges[i]->id << endl;
		}
		ofs.close();
	}
}

void Map::deleteIntersectionType2(int delNum, double minEdgeLengthM, bool doOutput /* = true */)
{
	//////////////////////////////////////////////////////////////////////////
	///删除类型2的路：即十字路口包含的四条路（双向为8条），共要删除delNum个路口
	///其中删除路的长度需要满足超过minEdgeLengthM
	///[TODO]:路口没删
	//////////////////////////////////////////////////////////////////////////

	int victimNodeId;
	int count = 0;
	while (1)
	{
		if (count >= delNum)
			break;
		if (nodes[victimNodeId = int(((double)rand()) / RAND_MAX * (nodes.size() - 1))] == NULL)
			continue;

		//遍历所有出度
		AdjNode* currentAdjNode = adjList[victimNodeId]->next;
		int connectCount = 0;
		while (currentAdjNode != NULL)
		{
			connectCount++;
			currentAdjNode = currentAdjNode->next;
		}
		if (connectCount < 4) //不是十字路口
			continue;

		//检测所有连接的路长度都要足够长
		currentAdjNode = adjList[victimNodeId]->next;
		bool satisfyTheLengthThres = true;
		while (currentAdjNode != NULL)
		{
			if (edges[currentAdjNode->edgeId]->lengthM < minEdgeLengthM)
			{
				satisfyTheLengthThres = false;
				break;
			}
			currentAdjNode = currentAdjNode->next;
		}
		if (!satisfyTheLengthThres)
			continue;

		//删路
		currentAdjNode = adjList[victimNodeId]->next;
		while (currentAdjNode != NULL)
		{
			int victimEdgeId = currentAdjNode->edgeId;
			deletedEdges.push_back(edges[victimEdgeId]);
			edges[victimEdgeId]->visited = true; //
			cout << "delete " << victimEdgeId << endl;
			int reverseVictimId = hasEdge(edges[victimEdgeId]->endNodeId, edges[victimEdgeId]->startNodeId);
			if (reverseVictimId != -1)
			{
				deletedEdges.push_back(edges[reverseVictimId]);
				edges[reverseVictimId]->visited = true; //
				cout << "delete reverse " << reverseVictimId << endl;
			}
			delEdge(victimEdgeId);
			currentAdjNode = currentAdjNode->next;
		}
		count++;
	}
	cout << ">> randomly deleted " << count << " edges" << endl;
	if (doOutput)
	{
		ofstream ofs("deletedEdgesType2.txt");
		if (!ofs)
		{
			cout << "open deletedEdgesType2.txt error" << endl;
		}
		cout << deletedEdges.size() << endl;
		for (int i = 0; i < deletedEdges.size(); i++)
		{
			//printf("%d: %d\n", i, deletedEdges[i]->id);
			//cout << this->deletedEdges.size() << endl;
			ofs << deletedEdges[i]->id << endl;
		}
		ofs.close();
	}
}

void Map::deleteIntersectionType3(int delNum, double minEdgeLengthM, bool doOutput /* = true */)
{
	//////////////////////////////////////////////////////////////////////////
	///删除类型3的路：即丁字路口包含的3条路（双向为6条），共要删除delNum个路口
	///其中删除路的长度需要满足超过minEdgeLengthM
	///[TODO]:路口没删
	//////////////////////////////////////////////////////////////////////////

	int victimNodeId;
	int count = 0;
	while (1)
	{
		if (count >= delNum)
			break;
		if (nodes[victimNodeId = int(((double)rand()) / RAND_MAX * (nodes.size() - 1))] == NULL)
			continue;

		//遍历所有出度
		AdjNode* currentAdjNode = adjList[victimNodeId]->next;
		int connectCount = 0;
		while (currentAdjNode != NULL)
		{
			connectCount++;
			currentAdjNode = currentAdjNode->next;
		}
		if (connectCount != 3) //不是丁字路口
			continue;

		//检测所有连接的路长度都要足够长
		currentAdjNode = adjList[victimNodeId]->next;
		bool satisfyTheLengthThres = true;
		while (currentAdjNode != NULL)
		{
			if (edges[currentAdjNode->edgeId]->lengthM < minEdgeLengthM)
			{
				satisfyTheLengthThres = false;
				break;
			}
			currentAdjNode = currentAdjNode->next;
		}
		if (!satisfyTheLengthThres)
			continue;

		//检测每条候选路段是不是周围比较空旷
		currentAdjNode = adjList[victimNodeId]->next;
		bool satisfyTheSparse = true;
		while (currentAdjNode != NULL)
		{
			vector<Edge*> nearEdges;
			getNearEdges(edges[currentAdjNode->edgeId], 150.0, nearEdges);
			double minNearEdgeDistM = 99999.0;
			bool hasNearedgeAlreayDeleted = false;
			for each (Edge* nearEdge in nearEdges)
			{
				//cal mindist <nearEdge, victimEdge>
				double dist = distM(edges[currentAdjNode->edgeId], nearEdge);
				if (dist > eps && dist < minNearEdgeDistM)
				{
					minNearEdgeDistM = dist;
				}
				if (nearEdge->visited == true && dist <= eps)
				{
					hasNearedgeAlreayDeleted = true;
					break;
				}
			}
			if (hasNearedgeAlreayDeleted || minNearEdgeDistM < 150.0) //如果附近30m内有其他路则不考虑这个路口
			{
				satisfyTheSparse = false;
				break;
			}
			currentAdjNode = currentAdjNode->next;
		}
		if (!satisfyTheSparse)
			continue;

		//删路
		currentAdjNode = adjList[victimNodeId]->next;
		while (currentAdjNode != NULL)
		{
			int victimEdgeId = currentAdjNode->edgeId;
			deletedEdges.push_back(edges[victimEdgeId]);
			edges[victimEdgeId]->visited = true; //
			cout << "delete " << victimEdgeId << endl;
			int reverseVictimId = hasEdge(edges[victimEdgeId]->endNodeId, edges[victimEdgeId]->startNodeId);
			if (reverseVictimId != -1)
			{
				deletedEdges.push_back(edges[reverseVictimId]);
				edges[reverseVictimId]->visited = true; //
				cout << "delete reverse " << reverseVictimId << endl;
			}
			delEdge(victimEdgeId);
			currentAdjNode = currentAdjNode->next;
		}
		count++;
	}
	cout << ">> randomly deleted " << count << " edges" << endl;
	if (doOutput)
	{
		ofstream ofs("deletedEdgesType3.txt");
		if (!ofs)
		{
			cout << "open deletedEdgesType3.txt error" << endl;
		}
		cout << deletedEdges.size() << endl;
		for (int i = 0; i < deletedEdges.size(); i++)
		{
			//printf("%d: %d\n", i, deletedEdges[i]->id);
			//cout << this->deletedEdges.size() << endl;
			ofs << deletedEdges[i]->id << endl;
		}
		ofs.close();
	}
}

void Map::deleteEdges(string path)
{
	ifstream ifs(path);
	if (!ifs)
	{
		cout << "open file " << path << " error! in func Map::deleteEdges(string path)" << endl;
	}
	int edgeId;
	vector<int> deletedEdgesId;
	while (ifs)
	{
		ifs >> edgeId;
		if (ifs.fail())
			break;
		deletedEdgesId.push_back(edgeId);
	}
	for (int i = 0; i < deletedEdgesId.size(); i++)
	{
		int victimId = deletedEdgesId[i];
		deletedEdges.push_back(edges[victimId]);
	}
	for (int i = 0; i < deletedEdgesId.size(); i++)
	{
		delEdge(deletedEdgesId[i]);
	}
	ifs.close();
}

void Map::loadPolylines(string filePath)
{
	//////////////////////////////////////////////////////////////////////////
	///读取预先生成好的polyline，存入对应的edge的r_hat成员中
	//////////////////////////////////////////////////////////////////////////
	ifstream ifs(filePath);
	if (!ifs)
	{
		cout << "open file error!" << endl;
		system("pause");
	}
	while (!ifs.eof())
	{
		int rId, count;
		double lat, lon;
		ifs >> rId >> count;
		edges[rId]->r_hat.clear();
		for (int i = 0; i < count; i++)
		{
			ifs >> lat >> lon;
			GeoPoint* tempPt = new GeoPoint(lat, lon, true);
			edges[rId]->r_hat.push_back(tempPt);
		}
	}
	cout << "polyline 读取完毕" << endl;
}

void Map::roadtypeSummary(vector<Traj*>& trajs_vec)
{
	for (auto& traj : trajs_vec)
	{
		for (auto& pt : *traj)
		{
			if (pt->mmRoadId < 0)
				continue;
			else
				edges[pt->mmRoadId]->trainData.push_back(pt);
		}
	}
	vector<int> roadTypePtsCounts(WAY_TYPE.size(), 0);
	vector<int> roadTypeEdgeCounts(WAY_TYPE.size(), 0);
	for (auto& edge : edges)
	{
		if (edge == NULL)
			continue;
		roadTypePtsCounts[edge->type] += edge->trainData.size();
		roadTypeEdgeCounts[edge->type] ++;
	}
	int totPtsCount = 0;
	int totEdgeCount = 0;
	for (int i = 0; i < roadTypePtsCounts.size(); i++)
	{
		totPtsCount += roadTypePtsCounts[i];
		totEdgeCount += roadTypeEdgeCounts[i];
	}
	printf("%d edges in total\n", totEdgeCount);
	for (int i = 0; i < roadTypePtsCounts.size(); i++)
	{
		printf("%s -- edge count %d (%.2f%%), pts count : %d (%.2f%%)\n", WAY_TYPE[i].c_str(), roadTypeEdgeCounts[i], (double)roadTypeEdgeCounts[i] / totEdgeCount * 100.0,
			roadTypePtsCounts[i], (double)roadTypePtsCounts[i] / totPtsCount * 100.0);
	}
}

void Map::roadtypeSummary(vector<Route>& routes_vec)
{
	vector<int> visitCount(edges.size(), 0);
	for (auto& route : routes_vec)
	{
		for (auto& edgeid : route)
		{
			if (edgeid < 0)
			{
				cout << "error: edgeid < 0" << endl;
				system("pause");
			}
			else
				visitCount[edgeid]++;
		}
	}
	vector<int> roadTypeVisitCounts(WAY_TYPE.size(), 0);
	vector<int> roadTypeEdgeCounts(WAY_TYPE.size(), 0);
	for (auto& edge : edges)
	{
		if (edge == NULL)
			continue;
		roadTypeVisitCounts[edge->type] += visitCount[edge->id];
		roadTypeEdgeCounts[edge->type] ++;
	}
	int totVisitCount = 0;
	int totEdgeCount = 0;
	for (int i = 0; i < roadTypeVisitCounts.size(); i++)
	{
		totVisitCount += roadTypeVisitCounts[i];
		totEdgeCount += roadTypeEdgeCounts[i];
	}
	printf("%d edges in total\n", totEdgeCount);
	for (int i = 0; i < roadTypeVisitCounts.size(); i++)
	{
		printf("%s -- edge count %d (%.2f%%), visit count : %d (%.2f%%)\n", WAY_TYPE[i].c_str(), roadTypeEdgeCounts[i], (double)roadTypeEdgeCounts[i] / totEdgeCount * 100.0,
			roadTypeVisitCounts[i], (double)roadTypeVisitCounts[i] / totVisitCount * 100.0);
	}
}
double Map::turningAngle(int e1, int e2)
{
	///返回e1与e2这条路段之间的转向角，范围为[0,360)
	///不考虑道路形状
	if (edges[e1]->endNodeId != edges[e2]->startNodeId)
	{
		cout << "Error: e1与e2需为邻接的路段" << endl;
		system("pause");
	}
	return Map::turningAngle(nodes[edges[e1]->startNodeId], nodes[edges[e1]->endNodeId], nodes[edges[e2]->endNodeId]);
}

void Map::estimateSpd(vector<Traj*>& trajs_vec)
{
	for (auto& traj : trajs_vec)
	{
		Traj::iterator iter = traj->begin(); iter++;
		Traj::iterator preIter = traj->begin();
		(*preIter)->speed = GeoPoint::speedMps(*iter, *preIter);
		for (; iter != traj->end(); iter++, preIter++)
		{
			(*iter)->speed = GeoPoint::speedMps(*iter, *preIter);
		}
	}
}

void Map::get_avg_spd_of_roads(vector<Traj*>& trajs_vec, double defaultSpd_mps /* = 5.0*/)
{
	estimateSpd(trajs_vec);
	vector<double> spd(edges.size(), 0.0);
	vector<int> count(edges.size(), 0);
	for (auto& traj : trajs_vec)
	{
		for (auto& pt : *traj)
		{
			if (pt->mmRoadId < 0)
				continue;
			else
			{
				spd[pt->mmRoadId] += pt->speed;
				count[pt->mmRoadId] ++;
			}
		}
	}
	ofstream ofs("spd.csv");
	for (auto& edge : edges)
	{
		if (edge == NULL)
			continue;
		if (count[edge->id] < 5)
			edge->avgSpd_mps = defaultSpd_mps;
		else
			edge->avgSpd_mps = (spd[edge->id] / count[edge->id]);

		ofs << edge->avgSpd_mps << endl;
	}
	ofs.close();
}

void Map::check_edge_visited(vector<Route>& routes_vec, bool dumpfile /* = false*/, string filePath /* = "unvisited_edges.csv"*/)
{
	for (auto& route : routes_vec)
	{
		for (auto& edgeId : route)
		{
			if (edges[edgeId] == NULL)
			{
				printf("edge %d is NULL\n", edgeId);
				system("pause");
			}
			edges[edgeId]->visitedExtern = true;
		}
	}
	if (dumpfile)
	{
		dump_unvisitied_edges(filePath);
	}
}

void Map::dump_unvisitied_edges(string filePath)
{
	ofstream ofs(filePath);
	if (!ofs)
	{
		cout << "create file " << filePath << " error!" << endl;
		return;
	}
	for (auto& edge : edges)
	{
		if (edge == NULL)
			continue;
		if (!edge->visitedExtern)
			ofs << edge->id << endl;
	}
	cout << "unvisited edges have been output to " << filePath << endl;
	ofs.close();
}
//////////////////////////////////////////////////////////////////////////
///private part
//////////////////////////////////////////////////////////////////////////
double Map::distM_withThres(double lat, double lon, Edge* edge, double threshold) const
{
	//////////////////////////////////////////////////////////////////////////
	///返回点(lat, lon)到边edge的距离上界 【注意】不可用于计算精确距离！
	///距离定义为：min(点到可投影边的投影距离，点到所有形状点的欧氏距离)
	///如果更新上界时发现已经低于threshold(单位米)则直接返回
	//////////////////////////////////////////////////////////////////////////
	double minDist = INF;
	if (edge == NULL)
	{
		cout << "edge = NULL";
		system("pause");
	}
	if (edge->figure == NULL)
	{
		cout << "edge->figure = NULL";
		system("pause");
	}
	//遍历端点距离
	for (Figure::iterator iter = edge->figure->begin(); iter != edge->figure->end(); iter++)
	{
		if (*iter == NULL)
		{
			cout << "*iter = NULL";
			system("pause");
		}
		/*if (!inArea(lat, lon))// || !inArea((*iter)->lat, (*iter)->lon))
		{
		cout << "not in area";
		printf("(lat,lon) = (%lf,%lf), iter = (%lf, %lf)\n", lat, lon, (*iter)->lat, (*iter)->lon);
		printf("minlat = %lf, maxlat = %lf\n", minLat, maxLat);
		printf("minlon = %lf, maxlon = %lf\n", minLon, maxLon);
		system("pause");
		}*/
		double tmpDist = GeoPoint::distM(lat, lon, (*iter)->lat, (*iter)->lon);
		if (tmpDist < threshold)
			return tmpDist;
		if (tmpDist < minDist)
			minDist = tmpDist;
	}
	//遍历投影距离
	Figure::iterator iter = edge->figure->begin();
	Figure::iterator nextIter = edge->figure->begin();
	nextIter++;
	while (nextIter != edge->figure->end())
	{
		//有投影
		GeoPoint pt(lat, lon);
		if (cosAngle(&pt, (*iter), (*nextIter)) <= 0 && cosAngle(&pt, (*nextIter), (*iter)) <= 0)
		{
			double A = ((*nextIter)->lat - (*iter)->lat);
			double B = -((*nextIter)->lon - (*iter)->lon);
			double C = (*iter)->lat * ((*nextIter)->lon - (*iter)->lon)
				- (*iter)->lon * ((*nextIter)->lat - (*iter)->lat);
			double tmpDist = abs(A * pt.lon + B * pt.lat + C) / sqrt(A * A + B * B);
			tmpDist *= GeoPoint::geoScale;
			if (tmpDist < threshold)
				return tmpDist;
			if (minDist > tmpDist)
				minDist = tmpDist;
		}
		iter++;
		nextIter++;
	}
	return minDist;
}

double Map::calEdgeLength(Figure* figure) const
{
	//////////////////////////////////////////////////////////////////////////
	///计算路段的长度，单位为m
	//////////////////////////////////////////////////////////////////////////
	double lengthM = 0;
	Figure::iterator ptIter = figure->begin(), nextPtIter = ptIter;
	nextPtIter++;
	while (1)
	{
		if (nextPtIter == figure->end())
			break;
		lengthM += GeoPoint::distM((*ptIter)->lat, (*ptIter)->lon, (*nextPtIter)->lat, (*nextPtIter)->lon);
		ptIter++;
		nextPtIter++;
	}
	return lengthM;
}

bool Map::inArea(double lat, double lon, bool use_geo) const
{
	return area->inArea(lat, lon, use_geo);
}

void Map::test()
{
	int* flag = new int[nodes.size()];
	for (int i = 0; i < nodes.size(); i++)
	{
		flag[i] = -1;
	}
	for (int i = 0; i < adjList.size(); i++)
	{
		if (i % 1000 == 0)
		{
			cout << i << endl;
		}
		AdjNode* current = adjList[i]->next;
		while (current != NULL)
		{
			int j = current->endPointId;
			/*int edgeIJId = current->edgeId;
			int edgeJIId = hasEdge(j, i);
			if (edgeJIId != -1)
			{
			Edge* edgeIJ = edges[edgeIJId];
			Edge* edgeJI = edges[edgeJIId];
			if (edgeIJ->size() != edgeJI->size())
			{
			cout << "XXXXXX" << endl;
			}
			Edge::iterator iter1 = edgeIJ->begin();
			Edge::iterator iter2 = edgeJI->end();
			iter2--;
			while (iter2 != edgeJI->begin())
			{
			if (abs((*iter1)->lat - (*iter2)->lat) > 1e-8 ||
			abs((*iter1)->lon - (*iter2)->lon) > 1e-8)
			{
			cout << "YYYYYY" << endl;
			printf("%lf,%lf,%lf,%lf\n", (*iter1)->lat, (*iter2)->lat, (*iter1)->lon, (*iter2)->lon);
			printf("edgeIJ = %d, edgeJI = %d", edgeIJId, edgeJIId);
			system("pause");
			}
			//printf("%lf,%lf,%lf,%lf\n", (*iter1)->lat, (*iter2)->lat, (*iter1)->lon, (*iter2)->lon);
			//system("pause");
			iter2--;
			iter1++;
			}
			}*/
			if (flag[j] == i)
			{
				cout << "ZZZZZZZ" << endl;
				printf("%d, %d\n", i, j);
				system("pause");
			}
			flag[j] = i;
			current = current->next;
		}
	}
}

void Map::createGridIndex()
{
	//////////////////////////////////////////////////////////////////////////
	///对全图建立网格索引
	///包括对边和点进行网格索引
	//////////////////////////////////////////////////////////////////////////
	//initialization
	if (gridWidth <= 0)
		return;
	gridHeight = int((area->maxLat - area->minLat) / (area->maxLon - area->minLon) * double(gridWidth)) + 1;
	gridSizeDeg = (area->maxLon - area->minLon) / double(gridWidth);
	grid = new list<Edge*>* *[gridHeight];
	for (int i = 0; i < gridHeight; i++)
		grid[i] = new list<Edge*>*[gridWidth];
	for (int i = 0; i < gridHeight; i++)
	{
		for (int j = 0; j < gridWidth; j++)
		{
			grid[i][j] = new list<Edge*>();
		}
	}
	printf("Map index gridWidth = %d, gridHeight = %d\n", gridWidth, gridHeight);
	cout << "gridSize = " << gridSizeDeg * GeoPoint::geoScale << "m" << endl;
	for (vector<Edge*>::iterator edgeIter = edges.begin(); edgeIter != edges.end(); edgeIter++)
	{
		createGridIndexForEdge((*edgeIter));
	}
	ptIndex.createIndex(nodesInArea, area, gridWidth);
}

void Map::insertEdgeIntoGrid(Edge* edge, int row, int col)
{
	//////////////////////////////////////////////////////////////////////////
	///将路段edge加入grid[row][col]中索引，如果已经加入过则不添加
	///改函数一定在对某条edge建立索引时调用,所以加入过的grid中最后一个一定是edge
	//////////////////////////////////////////////////////////////////////////
	if (row >= gridHeight || row < 0 || col >= gridWidth || col < 0)
		return;
	if (grid[row][col]->size() > 0 && grid[row][col]->back() == edge)
		return;
	else
		grid[row][col]->push_back(edge);
}

void Map::createGridIndexForSegment(Edge *edge, GeoPoint* fromPT, GeoPoint* toPt)
{
	//////////////////////////////////////////////////////////////////////////
	///对edge路中的fromPt->toPt段插入网格索引，经过的网格都加入其指针，如果与网格相交长度过小则不加入网格
	///如果2个端点都在当前区域外则不加入网格，如果有一个端点在当前区域外，则加入在区域内的那个端点所在的网格
	//////////////////////////////////////////////////////////////////////////
	if (edge == NULL)
		return;
	//都不在区域内则跳过
	if (!inArea(fromPT->lat, fromPT->lon, false) && !inArea(toPt->lat, toPt->lon, false))
		return;
	//以个点在区域内则只加另外一个端点所在的网格
	//TODO:这样加索引并不准确
	if (!inArea(fromPT->lat, fromPT->lon, false))
	{
		int row = (toPt->lat - area->minLat) / gridSizeDeg;
		int col = (toPt->lon - area->minLon) / gridSizeDeg;
		insertEdgeIntoGrid(edge, row, col);
		return;
	}
	if (!inArea(toPt->lat, toPt->lon, false))
	{
		int row = (fromPT->lat - area->minLat) / gridSizeDeg;
		int col = (fromPT->lon - area->minLon) / gridSizeDeg;
		insertEdgeIntoGrid(edge, row, col);
		return;
	}


	bool crossRow;
	GeoPoint* pt1 = fromPT;
	GeoPoint* pt2 = toPt;
	double x1 = pt1->lon - area->minLon;
	double y1 = pt1->lat - area->minLat;
	double x2 = pt2->lon - area->minLon;
	double y2 = pt2->lat - area->minLat;
	int row1 = y1 / gridSizeDeg;
	int row2 = y2 / gridSizeDeg;
	int col1 = x1 / gridSizeDeg;
	int col2 = x2 / gridSizeDeg;
	if (row1 >= gridHeight || row1 < 0 || col1 >= gridWidth || col1 < 0 ||
		row2 >= gridHeight || row2 < 0 || col2 >= gridWidth || col2 < 0)
	{
		cout << "************test**************" << endl;
		cout << "row1 = " << row1 << " col1 = " << col1 << endl;
		cout << "row2 = " << row2 << " col2 = " << col2 << endl;
		printf("pt1(%.8lf,%.8lf)\n", pt1->lat, pt1->lon);
		printf("pt2(%.8lf,%.8lf)\n", pt2->lat, pt2->lon);
		printf("min(%.8lf,%.8lf), max(%.8lf,%.8lf)\n", area->minLat, area->minLon, area->maxLat, area->maxLon);
		cout << "edgeId = " << edge->id << endl;
		cout << "from node " << edge->startNodeId << " to node " << edge->endNodeId << endl;
		cout << "inarea = " << inArea(pt2->lat, pt2->lon, false) << endl;
		cout << "maxRow = " << gridHeight - 1 << " maxCol = " << gridWidth - 1 << endl;
		system("pause");
		//TODO：这一坨没有仔细想过能不能这么写
		/*if (row1 >= gridHeight)	row1 = gridHeight;
		if (row2 >= gridHeight)	row2 = gridHeight;
		if (col1 >= gridWidth)	col1 = gridWidth;
		if (col2 >= gridWidth)	col2 = gridWidth;
		if (row1 < 0)	row1 = 0;
		if (row2 < 0)	row2 = 0;
		if (col1 < 0)	col1 = 0;
		if (col2 < 0)	col2 = 0;*/
	}
	double A = y2 - y1;
	double B = -(x2 - x1);
	double C = -B * y1 - A * x1;
	int i, j;
	//pt1,pt2都在一个cell中
	if (row1 == row2 && col1 == col2)
	{
		insertEdgeIntoGrid(edge, row1, col1);
		return;
	}
	//只穿越横向格子
	if (row1 == row2)
	{
		//头
		double headDist = ((min(col1, col2) + 1) * gridSizeDeg - min(x1, x2)) / gridSizeDeg;
		if (headDist / gridSizeDeg > strictThreshold)
			insertEdgeIntoGrid(edge, row1, min(col1, col2));
		//中间
		for (i = min(col1, col2) + 1; i < max(col1, col2); i++)
		{
			insertEdgeIntoGrid(edge, row1, i);
		}
		//尾
		double tailDist = (max(x1, x2) - max(col1, col2) * gridSizeDeg) / gridSizeDeg;
		if (tailDist / gridSizeDeg > strictThreshold)
			insertEdgeIntoGrid(edge, row1, max(col1, col2));
		return;
	}
	//只穿越纵向格子
	if (col1 == col2)
	{
		//头
		double headDist = ((min(row1, row2) + 1) * gridSizeDeg - min(y1, y2)) / gridSizeDeg;
		if (headDist / gridSizeDeg > strictThreshold)
			insertEdgeIntoGrid(edge, min(row1, row2), col1);
		//中间
		for (i = min(row1, row2) + 1; i < max(row1, row2); i++)
		{
			insertEdgeIntoGrid(edge, i, col1);
		}
		//尾
		double tailDist = (max(y1, y2) - max(row1, row2) * gridSizeDeg) / gridSizeDeg;
		if (tailDist / gridSizeDeg > strictThreshold)
			insertEdgeIntoGrid(edge, max(row1, row2), col1);
		return;
	}
	//斜向穿越
	simplePoint pts[1000];
	int n_pts = 0;
	for (i = min(row1, row2) + 1; i <= max(row1, row2); i++)
	{
		pts[n_pts++] = std::make_pair((-C - B*i*gridSizeDeg) / A, i*gridSizeDeg);
	}
	for (i = min(col1, col2) + 1; i <= max(col1, col2); i++)
	{
		pts[n_pts++] = std::make_pair(i*gridSizeDeg, (-C - A*i*gridSizeDeg) / B);
	}
	std::sort(pts, pts + n_pts, smallerInX);

	GeoPoint* leftPt, *rightPt;
	if (x1 < x2)
	{
		leftPt = pt1;
		rightPt = pt2;
	}
	else
	{
		leftPt = pt2;
		rightPt = pt1;
	}
	double xL = leftPt->lon - area->minLon;
	double xR = rightPt->lon - area->minLon;
	double yL = leftPt->lat - area->minLat;
	double yR = rightPt->lat - area->minLat;

	//头
	double headDist = sqrt((xL - pts[0].first)*(xL - pts[0].first) + (yL - pts[0].second)*(yL - pts[0].second));
	if (headDist / gridSizeDeg > strictThreshold)
	{
		insertEdgeIntoGrid(edge, (int)(yL / gridSizeDeg), (int)(xL / gridSizeDeg));
	}
	//中间
	for (i = 0; i < n_pts - 1; i++)
	{
		double dist = sqrt((pts[i].first - pts[i + 1].first)*(pts[i].first - pts[i + 1].first) + (pts[i].second - pts[i + 1].second)*(pts[i].second - pts[i + 1].second));
		if (dist / gridSizeDeg > strictThreshold)
			//insertEdgeIntoGrid(edge, getRowId(pts[i], pts[i + 1]), getColId(pts[i], pts[i + 1]));
		{
			//加1e-9是为了解决double的精度误差,比如原本row应该是13的,因为精度误差而算成12.99999999,取整后变成12
			int pts_i_row = (int)(pts[i].second / gridSizeDeg + 1e-9);
			int pts_i_col = (int)(pts[i].first / gridSizeDeg + 1e-9);
			int pts_i_plus_1_row = (int)(pts[i + 1].second / gridSizeDeg + 1e-9);
			int pts_i_plus_1_col = (int)(pts[i + 1].first / gridSizeDeg + 1e-9);
			int row = min(pts_i_row, pts_i_plus_1_row);
			int col = min(pts_i_col, pts_i_plus_1_col);
			insertEdgeIntoGrid(edge, row, col);
		}
	}
	//尾
	double tailDist = sqrt((xR - pts[n_pts - 1].first)*(xR - pts[n_pts - 1].first) + (yR - pts[n_pts - 1].second)*(yR - pts[n_pts - 1].second));
	if (tailDist / gridSizeDeg > strictThreshold)
	{
		insertEdgeIntoGrid(edge, (int)(yR / gridSizeDeg), (int)(xR / gridSizeDeg));
	}
	return;
}

void Map::createGridIndexForEdge(Edge *edge)
{
	if (edge == NULL)
		return;
	Figure::iterator ptIter = edge->figure->begin();
	Figure::iterator nextPtIter = edge->figure->begin(); nextPtIter++;
	while (nextPtIter != edge->figure->end())
	{
		createGridIndexForSegment(edge, *ptIter, *nextPtIter);
		ptIter++;
		nextPtIter++;
	}
}

void Map::insertEdge(int edgeId, int startNodeId, int endNodeId)
{
	//////////////////////////////////////////////////////////////////////////
	///向邻接表adjList中插入一条边的连通关系，初次构建图时使用，私有版本，不允许外部调用
	///[TODO]:可能会有问题@Line1487的while
	//////////////////////////////////////////////////////////////////////////
	/*if (startNodeId == -1)
	{
	system("pause");
	}
	if (startNodeId >= adjList.size())
	{
	cout << "startNodeId: " << startNodeId;
	cout << " adjList: " << adjList.size() << endl;
	}*/
	AdjNode* current = adjList[startNodeId];

	if (current == NULL)
	{
		cout << "[异常] current = NULL！" << endl;
		system("pause");
	}
	if (startNodeId >= adjList.size())
	{
		cout << "[异常] 索引越界 in Map::insertEdge(int edgeId, int startNodeId, int endNodeId)" << endl;
		printf("edgeId = %d, startNodeId = %d, endNodeId = %d, tot node count = %d\n", edgeId, startNodeId, endNodeId, nodes.size());
		system("pause");
	}


	while (current->next != NULL)
	{
		current = current->next;
	}
	AdjNode* tmpAdjNode = new AdjNode();
	tmpAdjNode->endPointId = endNodeId;
	tmpAdjNode->edgeId = edgeId;
	tmpAdjNode->next = NULL;
	current->next = tmpAdjNode;
	edges[edgeId]->startNodeId = startNodeId;
	edges[edgeId]->endNodeId = endNodeId;
}

int Map::getRowId(double lat) const
{
	return (int)((lat - area->minLat) / gridSizeDeg);
}

int Map::getColId(double lon) const
{
	return (int)((lon - area->minLon) / gridSizeDeg);
}

double Map::cosAngle(GeoPoint* pt1, GeoPoint* pt2, GeoPoint* pt3)
{
	//////////////////////////////////////////////////////////////////////////
	///cos<p1->p2, p2->p3>
	//////////////////////////////////////////////////////////////////////////	
	double v1x = pt2->lon - pt1->lon;
	double v1y = pt2->lat - pt1->lat;
	double v2x = pt3->lon - pt2->lon;
	double v2y = pt3->lat - pt2->lat;
	return (v1x * v2x + v1y * v2y) / sqrt((v1x * v1x + v1y * v1y)*(v2x * v2x + v2y * v2y));
}

double Map::turningAngle(GeoPoint* p1, GeoPoint* p2, GeoPoint* p3)
{
	///返回p1->p2与p2->p3这两根有向线段之间的转向角，范围为[0,360)
	double PI = 3.14159265358979323846;
	double tiny = 0.01;
	if (GeoPoint::distM(p1, p2) < tiny || GeoPoint::distM(p2, p3) < tiny)
	{
		cout << "warning: dist < tiny, 自动返回0度作为turning" << endl;
		return 0.0;
	}		
	double cos_e1 = (p2->lon - p1->lon) / GeoPoint::distM(p1, p2);
	double cos_e2 = (p3->lon - p2->lon) / GeoPoint::distM(p2, p3);
	double rad_e1 = acos(cos_e1);
	double rad_e2 = acos(cos_e2);
	if (p2->lat < p1->lat) // [pi, 2pi]
		rad_e1 = 2 * PI - rad_e1;
	if (p3->lat < p2->lat) // [pi, 2pi]
		rad_e2 = 2 * PI - rad_e2;
	double angle_e1 = rad_e1 / PI * 180.0;
	double angle_e2 = rad_e2 / PI * 180.0;
	printf("angle1: %.2f, angle2: %.2f\n", angle_e1, angle_e2);
	double angle = angle_e2 - angle_e1;
	if (angle < 0)
		angle += 360.0;
	return angle;
}
void Map::split(const string& src, const string& separator, vector<string>& dest)
{
	//////////////////////////////////////////////////////////////////////////
	///这个版本可能有问题，好像如果相邻两个分隔符之间没有任何东西，不会将空串作为字串的
	//////////////////////////////////////////////////////////////////////////
	std::string str = src;
	std::string substring;
	std::string::size_type start = 0, index;
	do
	{
		index = str.find_first_of(separator, start);
		if (index != std::string::npos)
		{
			substring = str.substr(start, index - start);
			dest.push_back(substring);
			start = str.find_first_not_of(separator, index);
			if (start == std::string::npos) return;
		}
	} while (index != std::string::npos);
	//the last token
	substring = str.substr(start);
	dest.push_back(substring);
}

void Map::split(const string& src, const char& separator, vector<string>& dest)
{
	int index = 0, start = 0;
	while (index != src.size())
	{
		if (src[index] == separator)
		{
			string substring = src.substr(start, index - start);
			dest.push_back(substring);
			while (src[index + 1] == separator)
			{
				dest.push_back("");
				index++;
			}
			index++;
			start = index;
		}
		else
			index++;
	}
	//the last token
	string substring = src.substr(start);
	dest.push_back(substring);
}

double Map::distM(Edge* edge1, Edge* edge2, double thresholdM /* = 0 */)
{
	//////////////////////////////////////////////////////////////////////////
	///返回两条路段的最短距离(一条路段上的点到另一条路段的最短距离),返回单位为米
	///如果计算过程中发现已经小于threshold米,则直接返回
	///注意：不是hausdoff距离！
	//////////////////////////////////////////////////////////////////////////
	double minDist = 9999999;
	//第一条到第二条
	for (Figure::iterator iter = edge1->figure->begin(); iter != edge1->figure->end(); iter++)
	{
		double tmpDist = distM((*iter)->lat, (*iter)->lon, edge2);
		if (tmpDist < minDist)
		{
			minDist = tmpDist;
			if (minDist < thresholdM)
				return minDist;
		}
	}
	//第二条到第一条
	for (Figure::iterator iter = edge2->figure->begin(); iter != edge2->figure->end(); iter++)
	{
		double tmpDist = distM((*iter)->lat, (*iter)->lon, edge1);
		if (tmpDist < minDist)
		{
			minDist = tmpDist;
			if (minDist < thresholdM)
				return minDist;
		}
	}
	return minDist;
}

void Map::getNearEdges(Edge* edge, double thresholdM, vector<Edge*>& dest)
{
	//////////////////////////////////////////////////////////////////////////
	///返回所有精确距离edge小于thresholdM的路段，存入dest
	//////////////////////////////////////////////////////////////////////////
	dest.clear();
	int gridSearchRange = int(thresholdM / (gridSizeDeg * GeoPoint::geoScale)) + 1;
	int minRow = INFINITE, maxRow = -1, minCol = INFINITE, maxCol = -1;
	for each (GeoPoint* pt in *(edge->figure))
	{
		int row = (pt->lat - area->minLat) / gridSizeDeg;
		int col = (pt->lon - area->minLon) / gridSizeDeg;
		if (row < minRow) minRow = row;
		if (row > maxRow) maxRow = row;
		if (col < minCol) minCol = col;
		if (col > maxCol) maxCol = col;
	}
	//求出搜索MBR
	minRow -= gridSearchRange;
	if (minRow < 0) minRow = 0;
	maxRow += gridSearchRange;
	if (maxRow >= gridHeight) maxRow = gridHeight - 1;
	minCol -= gridSearchRange;
	if (minCol < 0) minCol = 0;
	maxCol += gridSearchRange;
	if (maxCol >= gridWidth) maxCol = gridWidth - 1;
	double minDist = INFINITE;
	vector<Edge*> failList;
	for (int i = minRow; i <= maxRow; i++)
	{
		for (int j = minCol; j <= maxCol; j++)
		{
			for (list<Edge*>::iterator iter = grid[i][j]->begin(); iter != grid[i][j]->end(); iter++)
			{
				if ((*iter) == NULL)
					continue;
				if ((*iter) == edge) //防止把自己加进去
					continue;
				//本次迭代还未访问过
				if ((*iter)->visited != true)
				{
					(*iter)->visited = true; //标记为已访问
					double dist = distM(edge, (*iter), thresholdM);
					if (dist < thresholdM)// && dist > eps)
					{
						dest.push_back((*iter));
					}
					else
						failList.push_back((*iter));
				}
			}
		}
	}
	//visitFlag归零
	for (int i = 0; i < dest.size(); i++)
	{
		dest[i]->visited = false;
	}
	for (int i = 0; i < failList.size(); i++)
	{
		failList[i]->visited = false;
	}
}

bool smallerInX(simplePoint& pt1, simplePoint& pt2)
{
	//////////////////////////////////////////////////////////////////////////
	///将直线与网格交点统一按照x轴递增方向排列
	//////////////////////////////////////////////////////////////////////////
	return pt1.first < pt2.first;
}

bool smallerInDist(pair<Edge*, double>& c1, pair<Edge*, double>& c2)
{
	//////////////////////////////////////////////////////////////////////////
	///void getNearEdges(double lat, double lon, int k, vector<Edge*>& dest)函数中使用到的比较函数
	//////////////////////////////////////////////////////////////////////////
	return c1.second < c2.second;
}