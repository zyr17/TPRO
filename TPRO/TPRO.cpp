#include "TPRO.h"

//构造函数。points: 路径点序, edges: 路径边序, id: 路径ID, length: 路径长度
TPROData::TPROData(std::vector<int> &inputpoints, std::vector<int> &inputedges, long long inputid, double length){
	id = inputid;
	pointsData = inputpoints;
	edges = inputedges;
	popularitySum = 0;
	routeLength = length;
}

//构造函数。传入保存的字符串恢复状态
TPROData::TPROData(std::string &status){
	int nowpos = 0, temp = 0;
	const char *buffer = status.c_str();
	TPROFunc::getIntFromString(buffer + nowpos, temp);
	for (; buffer[nowpos++] != ' ';);
	for (int i = temp; i--;){
		TPROFunc::getIntFromString(buffer + nowpos, temp);
		for (; buffer[nowpos++] != ' ';);
		edges.push_back(temp);
	}
	TPROFunc::getIntFromString(buffer + nowpos, temp);
	for (; buffer[nowpos++] != ' ';);
	for (int i = temp; i--;){
		TPROFunc::getIntFromString(buffer + nowpos, temp);
		for (; buffer[nowpos++] != ' ';);
		pointsData.push_back(temp);
	}
	TPROFunc::getIntFromString(buffer + nowpos, temp);
	for (; buffer[nowpos++] != ' ';);
	for (int i = temp; i--;){
		TPROFunc::getIntFromString(buffer + nowpos, temp);
		for (; buffer[nowpos++] != ' ';);
		popularityData.push_back(temp);
	}
	TPROFunc::getIntFromString(buffer + nowpos, temp);
	for (; buffer[nowpos++] != ' ';);
	popularitySum = temp;
	TPROFunc::getIntFromString(buffer + nowpos, temp);
	for (; buffer[nowpos++] != ' ';);
	id = temp;
	double tdouble;
	sscanf(buffer + nowpos, "%lf ", &tdouble);
	routeLength = tdouble;
}

//保存数据状态。将结果通过string返回。保存edges, pointsData, popularityData, popularitySum, id, routeLength
std::string TPROData::saveStatus(){
	char buffer[111];
	std::string res = "";
	sprintf(buffer, "%d ", edges.size());
	res += buffer;
	for (auto i : edges){
		sprintf(buffer, "%d ", i);
		res += buffer;
	}
	sprintf(buffer, "%d ", pointsData.size());
	res += buffer;
	for (auto i : pointsData){
		sprintf(buffer, "%d ", i);
		res += buffer;
	}
	sprintf(buffer, "%d ", popularityData.size());
	res += buffer;
	for (auto i : popularityData){
		sprintf(buffer, "%d ", i);
		res += buffer;
	}
	sprintf(buffer, "%d %d %.20f ", popularitySum, id, routeLength);
	res += buffer;
	return res;
}

//构造函数。输入map, area, lonCut:分组时将经度分的块数, latCut:分组时将纬度分的块数, topK:最终选取前topK热门的路径
TPRO::TPRO(Map *inputMap, Area *inputArea, int n, int m, int k){
	map = inputMap;
	area = inputArea;
	dLat = (area->maxLat - area->minLat) / n;
	dLon = (area->maxLon - area->minLon) / m;
	latNum = n;
	lonNum = m;
	topK = k;
	//printf("dLat:%f, dLon:%f, latNum:%d, lonNum:%d\n", dLat, dLon, latNum, lonNum);
	/*
	groupData.resize(n * m);
	for (int i = 0; i < n * m; i++)
		groupData[i].resize(n * m);
	*/
	editDistanceDP.resize(1111);
	for (auto &i : editDistanceDP)
		i.resize(1111);
}

