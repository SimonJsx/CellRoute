#pragma once
#include <vector>
#include <cv.h>
#include <cxcore.h>
class CellRoute;

class CellManager
{
public:
	CellManager();
	~CellManager();
	
	// 将一幅图像中所有坐标作为输入参数
	void addCoordinates(CvPoint* pt, int size);

	std::vector<CellRoute*>& getCells()
	{
		return cells_;
	}

	// save data in image
	void saveRouteBmp(char* fileName, CvSize cs);
	void saveRouteBmp(IplImage* image);
	// save data in text
	void saveRoutText(char* fileName);
private:
	CvPoint* computeNearestPoint(CvPoint& query, CvPoint* pt, int size);
	int computeDistance(CvPoint& left, CvPoint& right);

	std::vector<CellRoute*> cells_;
};

