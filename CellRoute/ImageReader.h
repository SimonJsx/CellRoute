#pragma once
#include <cv.h>

class ImageReader
{
public:
	ImageReader();
	~ImageReader();

	void autoThreshold();

	void clean();

	CvPoint* computeCoordinates();

	void guassianBlur();

	int getPointsNumbers()
	{
		return currentPointsNumbers_;
	}

	void openImage(char* fileName);
	void openImage(IplImage* ipl);

	void sobelSharp();

	void templateMast(IplImage *pTo,
		int nTempH, int nTempW,
		int nTempMY, int nTempMX, int *pfArray, int fCoef);
private:
	IplImage* imageHandle_;
	CvPoint* coordinates_;
	int currentPointsNumbers_;

	void addImages(IplImage* image1, IplImage* image2);
	void cutImage();
	int detectThreshold();
};