//构造函数。输入map, area, status。从status字符串内载入训练信息。需要人工保证保存的训练信息和map, area对应
TPRO::TPRO(Map *inputMap, Area *inputArea, std::string &status){
	map = inputMap;
	area = inputArea;
	const char *buffer = status.c_str();
	int nowpos = 0;
	sscanf(buffer + nowpos, "%lf", &dLon);
	for (; buffer[nowpos++] != '\n';);
	sscanf(buffer + nowpos, "%lf", &dLat);
	for (; buffer[nowpos++] != '\n';);
	TPROFunc::getIntFromString(buffer + nowpos, lonNum);
	for (; buffer[nowpos++] != '\n';);
	TPROFunc::getIntFromString(buffer + nowpos, latNum);
	for (; buffer[nowpos++] != '\n';);
	TPROFunc::getIntFromString(buffer + nowpos, topK);
	for (; buffer[nowpos++] != '\n';);
	int temp;
	TPROFunc::getIntFromString(buffer + nowpos, temp);
	for (; buffer[nowpos++] != '\n';);
	for (; temp--;){
		int t1, t2, t3;
		//sscanf(buffer + nowpos, "%d", &t1);
		TPROFunc::getIntFromString(buffer + nowpos, t1);
		for (; buffer[nowpos++] != '\n';);
		TPROFunc::getIntFromString(buffer + nowpos, t2);
		for (; buffer[nowpos++] != '\n';);
		TPROFunc::getIntFromString(buffer + nowpos, t3);
		for (; buffer[nowpos++] != '\n';);
		//if (temp % 1000 == 0) printf("groupData remain: %d\n", temp);
		auto &k = groupData[t1][t2];
		for (int i = t3; i--;){
			int nextpos = nowpos;
			for (; buffer[++nextpos] != '|';);
			std::string datastr = status.substr(nowpos, nextpos - nowpos);
			nowpos = nextpos + 1;
			k.push_back(TPROData(datastr));
			//printf("%s\n", datastr.c_str());
			//getchar();
		}
	}
	TPROFunc::getIntFromString(buffer + nowpos, temp);
	for (; buffer[nowpos++] != '\n';);
	for (; temp--;){
		int t1, t2, t3;
		TPROFunc::getIntFromString(buffer + nowpos, t1);
		for (; buffer[nowpos++] != '\n';);
		TPROFunc::getIntFromString(buffer + nowpos, t2);
		for (; buffer[nowpos++] != '\n';);
		TPROFunc::getIntFromString(buffer + nowpos, t3);
		for (; buffer[nowpos++] != '\n';);
		//if (temp % 1000 == 0) printf("topKData remain: %d\n", temp);
		auto &k = topKData[t1][t2];
		for (int i = t3; i--;){
			int nextpos = nowpos;
			for (; buffer[++nextpos] != '|';);
			std::string datastr = status.substr(nowpos, nextpos - nowpos);
			nowpos = nextpos + 1;
			k.push_back(TPROData(datastr));
		}
	}
	editDistanceDP.resize(1111);
	for (auto &i : editDistanceDP)
		i.resize(1111);
}

//输入训练文件，文件为scv格式，一行一个路径，边序列。返回文件包含的路径的vector，每条路径用一个vector表示
std::vector<std::vector<int>> TPRO::setTrainData(std::string fileName){
	std::vector<std::vector<int>> res;
	FILE *file = fopen(fileName.c_str(), "r");
	for (;;){
		char buffer[11111];
		if (NULL == fgets(buffer, 11111, file)) break;
		int len = strlen(buffer);
		std::vector<int> oneData;
		for (int i = 0; i < len - 1; i++){
			int t = 0;
			for (; buffer[i] >= '0' && buffer[i] <= '9'; i++)
				t = t * 10 + buffer[i] - '0';
			//printf("%d\n", t);
			//getchar();
			oneData.push_back(t);
		}
		//printf("end\n");
		//getchar();
		res.push_back(oneData);
	}
	setTrainData(res);
	return res;
}

