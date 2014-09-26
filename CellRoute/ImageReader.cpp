#include "ImageReader.h"
#include <highgui.h>
#include <cxcore.h>
#include <vector>

using namespace std;

#define byte	unsigned char

ImageReader::ImageReader() :
	imageHandle_(nullptr),
	coordinates_(nullptr),
	currentPointsNumbers_(0)
{
}


ImageReader::~ImageReader()
{
	clean();
}

void ImageReader::autoThreshold()
{
	int thresd = detectThreshold();
	byte* temp = (byte*)(imageHandle_->imageData);
	for (int i = 0; i < imageHandle_->height; ++i)
	{
		for (int j = 0; j < imageHandle_->width; ++j)
		{
			if (temp[i * imageHandle_->width + j] < thresd)
				temp[i * imageHandle_->width + j] = 0;
			else
				temp[i * imageHandle_->width + j] = 255;
		}
	}
}

void ImageReader::clean()
{
	if (imageHandle_ != nullptr)
	{
		cvReleaseImage(&imageHandle_);
		imageHandle_ = nullptr;
	}
		
	if (coordinates_ != nullptr)
	{
		delete[] coordinates_;
		coordinates_ = nullptr;
	}
}

CvPoint* ImageReader::computeCoordinates()
{
	if (imageHandle_ == nullptr)
		return nullptr;

	if (coordinates_ == nullptr)
		coordinates_ = new CvPoint[500];

	int w = imageHandle_->width;
	int h = imageHandle_->height;

	guassianBlur();

	sobelSharp();

	autoThreshold();

	cutImage();

	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = 0;
	//查找contour  
	cvFindContours(imageHandle_, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
	
	CvSeq* head = contour;
	// 设置面积范围 
	int area, maxArea = 4500, minArea = 10;

	int curNum = 0;

	for (; contour; contour = contour->h_next)
	{
		area = (int)fabs(cvContourArea(contour, CV_WHOLE_SEQ)); //获取当前轮廓面积  
		if (area > minArea && area < maxArea)
		{
			CvRect aRect = cvBoundingRect(contour, 0);
			coordinates_[curNum].x = aRect.x + aRect.width / 2;
			coordinates_[curNum].y = aRect.y + aRect.height / 2;
			++curNum; 
		}
			
	}

	currentPointsNumbers_ = curNum;

	cvReleaseMemStorage(&storage);

	return coordinates_;
}

void ImageReader::guassianBlur()
{
	int guassianTemplate[9] = {
		1, 2, 1,
		2, 4, 2,
		1, 2, 1
	};
	int coefficient = 16;

	IplImage* temp = cvCreateImage(cvSize(imageHandle_->width, imageHandle_->height), 8, 1);
	templateMast(temp, 3, 3, 1, 1, guassianTemplate, coefficient);

	memcpy(imageHandle_->imageData, temp->imageData, temp->width * temp->height);

	cvReleaseImage(&temp);
}

void ImageReader::sobelSharp()
{
	IplImage* image1 = cvCreateImage(cvSize(imageHandle_->width, imageHandle_->height), 8, 1);
	IplImage* image2 = cvCreateImage(cvSize(imageHandle_->width, imageHandle_->height), 8, 1);
	// Sobel垂直边缘检测
	int Template_HSobel[9] = {
		-1, 0, 1,
		-2, 0, 2,
		-1, 0, 1 
	};

	// Sobel水平边缘检测
	int Template_VSobel[9] = {
		-1, -2, -1,
		0, 0, 0,
		1, 2, 1 
	};

	templateMast(image1, 3, 3, 1, 1, Template_HSobel, 1);
	templateMast(image2, 3, 3, 1, 1, Template_VSobel, 1);
	
	addImages(image1, image2);

	cvReleaseImage(&image1);
	cvReleaseImage(&image2);
}

void ImageReader::openImage(char* fileName)
{
	clean();

	IplImage* temp = cvLoadImage(fileName);

	imageHandle_ = cvCreateImage(cvSize(temp->width, temp->height), 8, 1);
	cvCvtColor(temp, imageHandle_, CV_BGR2GRAY);

	cvReleaseImage(&temp);
}

void ImageReader::openImage(IplImage* ipl)
{
	clean();

	imageHandle_ = cvCreateImage(cvSize(ipl->width, ipl->height), 8, 1);
	cvCvtColor(ipl, imageHandle_, CV_BGR2GRAY);
}

void ImageReader::templateMast(IplImage *pTo,
	int nTempH, int nTempW,
	int nTempMY, int nTempMX, int *pfArray, int fCoef)
{
	int width = imageHandle_->width;
	int height = imageHandle_->height;

	memset(pTo->imageData, 0, pTo->width * pTo->height); //目标图像初始化

	byte* temp = (byte*)(pTo->imageData);
	byte* tImage = (byte*)(imageHandle_->imageData);
	int i, j; //循环变量

	//扫描图像进行模板操作
	for (i = nTempMY; i<height - (nTempH - nTempMY) + 1; i++)
	{
		for (j = nTempMX; j<width - (nTempW - nTempMX) + 1; j++)
		{
			// (j,i)为中心点
			float fResult = 0;
			for (int k = 0; k<nTempH; k++)
			{
				for (int l = 0; l<nTempW; l++)
				{
					//计算加权和
					fResult += tImage[j + l - nTempMX + (i + k - nTempMY) * width] *
						pfArray[k * nTempW + l];
				}
			}

			// 乘以系数
			fResult /= fCoef;

			// 取正
			fResult = (float)fabs(fResult); //锐化时有可能出现负值

			byte b;
			if (fResult > 255)
				b = 255;
			else
				b = fResult + 0.5; //四舍五入

			temp[j + i * width] = b;
		}//for j
	}//for i
}

void ImageReader::addImages(IplImage* image1, IplImage* image2)
{
	//取得图像的高和宽
	int nHeight = imageHandle_->height;
	int nWidth = imageHandle_->width;

	int i, j;//循环变量

	//不能在CImg类对象中直接进行像素相加，因为相加的结果可能超过255
	vector< vector<int> > GrayMat;//求和后暂存图像的灰度点阵
	vector<int> vecRow(nWidth, 0); //GrayMat中的一行（初始化为0）
	for (i = 0; i<nHeight; i++)
	{
		GrayMat.push_back(vecRow);
	}

	//最大、最小灰度和值
	int nMax = 0;
	int nMin = 255 * 2;

	byte* temp1 = (byte*)(image1->imageData);
	byte* temp2 = (byte*)(image2->imageData);

	//逐行扫描图像
	for (i = 0; i<nHeight; i++)
	{
		for (j = 0; j<nWidth; j++)
		{
			//按位相加
			GrayMat[i][j] = temp1[i * nWidth + j] + temp2[i * nWidth + j];

			//统计最大、最小值
			if (GrayMat[i][j] > nMax)
				nMax = GrayMat[i][j];
			if (GrayMat[i][j] < nMin)
				nMin = GrayMat[i][j];
		}// j
	}// i


	//将GrayMat的取值范围重新归一化到[0, 255]
	int nSpan = nMax - nMin;

	byte* temp = (byte*)(imageHandle_->imageData);

	for (i = 0; i<nHeight; i++)
	{
		for (j = 0; j<nWidth; j++)
		{
			byte bt;
			if (nSpan > 0)
				bt = (GrayMat[i][j] - nMin) * 255 / nSpan;
			else if (GrayMat[i][j] <= 255)
				bt = GrayMat[i][j];
			else
				bt = 255;

			temp[i * nWidth + j] = bt;
		}// for j
	}// for i
}

void ImageReader::cutImage()
{
	IplImage* temp = cvCreateImage(cvSize(imageHandle_->width - 4, imageHandle_->height - 5), 8, 1);

	for (int i = 2; i < imageHandle_->width - 2; ++i)
	{
		for (int j = 3; j < imageHandle_->height - 2; ++j)
		{
			temp->imageData[(j - 3) * temp->width + (i - 2)] = imageHandle_->imageData[j * imageHandle_->width + i];
		}
	}

	cvReleaseImage(&imageHandle_);
	imageHandle_ = temp;
}

int ImageReader::detectThreshold()
{
	int nThreshold;

	int nDiffRet = 0;

	// 直方图数组
	int nHistogram[256] = { 0 };
	int i, j;

	byte bt;

	int nMin = 255;
	int nMax = 0;

	// 扫描图像,计算出最大、最小灰度和直方图
	for (j = 0; j < imageHandle_->height; j++)
	{
		for (i = 0; i<imageHandle_->width; i++)
		{
			bt = imageHandle_->imageData[j * imageHandle_->width + i];

			if (bt < nMin)
				nMin = bt;
			if (bt > nMax)
				nMax = bt;

			nHistogram[bt] ++;

		}
	}

	int nTotalGray = 0; //灰度值的和
	int nTotalPixel = 0; //像素数的和
	int nNewThreshold = (nMax + nMin) / 2; //初始阈值

	nDiffRet = nMax - nMin;

	if (nMax == nMin)
		nThreshold = nNewThreshold;

	else
	{
		nThreshold = 0;

		// 迭代开始,直到迭代次数达到100或新阈值与上一轮得到的阈值相等，迭代结束
		for (int nIterationTimes = 0; nThreshold != nNewThreshold && nIterationTimes < 100; nIterationTimes++)
		{
			nThreshold = nNewThreshold;
			nTotalGray = 0;
			nTotalPixel = 0;

			//计算图像中小于当前阈值部分的平均灰度
			for (i = nMin; i<nThreshold; i++)
			{
				nTotalGray += nHistogram[i] * i;
				nTotalPixel += nHistogram[i];
			}
			int nMean1GrayValue = nTotalGray / nTotalPixel;


			nTotalGray = 0;
			nTotalPixel = 0;

			//计算图像中大于当前阈值部分的平均灰度
			for (i = nThreshold + 1; i <= nMax; i++)
			{
				nTotalGray += nHistogram[i] * i;
				nTotalPixel += nHistogram[i];
			}
			int nMean2GrayValue = nTotalGray / nTotalPixel;

			nNewThreshold = (nMean1GrayValue + nMean2GrayValue) / 2; //计算出新的阈值
			nDiffRet = abs(nMean1GrayValue - nMean2GrayValue);
		}
	}

	return nThreshold;
}