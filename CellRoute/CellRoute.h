#pragma once
#include <cv.h>
#include <cxcore.h>
struct PointRoute
{
	int x;
	int y;
	struct PointRoute* next;
};

class CellRoute
{
public:
	static const int STATE_NUMBER = 4;
	static const int MEASURE_NUMBER = 2;

	CellRoute(int x, int y);
	~CellRoute();

	// 增加一个路径坐标，并且更新kalman
	void addRoute(int x, int y)
	{
		PointRoute* tempRoute = route_;
		if (tempRoute != nullptr)
		{
			while (tempRoute->next != nullptr)
			{
				tempRoute = tempRoute->next;
			}
			tempRoute->next = new PointRoute;
			tempRoute->next->x = x;
			tempRoute->next->y = y;
			tempRoute->next->next = nullptr;
		}
		else
		{
			route_ = new PointRoute;
			route_->x = x;
			route_->y = y;
			route_->next = nullptr;
		}

		++routeCount_;
		measurement_->data.fl[0] = static_cast<float>(x);
		measurement_->data.fl[1] = static_cast<float>(y);
		update(measurement_);
	}
	// 清除位置点信息以及kalman滤波器
	void clear();

	int getRouteNumber()
	{
		return routeCount_;
	}

	// 初始化kalmanFilter
	void init(int x, int y);

	// 预测下次坐标
	CvPoint& predict();

	// 使状态稳定
	void statusStable();

	// 修正参数
	void update(CvMat* measure);

private:
	PointRoute* route_;
	CvPoint predictPoint_;
	CvMat* measurement_;
	CvKalman* predictMethod_;
	int routeCount_;

	friend void saveBmp(CellRoute& cr, IplImage* image, CvScalar color);
	friend void saveText(CellRoute& cr, FILE* fp);
	friend class CellManager;
};

