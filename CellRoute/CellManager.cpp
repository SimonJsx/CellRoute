#include "CellManager.h"
#include <highgui.h>
#include "CellRoute.h"

CellManager::CellManager()
{
}


CellManager::~CellManager()
{
}

void CellManager::addCoordinates(CvPoint* pt, int size)
{
	if (cells_.empty())
	{
		for (int i = 0; i < size; ++i)
		{
			CellRoute* t = new CellRoute(pt[i].x, pt[i].y);
			t->addRoute(pt[i].x, pt[i].y);
			t->statusStable();
			cells_.push_back(t);
		}		
	}
	else
	{
		for (size_t i = 0; i < cells_.size(); ++i)
		{
			CvPoint& prePt = cells_[i]->predict();
		//	printf("(%d, %d)\n", prePt.x, prePt.y);
			CvPoint* nearestPt = computeNearestPoint(prePt, pt, size);
			if (nearestPt != nullptr)
			{
				cells_[i]->addRoute(nearestPt->x, nearestPt->y);
			}				
		}
	}
}

CvPoint* CellManager::computeNearestPoint(CvPoint& query, CvPoint* pt, int size)
{
	int minDis = computeDistance(query, pt[0]);
	int minPoint = 0;
	for (int i = 1; i < size; ++i)
	{
		int distance = computeDistance(query, pt[i]);
		if (distance < minDis)
		{
			minDis = distance;
			minPoint = i;
		}
	}

	if (minDis > 500)	// set move size sqrt(minDis) is the true distance
	{
		return nullptr;
	}

	return pt + minPoint;
}

int CellManager::computeDistance(CvPoint& left, CvPoint& right)
{
	return (left.x - right.x) * (left.x - right.x) + (left.y - right.y) * (left.y - right.y);
}

/* save data */
void CellManager::saveRouteBmp(char* fileName, CvSize cs)
{
	IplImage* image = cvCreateImage(cs, 8, 3);

	saveRouteBmp(image);

	cvSaveImage(fileName, image);
}

void CellManager::saveRouteBmp(IplImage* image)
{
	for (size_t i = 0; i < cells_.size(); ++i)
	{
		saveBmp(*cells_[i], image, CV_RGB(255 - i, 128 + i, i));
	}
}

void CellManager::saveRoutText(char* fileName)
{
	FILE* fp = fopen(fileName, "wt");
	for (size_t i = 0; i < cells_.size(); ++i)
	{
		fprintf(fp, "%d\r\n", i);
		saveText(*cells_[i], fp);
	}
	fclose(fp);
}