//输入训练数据，routeData:路径的vector，每条路径用一个vector表示，路径为边序列
void TPRO::setTrainData(std::vector<std::vector<int>> &dataVector){
	//FILE *f = fopen("a.txt", "w");
	int id = 0;
	for (auto i : dataVector){
		std::vector<int> points;
		int start = i[0], end = *i.rbegin();
		start = map->edges[start]->startNodeId;
		end = map->edges[end]->endNodeId;
		points.push_back(start);
		start = getGroupId(start);
		end = getGroupId(end);
		double length = 0;
		for (auto j : i){
			points.push_back(map->edges[j]->endNodeId);
			length += map->edges[j]->lengthM;
		}
		groupData[start][end].push_back(TPROData(points, i, id++, length));
	}
	std::vector<int> timesCount, countLastChange;
	timesCount.resize(map->nodes.size());
	countLastChange.resize(map->nodes.size());
	int changeTime = 0;
	std::map<int, int> counter;
	for (auto &i : groupData)
		for (auto &j : i.second){
			counter[j.second.size()]++;
			for (auto &k : j.second){
				changeTime++;
				for (auto l : k.pointsData)
					if (countLastChange[l] != changeTime){
						countLastChange[l] = changeTime;
						timesCount[l]++;
					}
			}
			for (auto &k : j.second){
				changeTime++;
				for (auto l : k.pointsData)
					if (countLastChange[l] != changeTime){
						countLastChange[l] = changeTime;
						k.popularityData.push_back(timesCount[l]);
					}
				std::sort(k.popularityData.begin(), k.popularityData.end());
				k.popularitySum = 0;
				for (auto l : k.popularityData)
					k.popularitySum += l;
			}
			for (auto &k : j.second)
				for (auto l : k.pointsData)
					timesCount[l] = 0;
			std::sort(j.second.begin(), j.second.end());
			std::set<double> lengthset;
			for (auto &k : j.second)
				if (lengthset.size() >= topK)
					break;
				else if (lengthset.find(k.routeLength) == lengthset.end()){
					lengthset.insert(k.routeLength);
					topKData[i.first][j.first].push_back(k);
				}
			/*for (auto &k : j.second){
				for (auto &l : k.popularityData)
					fprintf(f, "%d ", l);
				fprintf(f, "\n");
			}
			fprintf(f, "----------\n");
			for (auto &k : topKData[i.first][j.first]){
				for (auto &l : k.popularityData)
					fprintf(f, "%d ", l);
				fprintf(f, "\n");
			}
			fprintf(f, "==========\n");*/
		}
	//fclose(f);
	/*for (auto i : counter)
		if (i.first < 100) printf("%d: %d\n", i.first, i.second);
	getchar();*/
}

//给出每条路径的热门度的值，按照训练数据输入的时候的顺序
std::vector<int> TPRO::getPopularitySum(){
	std::vector<std::pair<int, int>> sorts;
	for (auto &i : groupData)
		for (auto &j : i.second)
			for (auto &k : j.second)
				sorts.push_back(std::make_pair(k.id, k.popularitySum));
	std::sort(sorts.begin(), sorts.end());
	std::vector<int> res;
	for (auto i : sorts)
		res.push_back(i.second);
	return res;
}

//给出训练数据集，每条路径用一个vector表示，内为边序列
std::vector<std::vector<int>> TPRO::getTrainData(){
	std::vector<std::pair<int, std::vector<int>>> sorts;
	for (auto &i : groupData)
		for (auto &j : i.second)
			for (auto &k : j.second)
				sorts.push_back(std::make_pair(k.id, k.edges));
	std::sort(sorts.begin(), sorts.end());
	std::vector<std::vector<int>> res;
	for (auto i : sorts)
		res.push_back(i.second);
	return res;
}

//输入若干条路径数据，返回每条路径的异常值，为0-1之间的一个实数。routes:路径的vector，每条路径用一个vector表示，路径为边序列
std::vector<double> TPRO::getAnomalousScore(std::vector<std::vector<int>> &inputData){
	std::vector<double> res;
	auto &groupData = topKData;
	for (auto &i : inputData){
		long long tt = (&i - &inputData[0]);// / sizeof(std::vector<int>);
		if (tt % 10000 == 0 & tt) printf("calculating %d anomalous score\n", tt);
		std::vector<int> points;
		int start = i[0], end = *i.rbegin();
		start = map->edges[start]->startNodeId;
		end = map->edges[end]->endNodeId;
		points.push_back(start);
		start = getGroupId(start);
		end = getGroupId(end);
		for (auto j : i)
			points.push_back(map->edges[j]->endNodeId);
		double popularityWeightUp = 0, popularityWeightDown = 0;
		//if (groupData[start][end].size() > topK) printf("warning! too big: %d\n", groupData[start][end].size());
		for (int i = 0; i < topK && i < groupData[start][end].size(); i++){
			auto &oneData = groupData[start][end][i];
			popularityWeightDown += oneData.popularitySum;
			double delta = editDistance(points, oneData.pointsData);
			popularityWeightUp += oneData.popularitySum * delta;
			/*if (inputData.size() == 1){
				for (auto j : points)
					printf("%d ", j);
				printf("\n----------\n");
				for (auto j : oneData.pointsData)
					printf("%d ", j);
				printf("\n----------\n");
				printf("%d * %f = %f\n", oneData.popularitySum, delta, oneData.popularitySum * delta);
			}*/
		}
		//if (inputData.size() == 1) printf("%f / %f = %f\n", popularityWeightUp, popularityWeightDown, popularityWeightUp / popularityWeightDown);
		double popularityWeight = popularityWeightDown == 0 ? 1 : popularityWeightUp / popularityWeightDown;
		res.push_back(popularityWeight);
		/*if (groupData[start][end].size() == 1){
			printf("%f\n", popularityWeight);
			for (auto i : points)
				printf("%d ", i);
			printf("\n");
			for (auto i : groupData[start][end][0].pointsData)
				printf("%d ", i);
			printf("\n");
			for (int i = 0; i <= points.size(); i++){
				for (int j = 0; j <= groupData[start][end][0].pointsData.size(); j++)
					printf("%d ", editDistanceDP[i][j]);
				printf("\n");
			}
			getchar();
		}*/
	}
	return res;
}

