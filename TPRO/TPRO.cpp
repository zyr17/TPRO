#include "TPRO.h"

//���캯����points: ·������, edges: ·������, id: ·��ID, length: ·������
TPROData::TPROData(std::vector<int> &inputpoints, std::vector<int> &inputedges, long long inputid, double length){
	id = inputid;
	pointsData = inputpoints;
	edges = inputedges;
	popularitySum = 0;
	routeLength = length;
}

//���캯�������뱣����ַ����ָ�״̬
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

//��������״̬�������ͨ��string���ء�����edges, pointsData, popularityData, popularitySum, id, routeLength
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

//���캯��������map, area, lonCut:����ʱ�����ȷֵĿ���, latCut:����ʱ��γ�ȷֵĿ���, topK:����ѡȡǰtopK���ŵ�·��
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

//���캯��������map, area, status����status�ַ���������ѵ����Ϣ����Ҫ�˹���֤�����ѵ����Ϣ��map, area��Ӧ
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

//����ѵ���ļ����ļ�Ϊscv��ʽ��һ��һ��·���������С������ļ�������·����vector��ÿ��·����һ��vector��ʾ
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

//����ѵ�����ݣ�routeData:·����vector��ÿ��·����һ��vector��ʾ��·��Ϊ������
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

//����ÿ��·�������Ŷȵ�ֵ������ѵ�����������ʱ���˳��
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

//����ѵ�����ݼ���ÿ��·����һ��vector��ʾ����Ϊ������
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

//����������·�����ݣ�����ÿ��·�����쳣ֵ��Ϊ0-1֮���һ��ʵ����routes:·����vector��ÿ��·����һ��vector��ʾ��·��Ϊ������
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

//����һ��·�����������·�����ڿ��topK·����vector��inputRoute:����·����������, result:���topK��·����������
void TPRO::getTopKRouteByRoute(std::vector<int> &inputRoute, std::vector<std::vector<int>> &res){
	std::vector<int> p;
	getTopKRouteByRoute(inputRoute, res, p);
}

//����һ��·�����������·�����ڿ��topK·����vector��inputRoute:����·����������, result:���topK��·����������, popularityResult:���ÿ��·�����ŶȵĽ��
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

//�õ�������������
int inline TPRO::getGroupId(int pointId){
	double lat = map->nodes[pointId]->lat;
	double lon = map->nodes[pointId]->lon;
	return int((lat - area->minLat) / dLat) * lonNum + int((lon - area->minLon)) / dLon;
}

//��Ŀǰ��ѵ��������档�ᱣ��dLon, dLat, lonNum, latNum, topK, groupData, topKData��map, areaΪָ�벻�ᱣ�档�������Ϊstring����
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

//������·���ı༭���롣����Ϊ������
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

//���������л������ݷֲ���data:��������, delta:���ֳɼ�������
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

