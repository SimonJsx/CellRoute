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
	//����contour  
	cvFindContours(imageHandle_, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
	
	CvSeq* head = contour;
	// ���������Χ 
	int area, maxArea = 4500, minArea = 10;

	int curNum = 0;

	for (; contour; contour = contour->h_next)
	{
		area = (int)fabs(cvContourArea(contour, CV_WHOLE_SEQ)); //��ȡ��ǰ�������  
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
	// Sobel��ֱ��Ե���
	int Template_HSobel[9] = {
		-1, 0, 1,
		-2, 0, 2,
		-1, 0, 1 
	};

	// Sobelˮƽ��Ե���
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

	memset(pTo->imageData, 0, pTo->width * pTo->height); //Ŀ��ͼ���ʼ��

	byte* temp = (byte*)(pTo->imageData);
	byte* tImage = (byte*)(imageHandle_->imageData);
	int i, j; //ѭ������

	//ɨ��ͼ�����ģ�����
	for (i = nTempMY; i<height - (nTempH - nTempMY) + 1; i++)
	{
		for (j = nTempMX; j<width - (nTempW - nTempMX) + 1; j++)
		{
			// (j,i)Ϊ���ĵ�
			float fResult = 0;
			for (int k = 0; k<nTempH; k++)
			{
				for (int l = 0; l<nTempW; l++)
				{
					//�����Ȩ��
					fResult += tImage[j + l - nTempMX + (i + k - nTempMY) * width] *
						pfArray[k * nTempW + l];
				}
			}

			// ����ϵ��
			fResult /= fCoef;

			// ȡ��
			fResult = (float)fabs(fResult); //��ʱ�п��ܳ��ָ�ֵ

			byte b;
			if (fResult > 255)
				b = 255;
			else
				b = fResult + 0.5; //��������

			temp[j + i * width] = b;
		}//for j
	}//for i
}

void ImageReader::addImages(IplImage* image1, IplImage* image2)
{
	//ȡ��ͼ��ĸߺͿ�
	int nHeight = imageHandle_->height;
	int nWidth = imageHandle_->width;

	int i, j;//ѭ������

	//������CImg�������ֱ�ӽ���������ӣ���Ϊ��ӵĽ�����ܳ���255
	vector< vector<int> > GrayMat;//��ͺ��ݴ�ͼ��ĻҶȵ���
	vector<int> vecRow(nWidth, 0); //GrayMat�е�һ�У���ʼ��Ϊ0��
	for (i = 0; i<nHeight; i++)
	{
		GrayMat.push_back(vecRow);
	}

	//�����С�ҶȺ�ֵ
	int nMax = 0;
	int nMin = 255 * 2;

	byte* temp1 = (byte*)(image1->imageData);
	byte* temp2 = (byte*)(image2->imageData);

	//����ɨ��ͼ��
	for (i = 0; i<nHeight; i++)
	{
		for (j = 0; j<nWidth; j++)
		{
			//��λ���
			GrayMat[i][j] = temp1[i * nWidth + j] + temp2[i * nWidth + j];

			//ͳ�������Сֵ
			if (GrayMat[i][j] > nMax)
				nMax = GrayMat[i][j];
			if (GrayMat[i][j] < nMin)
				nMin = GrayMat[i][j];
		}// j
	}// i


	//��GrayMat��ȡֵ��Χ���¹�һ����[0, 255]
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

	// ֱ��ͼ����
	int nHistogram[256] = { 0 };
	int i, j;

	byte bt;

	int nMin = 255;
	int nMax = 0;

	// ɨ��ͼ��,����������С�ҶȺ�ֱ��ͼ
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

	int nTotalGray = 0; //�Ҷ�ֵ�ĺ�
	int nTotalPixel = 0; //�������ĺ�
	int nNewThreshold = (nMax + nMin) / 2; //��ʼ��ֵ

	nDiffRet = nMax - nMin;

	if (nMax == nMin)
		nThreshold = nNewThreshold;

	else
	{
		nThreshold = 0;

		// ������ʼ,ֱ�����������ﵽ100������ֵ����һ�ֵõ�����ֵ��ȣ���������
		for (int nIterationTimes = 0; nThreshold != nNewThreshold && nIterationTimes < 100; nIterationTimes++)
		{
			nThreshold = nNewThreshold;
			nTotalGray = 0;
			nTotalPixel = 0;

			//����ͼ����С�ڵ�ǰ��ֵ���ֵ�ƽ���Ҷ�
			for (i = nMin; i<nThreshold; i++)
			{
				nTotalGray += nHistogram[i] * i;
				nTotalPixel += nHistogram[i];
			}
			int nMean1GrayValue = nTotalGray / nTotalPixel;


			nTotalGray = 0;
			nTotalPixel = 0;

			//����ͼ���д��ڵ�ǰ��ֵ���ֵ�ƽ���Ҷ�
			for (i = nThreshold + 1; i <= nMax; i++)
			{
				nTotalGray += nHistogram[i] * i;
				nTotalPixel += nHistogram[i];
			}
			int nMean2GrayValue = nTotalGray / nTotalPixel;

			nNewThreshold = (nMean1GrayValue + nMean2GrayValue) / 2; //������µ���ֵ
			nDiffRet = abs(nMean1GrayValue - nMean2GrayValue);
		}
	}

	return nThreshold;
}