//输入一条路径，输出这条路径所在块的topK路径的vector。inputRoute:输入路径，边序列, result:输出topK的路径，边序列
void TPRO::getTopKRouteByRoute(std::vector<int> &inputRoute, std::vector<std::vector<int>> &res){
	std::vector<int> p;
	getTopKRouteByRoute(inputRoute, res, p);
}

//输入一条路径，输出这条路径所在块的topK路径的vector。inputRoute:输入路径，边序列, result:输出topK的路径，边序列, popularityResult:输出每条路径热门度的结果
void TPRO::getTopKRouteByRoute(std::vector<int> &inputRoute, std::vector<std::vector<int>> &res, std::vector<int> &popularity){
	res.clear();
	int start = inputRoute[0], end = *inputRoute.rbegin();
	start = map->edges[start]->startNodeId;
	end = map->edges[end]->endNodeId;
	start = getGroupId(start);
	end = getGroupId(end);
	for (auto &i : topKData[start][end]){
		res.push_back(i.edges);
		popularity.push_back(i.popularitySum);
	}
}

//得到点所属的区域。
int inline TPRO::getGroupId(int pointId){
	double lat = map->nodes[pointId]->lat;
	double lon = map->nodes[pointId]->lon;
	return int((lat - area->minLat) / dLat) * lonNum + int((lon - area->minLon)) / dLon;
}

//将目前的训练结果保存。会保存dLon, dLat, lonNum, latNum, topK, groupData, topKData。map, area为指针不会保存。将结果作为string返回
std::string TPRO::saveStatus(){
	std::string res = "";
	char buffer[111];
	sprintf(buffer, "%.20f\n%.20f\n%d\n%d\n%d\n", dLon, dLat, lonNum, latNum, topK);
	res += buffer;
	int tot = 0;
	for (auto &i : groupData)
		tot += i.second.size();
	sprintf(buffer, "%d\n", tot);
	res += buffer;
	for (auto &i : groupData)
		for (auto &j : i.second){
			sprintf(buffer, "%d\n%d\n%d\n", i.first, j.first, j.second.size());
			res += buffer;
			for (auto &k : j.second){
				res += k.saveStatus();
				res += "|";
			}
		}
	tot = 0;
	for (auto &i : topKData)
		tot += i.second.size();
	sprintf(buffer, "%d\n", tot);
	res += buffer;
	for (auto &i : topKData)
		for (auto &j : i.second){
			sprintf(buffer, "%d\n%d\n%d\n", i.first, j.first, j.second.size());
			res += buffer;
			for (auto &k : j.second){
				res += k.saveStatus();
				res += "|";
			}
		}
	return res;
}

//求两条路径的编辑距离。输入为点序列
double TPRO::editDistance(std::vector<int> &a, std::vector<int> &b){
	int n = a.size();
	int m = b.size();
	assert(n < 1111 && m < 1111);
	for (int i = 0; i <= n; i++)
		for (int j = 0; j <= m; j++){
			if (i == 0 && j == 0)
				editDistanceDP[i][j] = 0;
			else if (i == 0)
				editDistanceDP[i][j] = j;
			else if (j == 0)
				editDistanceDP[i][j] = i;
			else{
				if (a[i - 1] == b[j - 1])
					editDistanceDP[i][j] = editDistanceDP[i - 1][j - 1];
				else
					editDistanceDP[i][j] = editDistanceDP[i - 1][j - 1] + 1;
				if (editDistanceDP[i - 1][j] + 1 < editDistanceDP[i][j])
					editDistanceDP[i][j] = editDistanceDP[i - 1][j] + 1;
				if (editDistanceDP[i][j - 1] + 1 < editDistanceDP[i][j])
					editDistanceDP[i][j] = editDistanceDP[i][j - 1] + 1;
			}
		}
	return editDistanceDP[n][m] * 1.0 / max(n, m);
}