//��ȡ ���� ���˲����ɣ��ǲ�����ĺ��ܡ���Ȼ��������˲����ɣ���������������N:���Ȼ��ֿ���, M:γ�Ȼ��ֿ���, K:topK·������, testAll:�Ƿ��ÿ��·�����쳣ֵ, showRandomMap:�Ƿ񴴽�PNGͼƬ�۲�Ч��, fixedGroup:����ͼƬ��ѡ·���Ƿ��Ϊһ������
void TPROFunc::TPROTest(int N, int M, int K, bool testAll, bool showRandomMap, bool fixedGroup){
	int startTime = time(NULL);
	//41.140519 41.175893 -8.651993 -8.579304
	double minLat = 41.140519, maxLat = 41.175893, minLon = -8.651993, maxLon = -8.579304;
	double dLat = maxLat - minLat, dLon = maxLon - minLon;
	/* �½�Area */
	Area area(minLat - dLat / 10, maxLat + dLat / 10, minLon - dLon / 10, maxLon + dLon / 10, true);
	/* �½�Map */
	Map map(std::string("map"), &area, 0.0);
	printf("map read done, %d\n", time(NULL) - startTime);
	//dLat: 4720.094166m, dLon: 9699.183717m
	/* ��ʼ��TPRO */
	TPRO tpro(&map, &area, N, M, K);
	printf("TPRO initialize done, %d\n", time(NULL) - startTime);
	/* ��ȡѵ������ */
	auto routes = tpro.setTrainData(std::string("cleaned_mm_edges.txt"));
	printf("train done, %d\n", time(NULL) - startTime);
	/*
	//����TPROData���ݱ������ȷ��
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
	//����TPRO���ݱ��棬�����ٶȺ���ȷ��
	auto res1 = tpro.saveStatus();
	printf("make cache done, %d\n", time(NULL) - startTime);
	TPRO tpro2(&map, &area, res1);
	printf("read cache done, %d\n", time(NULL) - startTime);
	getchar();
	exit(0);
	*/
	/*
	//����ѵ������ļ�
	FILE *f = fopen("tprostatus.txt", "w");
	fprintf(f, "%s", tpro.saveStatus().c_str());
	fclose(f);
	exit(0);
	*/
	if (testAll){
		int cc = clock();
		/* �����쳣ֵ */
		auto res = tpro.getAnomalousScore(routes);
		printf("res done\n");
		printf("time: %d\n", clock() - cc);
		/*FILE *f = fopen("res.txt", "w");
		for (auto i : res)
		fprintf(f, "%f\n", i);*/
		/* ���Ƽ������ݷֲ� */
		drawData(res);
	}
	/* ����PNG */
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
//��ȡ ���� ���˲����ɣ��ǲ��������һ�����ܡ���Ȼ��Ӳ��̫��������TPROTest��Ҫ����inputFileName:״̬�ļ���λ��, testAll:�Ƿ��ÿ��·�����쳣ֵ, showRandomMap:�Ƿ񴴽�PNGͼƬ�۲�Ч��, fixedGroup:����ͼƬ��ѡ·���Ƿ��Ϊһ������
void TPROFunc::TPROQuickTest(std::string inputFileName, bool testAll, bool showRandomMap, bool fixedGroup){
	int startTime = time(NULL);
	//41.140519 41.175893 -8.651993 -8.579304
	double minLat = 41.140519, maxLat = 41.175893, minLon = -8.651993, maxLon = -8.579304;
	double dLat = maxLat - minLat, dLon = maxLon - minLon;
	/* �½�Area */
	Area area(minLat - dLat / 10, maxLat + dLat / 10, minLon - dLon / 10, maxLon + dLon / 10, true);
	/* �½�Map */
	Map map(std::string("map"), &area, 0.0);
	printf("map read done, %d\n", time(NULL) - startTime);
	//dLat: 4720.094166m, dLon: 9699.183717m
	/* ��Ԥ������ʼ��TPRO */
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
		/* �����쳣ֵ */
		auto res = tpro.getAnomalousScore(routes);
		printf("res done\n");
		printf("time: %d\n", clock() - cc);
		/*FILE *f = fopen("res.txt", "w");
		for (auto i : res)
		fprintf(f, "%f\n", i);*/
		/* ���Ƽ������ݷֲ� */
		drawData(res);
	}
	/* ����PNG */
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


//����һ�Ű������ɹ켣��ͼƬ��map:Map��ָ��, tpro:TPRO���ָ��, route:һ��·���������У�������ͼƬ�л�������·��������·���������topK·��, filePath:�����ļ���λ�ã�Ϊһ���ļ���·��, filePrefix:�����ļ�����ǰ׺��Ĭ�ϻ����ļ��������maxPopularity��anomalousRate��������Ϣ���Ը����ڴ�
void TPROFunc::makePNGByRoute(Map *map, TPRO *tpro, std::vector<int> &route, std::string filePath, std::string filePrefix){
	/*
	* �ڵ�ͼ�ϻ���route���������topK·��������·�����ų̶Ȼ��ɺ쵽�ײ�ͬ����ɫ���������ص���Ч�����Ǻܺã���
	* ֮�󣬽�route�ý�ϸ����ɫ�߻����ڵ�ͼ�ϡ�Ϊ�˻��ƽ�ϸ��·�����Ķ���Map::drawRoute�������������Ƿ���ƴ�·����bool
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

//sscanf�����strlen̫���ˣ���д���ַ������ȡint��
inline void TPROFunc::getIntFromString(const char *buffer, int &i){
	i = 0;
	for (; *buffer >= '0' && *buffer <= '9'; buffer++)
		i = i * 10 + *buffer - '0';
}

//sscanf�����strlen̫���ˣ���д���ַ������ȡdouble��
inline void TPROFunc::getDoubleFromString(const char *buffer, double &i){
	//doubleû����������д��
	throw E_NOTIMPL;
}