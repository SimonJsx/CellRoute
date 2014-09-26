#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <cmath>
#include <vector>
#include <iostream>

#include <time.h>

#include "CellRoute.h"
#include "CellManager.h"
#include <Windows.h>

#include "ImageReader.h"

//#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

using namespace std;
using namespace System;
using namespace System::Windows::Forms;

const int winHeight = 600;
const int winWidth = 800;
CvPoint mousePosition = cvPoint(winWidth >> 1, winHeight >> 1);
//mouse event callback
void mouseEvent(int event, int x, int y, int flags, void *param)
{
	if (event == CV_EVENT_MOUSEMOVE) {
		mousePosition = cvPoint(x, y);
	}
}

[STAThreadAttribute]
int main(void)
{
#if 0
	//1.kalman filter setup
	const int stateNum = 4;
	const int measureNum = 2;

	CvKalman* kalman = cvCreateKalman(stateNum, measureNum, 0);//state(x,y,detaX,detaY)
	CvMat* process_noise = cvCreateMat(stateNum, 1, CV_32FC1);
	CvMat* measurement = cvCreateMat(measureNum, 1, CV_32FC1);//measurement(x,y)
	CvRNG rng = cvRNG(-1);
	float A[stateNum][stateNum] = {//transition matrix
		1, 0, 1, 0,
		0, 1, 0, 1,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	memcpy(kalman->transition_matrix->data.fl, A, sizeof(A));
	cvSetIdentity(kalman->measurement_matrix, cvRealScalar(1));
	cvSetIdentity(kalman->process_noise_cov, cvRealScalar(1e-5));
	cvSetIdentity(kalman->measurement_noise_cov, cvRealScalar(1e-1));
	cvSetIdentity(kalman->error_cov_post, cvRealScalar(1));
	//initialize post state of kalman filter at random
	cvRandArr(&rng, kalman->state_post, CV_RAND_UNI, cvRealScalar(0), cvRealScalar(winHeight>winWidth ? winWidth : winHeight));
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_SCRIPT_COMPLEX, 1, 1);
	cvNamedWindow("kalman");
	cvSetMouseCallback("kalman", mouseEvent);
	IplImage* img = cvCreateImage(cvSize(winWidth, winHeight), 8, 3);
	while (1){
		clock_t start = clock();
		//2.kalman prediction
		const CvMat* prediction = cvKalmanPredict(kalman, 0);
		CvPoint predict_pt = cvPoint((int)prediction->data.fl[0], (int)prediction->data.fl[1]);
		//3.update measurement
		measurement->data.fl[0] = (float)mousePosition.x;
		measurement->data.fl[1] = (float)mousePosition.y;
		//4.update
		cvKalmanCorrect(kalman, measurement);
		//draw 
		cvSet(img, cvScalar(255, 255, 255, 0));
		cvCircle(img, predict_pt, 5, CV_RGB(0, 255, 0), 3);//predicted point with green
		cvCircle(img, mousePosition, 5, CV_RGB(255, 0, 0), 3);//current position with red
		char buf[256];
		sprintf_s(buf, 256, "predicted position:(%3d,%3d)", predict_pt.x, predict_pt.y);
		cvPutText(img, buf, cvPoint(10, 30), &font, CV_RGB(0, 0, 0));
		sprintf_s(buf, 256, "current position :(%3d,%3d)", mousePosition.x, mousePosition.y);
		cvPutText(img, buf, cvPoint(10, 60), &font, CV_RGB(0, 0, 0));
		clock_t end = clock();
	//	printf("%d", end - start);
		printf("(%d, %d)\n", predict_pt.x, predict_pt.y);
		cvShowImage("kalman", img);
		int key = cvWaitKey(300);
		if (key == 27){//esc   
			break;
		}
	}
	cvReleaseImage(&img);
	cvReleaseKalman(&kalman);
#else
	System::Windows::Forms::OpenFileDialog^ fileOpen = gcnew System::Windows::Forms::OpenFileDialog();
	fileOpen->Multiselect = false;
	if (fileOpen->ShowDialog() != System::Windows::Forms::DialogResult::OK)
	{
		return 0;
	}
	char* fileName = (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi((System::String^)fileOpen->FileName);
	CvCapture* capture = cvCaptureFromAVI(fileName);
	IplImage* frame = cvQueryFrame(capture);

	CellManager cm;
	ImageReader readImage;

	int round = 0;

	while (frame)
	{		
		clock_t start = clock();

		readImage.openImage(frame);
		
		CvPoint* pointArr = readImage.computeCoordinates();
		
		//2.kalman prediction
		//3.update measurement
		//4.update
		cm.addCoordinates(pointArr, readImage.getPointsNumbers());

		clock_t end = clock();
		printf("%d\n", end - start);
		
		
	//	cvShowImage("1", frame);
	//	cvWaitKey(10);

		frame = cvQueryFrame(capture);
	}
	cm.saveRouteBmp("d:\\1.bmp", cvSize(736, 544));
	cm.saveRoutText("d:\\1.txt");

	System::Runtime::InteropServices::Marshal::FreeHGlobal(System::IntPtr(fileName));
	getchar();
#endif
	return 0;
}
