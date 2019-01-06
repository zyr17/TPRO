#include "PointGridIndex.h"

bool smallerThan(GeoPoint* pt1, GeoPoint* pt2)
{
	return pt1->dist < pt2->dist;
}
//////////////////////////////////////////////////////////////////////////
///public
//////////////////////////////////////////////////////////////////////////
void PointGridIndex::createIndex(list<GeoPoint*>& pts, Area* area, int gridWidth)
{
	this->area = area;
	this->gridWidth = gridWidth;
	initialization();
	for (list<GeoPoint*>::iterator ptIter = pts.begin(); ptIter != pts.end(); ptIter++)
	{
		insertOnePt(*ptIter);
	}
}

pair<int, int> PointGridIndex::getRowCol(GeoPoint* pt)
{
	//////////////////////////////////////////////////////////////////////////
	///first row, second col
	//////////////////////////////////////////////////////////////////////////
	return make_pair((int)((pt->lat - area->minLat) / gridSizeDeg), (int)((pt->lon - area->minLon) / gridSizeDeg));
}

void PointGridIndex::drawGridLine(Gdiplus::Color color, MapDrawer& md)
{
	//////////////////////////////////////////////////////////////////////////
	///��ͼƬ�ϻ��������� 
	//////////////////////////////////////////////////////////////////////////
	Gdiplus::ARGB argb = Gdiplus::Color::MakeARGB(90, color.GetR(), color.GetG(), color.GetB());
	color.SetValue(argb);
	double delta = 0.0000001;
	for (int i = 0; i < gridHeight; i++)
	{
		double lat = area->minLat + gridSizeDeg * i;
		md.drawLine(color, lat, area->minLon + delta, lat, area->maxLon - delta);
	}
	for (int i = 0; i < gridWidth; i++)
	{
		double lon = area->minLon + gridSizeDeg * i;
		md.drawLine(color, area->minLat + delta, lon, area->maxLat - delta, lon);
	}
}

void PointGridIndex::getNearPts(GeoPoint* pt, double thresholdM, vector<GeoPoint*>& dest)
{
	dest.clear();
	//calculate search range
	int gridSearchRange = int(thresholdM / (gridSizeDeg * GeoPoint::geoScale)) + 1;
	pair<int, int> rowCol = getRowCol(pt);
	int rowPt = rowCol.first;
	int colPt = rowCol.second;
	int row1 = rowPt - gridSearchRange;
	int col1 = colPt - gridSearchRange;
	int row2 = rowPt + gridSearchRange;
	int col2 = colPt + gridSearchRange;
	if (row1 < 0) row1 = 0;
	if (row2 >= gridHeight) row2 = gridHeight - 1;
	if (col1 < 0) col1 = 0;
	if (col2 >= gridWidth) col2 = gridWidth - 1;

	for (int row = row1; row <= row2; row++)
	{
		for (int col = col1; col <= col2; col++)
		{
			for (list<GeoPoint*>::iterator iter = grid[row][col]->begin(); iter != grid[row][col]->end(); iter++)
			{
				double dist = GeoPoint::distM((*iter), pt);
				if (dist < thresholdM && (*iter) != pt)
					dest.push_back((*iter));
			}
		}
	}
}

void PointGridIndex::getNearPts(GeoPoint* pt, int gridRange, vector<GeoPoint*>& dest)
{
	dest.clear();
	pair<int, int> rolCol = getRowCol(pt);
	int rowPt = rolCol.first;
	int colPt = rolCol.second;
	int row1 = rowPt - gridRange;
	int col1 = colPt - gridRange;
	int row2 = rowPt + gridRange;
	int col2 = colPt + gridRange;
	if (row1 < 0) row1 = 0;
	if (row2 >= gridHeight) row2 = gridHeight - 1;
	if (col1 < 0) col1 = 0;
	if (col2 >= gridWidth) col2 = gridWidth - 1;

	for (list<GeoPoint*>::iterator iter = grid[rowPt][colPt]->begin(); iter != grid[rowPt][colPt]->end(); iter++)
	{
		if ((*iter) != pt)
			dest.push_back((*iter));
	}
	for (int row = row1; row <= row2; row++)
	{
		for (int col = col1; col <= col2; col++)
		{
			if (row == rowPt && col == colPt)
				continue;			
			for (list<GeoPoint*>::iterator iter = grid[row][col]->begin(); iter != grid[row][col]->end(); iter++)
			{
				if ((*iter) != pt)
					dest.push_back((*iter));
			}
		}
	}
}

void PointGridIndex::kNN_approx(GeoPoint* pt, int k, double thresholdM, vector<GeoPoint*>& dest)
{
	//////////////////////////////////////////////////////////////////////////
	///�����k���㶼�������ptС��thresholdM�ף�����������������k���㷵��
	///[TODO]:͵����Ҫ��д��
	//////////////////////////////////////////////////////////////////////////
	dest.clear();
	vector<GeoPoint*> candidatePts;
	int range = 1;
	while (candidatePts.size() < k)
	{
		getNearPts(pt, range, candidatePts);
		range++;
	}
	for (int i = 0; i < candidatePts.size(); i++)
	{
		double dist = GeoPoint::distM(pt, candidatePts[i]);
		candidatePts[i]->dist = dist;
		if (dist < thresholdM && dest.size() < k)
		{
			dest.push_back(candidatePts[i]);
		}
		if (dest.size() == k)
			return;
	}
	dest.clear();
	sort(candidatePts.begin(), candidatePts.end(), smallerThan);
	for (int i = 0; i < k; i++)
	{
		dest.push_back(candidatePts[i]);
	}
}

