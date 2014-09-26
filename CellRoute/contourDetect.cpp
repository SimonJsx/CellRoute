#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include <opencv2/imgproc/imgproc.hpp>


int main(int argc, char** argv)
{
	//����IplImageָ��  
	IplImage* pImg = cvLoadImage("C:/Users/nopend/Desktop/contour.png", CV_LOAD_IMAGE_GRAYSCALE);
	IplImage* pContourImg = NULL;
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = 0;
	CvSeq* contmax = 0;
	cvShowImage("src", pImg);

	//Ϊ������ʾͼ������ռ�  
	//3ͨ��ͼ���Ա��ò�ɫ��ʾ  
	pContourImg = cvCreateImage(cvGetSize(pImg), IPL_DEPTH_8U, 3);

	//copy source image and convert it to BGR image  
	cvCvtColor(pImg, pContourImg, CV_GRAY2BGR);
	
	//����contour  
	cvFindContours(pImg, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));

	//����������     
	cvDrawContours(pContourImg, contour, CV_RGB(255, 0, 0), CV_RGB(255, 0, 0), 2, 2, 8, cvPoint(0, 0));

	//��ȡ������ֵ  
	int area, maxArea = 0;
	for (; contour; contour = contour->h_next)
	{
		area = (int)fabs(cvContourArea(contour, CV_WHOLE_SEQ)); //��ȡ��ǰ�������  
		printf("area == %lf\n", area);
		if (area > maxArea)
		{
			contmax = contour;
			maxArea = area;
		}
	}
	CvRect aRect = cvBoundingRect(contmax, 0);
	//cvSetImageROI(pContourImg, aRect);
	
	//��ʾͼ��  
	cvShowImage("contour", pContourImg);
	cvSaveImage("contour.bmp", pContourImg);
	cvWaitKey(0);

	//���ٴ���  
	cvDestroyWindow("src");
	cvDestroyWindow("contour");
	
	//�ͷ�ͼ��  
	cvReleaseImage(&pImg);
	cvReleaseImage(&pContourImg);
	cvReleaseMemStorage(&storage);

	return 0;
}
