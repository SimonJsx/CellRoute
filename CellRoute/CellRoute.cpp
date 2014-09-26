#include "CellRoute.h"


CellRoute::CellRoute(int x, int y)
	: routeCount_(0),
	  route_(nullptr)
{
	init(x, y);
	measurement_ = cvCreateMat(MEASURE_NUMBER, 1, CV_32FC1);
}


CellRoute::~CellRoute()
{
	clear();
}

void CellRoute::init(int x, int y)
{
	predictMethod_ = cvCreateKalman(STATE_NUMBER, MEASURE_NUMBER, 0);

	float A[STATE_NUMBER][STATE_NUMBER] = 
	{//transition matrix
		1, 0, 1, 0,
		0, 1, 0, 1,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	memcpy(predictMethod_->transition_matrix->data.fl, A, sizeof(A));

	cvSetIdentity(predictMethod_->measurement_matrix);
	cvSetIdentity(predictMethod_->process_noise_cov, cvRealScalar(1e-5));
	cvSetIdentity(predictMethod_->measurement_noise_cov, cvRealScalar(1e-1));
	cvSetIdentity(predictMethod_->error_cov_post, cvRealScalar(1));

	// set initial state, that cell is locate at (x, y)
	predictMethod_->state_post->data.fl[0] = x;
	predictMethod_->state_post->data.fl[1] = y;
	predictMethod_->state_post->data.fl[2] = 0;
	predictMethod_->state_post->data.fl[3] = 0;

	predictMethod_->state_pre->data.fl[0] = x;
	predictMethod_->state_pre->data.fl[1] = y;
	predictMethod_->state_pre->data.fl[2] = 0;
	predictMethod_->state_pre->data.fl[3] = 0;
}

void CellRoute::clear()
{
	cvReleaseKalman(&predictMethod_);
	cvReleaseMat(&measurement_);
	PointRoute* tempRoute = route_;
	while (tempRoute != nullptr)
	{
		route_ = route_->next;
		delete tempRoute;
		tempRoute = route_;
	}
	route_ = nullptr;
	routeCount_ = 0;
}

CvPoint& CellRoute::predict()
{
	const CvMat* pt = cvKalmanPredict(predictMethod_, 0);
	predictPoint_.x = pt->data.fl[0];
	predictPoint_.y = pt->data.fl[1];
	return predictPoint_;
}

void CellRoute::statusStable()
{
	// it does not cost time, for predict and post state are the same after initial
	while (1)
	{
		CvPoint& pt = predict();
		if (pt.x - measurement_->data.fl[0] == 0 && pt.y == measurement_->data.fl[1])
		{
			break;
		}
		else
		{
			update(measurement_);
		}			
	}
}

void CellRoute::update(CvMat* measure)
{
	cvKalmanCorrect(predictMethod_, measure);
}

void saveBmp(CellRoute& cr, IplImage* image, CvScalar color)
{
	PointRoute* tempRoute = cr.route_;
	CvPoint pt1, pt2;

	pt1.x = tempRoute->x;
	pt1.y = tempRoute->y;

	while (tempRoute->next != nullptr)
	{
		tempRoute = tempRoute->next;
		pt2.x = tempRoute->x;
		pt2.y = tempRoute->y;
		cvLine(image, pt1, pt2, color, 3);

		pt1 = pt2;
	}
}

void saveText(CellRoute& cr, FILE* fp)
{
	PointRoute* tempRoute = cr.route_;
	CvPoint pt1, pt2;

	pt1.x = tempRoute->x;
	pt1.y = tempRoute->y;

	for (int i = 0; i < cr.routeCount_; i++)
	{
		if (tempRoute == nullptr)
		{
			break;
		}
		fprintf(fp, "(%d, %d)\r\n", tempRoute->x, tempRoute->y);
		tempRoute = tempRoute->next;
	}
}