void PointGridIndex::kNN_exact(GeoPoint* pt, int k, vector<GeoPoint*>& dest)
{
	//////////////////////////////////////////////////////////////////////////
	///��ȷkNN
	///���������� ���Բ�ѯ�����ڵĸ���Ϊ���ģ�gridSearchRangeΪ�����뾶����������
	///Ȼ����ÿ�������뾶����gridExtendStep�ķ���������ɢ������ֱ��candidates������Ԫ�ظ���
	///���ڵ���kֹͣ������
	//////////////////////////////////////////////////////////////////////////
	dest.clear();
	vector<GeoPoint*> candidatePts;
	//��������,��Χ�˸�
	int gridSearchRange = 1;
	int rowPt = getRowCol(pt).first;
	int colPt = getRowCol(pt).second;
	int row1 = rowPt - gridSearchRange;
	int col1 = colPt - gridSearchRange;
	int row2 = rowPt + gridSearchRange;
	int col2 = colPt + gridSearchRange;
	if (row1 < 0) row1 = 0;
	if (row1 >= gridHeight)
	{
		cout << "Խ��@PointGridIndex::kNN_exact" << endl;
		system("pause");
	}
	if (row2 >= gridHeight) row2 = gridHeight - 1;
	if (col1 < 0) col1 = 0;
	if (col1 >= gridWidth)
	{
		cout << "Խ��@PointGridIndex::kNN_exact" << endl;
		system("pause");
	}
	if (col2 >= gridWidth) col2 = gridWidth - 1;
	//��ʼɨ����Χ�˸񣬰����Լ�����
	for (int row = row1; row <= row2; row++)
	{
		for (int col = col1; col <= col2; col++)
		{
			for (list<GeoPoint*>::iterator iter = grid[row][col]->begin(); iter != grid[row][col]->end(); iter++)
			{
				if ((*iter) != pt)
					candidatePts.push_back((*iter));
			}
		}
	}

	int gridExtendStep = 1; //��չ������ÿ��������1��	
	int newRow1, newRow2, newCol1, newCol2; //��¼�µ�������Χ
	int preRow1 = row1;
	int preRow2 = row2;
	int preCol1 = col1;
	int preCol2 = col2;
	while (candidatePts.size() < k)
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
			cout << "�쳣 @getNearEdges" << endl;
			system("pause");
		}
		for (int row = newRow1; row <= newRow2; row++)
		{
			for (int col = newCol1; col <= newCol2; col++)
			{
				if (row >= preRow1 && row <= preRow2 && col >= preCol1 && col <= preCol2) //�Ѿ��������ľͲ�������
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
				for (list<GeoPoint*>::iterator iter = grid[row][col]->begin(); iter != grid[row][col]->end(); iter++)
				{
					if (*iter == NULL)
					{
						cout << "NULL";
						system("pause");
					}
					candidatePts.push_back((*iter));
				}

			}
		}
		preRow1 = newRow1;
		preRow2 = newRow2;
		preCol1 = newCol1;
		preCol2 = newCol2;
		if (newRow1 == 0 && newRow2 == gridHeight - 1 && newCol1 == 0 && newCol2 == gridWidth - 1)//�������ȫͼ��û���㣬���ж�����
			break;
	}
	//����candidateÿ���㵽pt�ľ���
	for (int i = 0; i < candidatePts.size(); i++)
	{
		candidatePts[i]->dist = GeoPoint::distM(pt, candidatePts[i]);
	}
	sort(candidatePts.begin(), candidatePts.end(), smallerThan);
	if (candidatePts.size() < k)
	{
		cout << "�ѱ�ȫͼҲ�޷���ȡk��NN" << endl;
		system("pause");
	}
	for (int i = 0; i < k; i++)
	{
		dest.push_back(candidatePts[i]);
	}
	return;
}

//////////////////////////////////////////////////////////////////////////
///private
//////////////////////////////////////////////////////////////////////////
void PointGridIndex::initialization()
{
	//////////////////////////////////////////////////////////////////////////
	///��ʼ����������²���
	///1.����area������������
	///2.�ڶ��Ͽ��ռ�
	///[ATTENTION]��[MEM_LEAK]��������ʱ�����ж�grid�Ƿ���NULL��������deleteǰһ�������ռ�
	//////////////////////////////////////////////////////////////////////////	
	if (gridWidth <= 0)
		return;
	gridHeight = int((area->maxLat - area->minLat) / (area->maxLon - area->minLon) * double(gridWidth)) + 1;
	gridSizeDeg = (area->maxLon - area->minLon) / double(gridWidth);
	grid = new list<GeoPoint*>* *[gridHeight];
	for (int i = 0; i < gridHeight; i++)
		grid[i] = new list<GeoPoint*>*[gridWidth];
	for (int i = 0; i < gridHeight; i++)
	{
		for (int j = 0; j < gridWidth; j++)
		{
			grid[i][j] = new list<GeoPoint*>();
		}
	}
	printf("Point index gridWidth = %d, gridHeight = %d\n", gridWidth, gridHeight);
	cout << "gridSize = " << gridSizeDeg * GeoPoint::geoScale << "m" << endl;
}

void PointGridIndex::insertOnePt(GeoPoint* pt)
{
	if (!(pt->lat > area->minLat && pt->lat < area->maxLat && pt->lon > area->minLon && pt->lon < area->maxLon))
	{
		printf("pt(%.8lf, %.8lf)\n", pt->lat, pt->lon);
		area->print();
		system("pause");
	}
	pair<int, int> rolCol = getRowCol(pt);
	grid[rolCol.first][rolCol.second]->push_back(pt);
}