//在命令行中绘制数据分布。data:数据内容, delta:划分成几个部分
void TPROFunc::drawData(std::vector<double> &inputVector, int delta){
	std::vector<int> bucket;
	bucket.resize(delta + 1);
	for (auto i : inputVector)
		bucket[(int)(i / (1.0 / delta))]++;
	int maxlength = 60;
	double max = 0;
	for (auto i : bucket)
		if (max < i) max = i;
	for (auto &i : bucket)
		i = i * 1.0 / max * 60;
	double nowDelta = -1.0 / delta;
	for (auto i : bucket){
		printf("%f: ", nowDelta += 1.0 / delta);
		for (int j = 0; j < i; j++)
			printf("#");
		printf("\n");
	}
}

//读取 处理 求解瞬间完成，是测试里的豪杰。（然而并不能瞬间完成，甚至还会编译错误）N:经度划分块数, M:纬度划分块数, K:topK路径条数, testAll:是否对每条路径求异常值, showRandomMap:是否创建PNG图片观测效果, fixedGroup:创建图片所选路径是否均为一个集合
void TPROFunc::TPROTest(int N, int M, int K, bool testAll, bool showRandomMap, bool fixedGroup){
	int startTime = time(NULL);
	//41.140519 41.175893 -8.651993 -8.579304
	double minLat = 41.140519, maxLat = 41.175893, minLon = -8.651993, maxLon = -8.579304;
	double dLat = maxLat - minLat, dLon = maxLon - minLon;
	/* 新建Area */
	Area area(minLat - dLat / 10, maxLat + dLat / 10, minLon - dLon / 10, maxLon + dLon / 10, true);
	/* 新建Map */
	Map map(std::string("map"), &area, 0.0);
	printf("map read done, %d\n", time(NULL) - startTime);
	//dLat: 4720.094166m, dLon: 9699.183717m
	/* 初始化TPRO */
	TPRO tpro(&map, &area, N, M, K);
	printf("TPRO initialize done, %d\n", time(NULL) - startTime);
	/* 读取训练数据 */
	auto routes = tpro.setTrainData(std::string("cleaned_mm_edges.txt"));
	printf("train done, %d\n", time(NULL) - startTime);
	/*
	//测试TPROData数据保存的正确性
	auto &onedata = (++ ++ ++(++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++tpro.groupData.begin())->second.begin())->second[0];
	auto res1 = onedata.saveStatus();
	FILE *f1 = fopen("z1.txt", "w");
	fprintf(f1, "%s", res1.c_str());
	TPROData twodata(res1);
	auto res2 = twodata.saveStatus();
	FILE *f2 = fopen("z2.txt", "w");
	fprintf(f2, "%s", res2.c_str());
	exit(0);
	*/
	/*
	//测试TPRO数据保存，检验速度和正确性
	auto res1 = tpro.saveStatus();
	printf("make cache done, %d\n", time(NULL) - startTime);
	TPRO tpro2(&map, &area, res1);
	printf("read cache done, %d\n", time(NULL) - startTime);
	getchar();
	exit(0);
	*/
	/*
	//生成训练结果文件
	FILE *f = fopen("tprostatus.txt", "w");
	fprintf(f, "%s", tpro.saveStatus().c_str());
	fclose(f);
	exit(0);
	*/
	if (testAll){
		int cc = clock();
		/* 计算异常值 */
		auto res = tpro.getAnomalousScore(routes);
		printf("res done\n");
		printf("time: %d\n", clock() - cc);
		/*FILE *f = fopen("res.txt", "w");
		for (auto i : res)
		fprintf(f, "%f\n", i);*/
		/* 绘制简易数据分布 */
		drawData(res);
	}
	/* 绘制PNG */
	if (showRandomMap){
		system("mkdir PNGs");
		srand(unsigned(time(NULL)));
		rand();
		auto popularitySums = tpro.getPopularitySum();
		if (fixedGroup){
			int first = (rand() * 65536 + rand()) % routes.size();
			int stid = tpro.getGroupId(map.edges[routes[first][0]]->startNodeId);
			int edid = tpro.getGroupId(map.edges[*routes[first].rbegin()]->endNodeId);
			std::vector<int> candidate;
			for (auto &i : routes){
				int istid = tpro.getGroupId(map.edges[i[0]]->startNodeId);
				int iedid = tpro.getGroupId(map.edges[*i.rbegin()]->endNodeId);
				if (istid == stid && iedid == edid){
					candidate.push_back(&i - &routes[0]);
					//printf("%d\n", *candidate.rbegin());
					//printf("%d %d %d %d\n", istid, iedid, tpro.getGroupId(map.edges[routes[*candidate.rbegin()][0]]->startNodeId), tpro.getGroupId(map.edges[*routes[*candidate.rbegin()].rbegin()]->endNodeId));
				}
			}
			for (;;){
				int routeNum = (rand() * 65536 + rand()) % candidate.size();
				std::string prefix = "populritySum_";
				char buffer[111];
				sprintf(buffer, "%d-", popularitySums[candidate[routeNum]]);
				makePNGByRoute(&map, &tpro, routes[candidate[routeNum]], "PNGs", prefix + buffer);
				printf("Output one map, route ID: %d\n", candidate[routeNum]);
				getchar();
			}
		}
		else
			for (;;){
				int routeNum = (rand() * 65536 + rand()) % routes.size();
				std::string prefix = "populritySum_";
				char buffer[111];
				sprintf(buffer, "%d-", popularitySums[routeNum]);
				makePNGByRoute(&map, &tpro, routes[routeNum], "PNGs", prefix + buffer);
				printf("Output one map, route ID: %d\n", routeNum);
				getchar();
			}
	}
}
//读取 处理 求解瞬间完成，是测试里的另一个豪杰。（然而硬盘太烂甚至比TPROTest还要慢）inputFileName:状态文件的位置, testAll:是否对每条路径求异常值, showRandomMap:是否创建PNG图片观测效果, fixedGroup:创建图片所选路径是否均为一个集合
void TPROFunc::TPROQuickTest(std::string inputFileName, bool testAll, bool showRandomMap, bool fixedGroup){
	int startTime = time(NULL);
	//41.140519 41.175893 -8.651993 -8.579304
	double minLat = 41.140519, maxLat = 41.175893, minLon = -8.651993, maxLon = -8.579304;
	double dLat = maxLat - minLat, dLon = maxLon - minLon;
	/* 新建Area */
	Area area(minLat - dLat / 10, maxLat + dLat / 10, minLon - dLon / 10, maxLon + dLon / 10, true);
	/* 新建Map */
	Map map(std::string("map"), &area, 0.0);
	printf("map read done, %d\n", time(NULL) - startTime);
	//dLat: 4720.094166m, dLon: 9699.183717m
	/* 用预存结果初始化TPRO */
	std::string tprostatus = "";
	FILE *statusfile = fopen(inputFileName.c_str(), "r");
	for (char ch; (ch = getc(statusfile)) != EOF;)
		tprostatus += ch;
	fclose(statusfile);
	printf("read file done, %d\n", time(NULL) - startTime);
	TPRO tpro(&map, &area, tprostatus);
	auto routes = tpro.getTrainData();
	printf("TPRO initialize done, %d\n", time(NULL) - startTime);

	if (testAll){
		int cc = clock();
		/* 计算异常值 */
		auto res = tpro.getAnomalousScore(routes);
		printf("res done\n");
		printf("time: %d\n", clock() - cc);
		/*FILE *f = fopen("res.txt", "w");
		for (auto i : res)
		fprintf(f, "%f\n", i);*/
		/* 绘制简易数据分布 */
		drawData(res);
	}
	/* 绘制PNG */
	if (showRandomMap){
		system("mkdir PNGs");
		srand(unsigned(time(NULL)));
		rand();
		auto popularitySums = tpro.getPopularitySum();
		if (fixedGroup){
			int first = (rand() * 65536 + rand()) % routes.size();
			int stid = tpro.getGroupId(map.edges[routes[first][0]]->startNodeId);
			int edid = tpro.getGroupId(map.edges[*routes[first].rbegin()]->endNodeId);
			std::vector<int> candidate;
			for (auto &i : routes){
				int istid = tpro.getGroupId(map.edges[i[0]]->startNodeId);
				int iedid = tpro.getGroupId(map.edges[*i.rbegin()]->endNodeId);
				if (istid == stid && iedid == edid){
					candidate.push_back(&i - &routes[0]);
					//printf("%d\n", *candidate.rbegin());
					//printf("%d %d %d %d\n", istid, iedid, tpro.getGroupId(map.edges[routes[*candidate.rbegin()][0]]->startNodeId), tpro.getGroupId(map.edges[*routes[*candidate.rbegin()].rbegin()]->endNodeId));
				}
			}
			for (;;){
				int routeNum = (rand() * 65536 + rand()) % candidate.size();
				std::string prefix = "populritySum_";
				char buffer[111];
				sprintf(buffer, "%d-", popularitySums[candidate[routeNum]]);
				makePNGByRoute(&map, &tpro, routes[candidate[routeNum]], "PNGs", prefix + buffer);
				printf("Output one map, route ID: %d\n", candidate[routeNum]);
				getchar();
			}
		}
		else
			for (;;){
				int routeNum = (rand() * 65536 + rand()) % routes.size();
				std::string prefix = "populritySum_";
				char buffer[111];
				sprintf(buffer, "%d-", popularitySums[routeNum]);
				makePNGByRoute(&map, &tpro, routes[routeNum], "PNGs", prefix + buffer);
				printf("Output one map, route ID: %d\n", routeNum);
				getchar();
			}
	}
}


