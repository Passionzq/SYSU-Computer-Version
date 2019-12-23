#pragma once

#include "CImg.h"
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <cassert>

#define M_PI acos(-1.0)
#define SIGMA 1
#define LOW_BOUND 40
#define HIGH_BOUND 100
#define DISTANCE 20

typedef unsigned char uchar;


using namespace std;
using namespace cimg_library;

struct Point {
	int x, y;
	
	Point(int _x, int _y) {
		x = _x;
		y = _y;
	}
};

class Canny {
private:
	CImg<uchar> img; //源图像
	CImg<uchar> grayscaled; // 灰度图
	CImg<uchar> gFiltered; // 梯度
	CImg<uchar> sFiltered; //Sobel算子
	CImg<float> angles; //Angle Map
	CImg<uchar> non_max_sup; // 非最大化抑制
	CImg<uchar> thres; //双阈值
	CImg<uchar> edge;
public:
	Canny();
	Canny(string);
	~Canny();

	CImg<uchar> toGrayScale();
	vector<vector<double>> createFilter(int, int, double); //返回一个x*y的sigma高斯滤波器
	CImg<uchar> useFilter(vector<vector<double>>); //使用高斯滤波器
	CImg<uchar> sobel(); //Sobel算子对应的滤波器
	CImg<uchar> nonMaxSupp(); //非最大化抑制
	CImg<uchar> threshold(int, int); //双阈值，得到最后的图片
	CImg<uchar> edgeTrack(CImg<uchar>);

};