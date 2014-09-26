#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include <opencv2/imgproc/imgproc.hpp>


int main(int argc, char** argv)
{
	//声明IplImage指针  
	IplImage* pImg = cvLoadImage("C:/Users/nopend/Desktop/contour.png", CV_LOAD_IMAGE_GRAYSCALE);
	IplImage* pContourImg = NULL;
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = 0;
	CvSeq* contmax = 0;
	cvShowImage("src", pImg);

	//为轮廓显示图像申请空间  
	//3通道图像，以便用彩色显示  
	pContourImg = cvCreateImage(cvGetSize(pImg), IPL_DEPTH_8U, 3);

	//copy source image and convert it to BGR image  
	cvCvtColor(pImg, pContourImg, CV_GRAY2BGR);
	
	//查找contour  
	cvFindContours(pImg, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));

	//将轮廓画出     
	cvDrawContours(pContourImg, contour, CV_RGB(255, 0, 0), CV_RGB(255, 0, 0), 2, 2, 8, cvPoint(0, 0));

	//获取面积最大值  
	int area, maxArea = 0;
	for (; contour; contour = contour->h_next)
	{
		area = (int)fabs(cvContourArea(contour, CV_WHOLE_SEQ)); //获取当前轮廓面积  
		printf("area == %lf\n", area);
		if (area > maxArea)
		{
			contmax = contour;
			maxArea = area;
		}
	}
	CvRect aRect = cvBoundingRect(contmax, 0);
	//cvSetImageROI(pContourImg, aRect);
	
	//显示图像  
	cvShowImage("contour", pContourImg);
	cvSaveImage("contour.bmp", pContourImg);
	cvWaitKey(0);

	//销毁窗口  
	cvDestroyWindow("src");
	cvDestroyWindow("contour");
	
	//释放图像  
	cvReleaseImage(&pImg);
	cvReleaseImage(&pContourImg);
	cvReleaseMemStorage(&storage);

	return 0;
}