//绘制一张包含若干轨迹的图片。map:Map的指针, tpro:TPRO类的指针, route:一条路径，边序列，将会在图片中画上这条路径和这条路径所在组的topK路径, filePath:保存文件的位置，为一个文件夹路径, filePrefix:保存文件名的前缀。默认会在文件名后添加maxPopularity和anomalousRate，其余信息可以附加在此
void TPROFunc::makePNGByRoute(Map *map, TPRO *tpro, std::vector<int> &route, std::string filePath, std::string filePrefix){
	/*
	* 在地图上绘制route所属分组的topK路径。根据路径热门程度会由红到白不同的颜色（由于有重叠，效果不是很好）。
	* 之后，将route用较细的紫色线绘制在地图上。为了绘制较细的路径，改动了Map::drawRoute函数，增加了是否绘制粗路径的bool
	*/
	MapDrawer md;
	md.setArea(map->area);
	md.setResolution(1920);
	md.newBitmap();
	md.lockBits();
	map->drawMap(Gdiplus::Color(255, 222, 173), md, false);
	std::vector<std::vector<int>> routeRes;
	std::vector<int> popularityRes;
	tpro->getTopKRouteByRoute(route, routeRes, popularityRes);
	int maxPopularity = 0;
	for (auto i : popularityRes)
		if (maxPopularity < i)
			maxPopularity = i;
	for (int i = 0; i < routeRes.size(); i++){
		int t = 256 - popularityRes[i] * 256 / maxPopularity;
		if (t == 256) t = 255;
		map->drawRoute(Gdiplus::Color(255, t, t), md, routeRes[i]);
	}

	map->drawRoute(Gdiplus::Color(128, 0, 128), md, route, false);
	md.unlockBits();
	char fileName[100];
	if (filePath[filePath.size() - 1] != '\\')
		filePath += '\\';
	std::vector<std::vector<int>> routes;
	routes.push_back(route);
	auto res = tpro->getAnomalousScore(routes);
	sprintf(fileName, "maxPopularity_%d-anomalousRate_%f.png", maxPopularity, res[0]);
	md.saveBitmap(filePath + filePrefix + fileName);
}

//sscanf会调用strlen太慢了，手写从字符串里读取int型
inline void TPROFunc::getIntFromString(const char *buffer, int &i){
	i = 0;
	for (; *buffer >= '0' && *buffer <= '9'; buffer++)
		i = i * 10 + *buffer - '0';
}

//sscanf会调用strlen太慢了，手写从字符串里读取double型
inline void TPROFunc::getDoubleFromString(const char *buffer, double &i){
	//double没几个，不想写了
	throw E_NOTIMPL;
}