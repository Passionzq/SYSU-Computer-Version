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
	CImg<uchar> img; //Դͼ��
	CImg<uchar> grayscaled; // �Ҷ�ͼ
	CImg<uchar> gFiltered; // �ݶ�
	CImg<uchar> sFiltered; //Sobel����
	CImg<float> angles; //Angle Map
	CImg<uchar> non_max_sup; // ���������
	CImg<uchar> thres; //˫��ֵ
	CImg<uchar> edge;
public:
	Canny();
	Canny(string);
	~Canny();

	CImg<uchar> toGrayScale();
	vector<vector<double>> createFilter(int, int, double); //����һ��x*y��sigma��˹�˲���
	CImg<uchar> useFilter(vector<vector<double>>); //ʹ�ø�˹�˲���
	CImg<uchar> sobel(); //Sobel���Ӷ�Ӧ���˲���
	CImg<uchar> nonMaxSupp(); //���������
	CImg<uchar> threshold(int, int); //˫��ֵ���õ�����ͼƬ
	CImg<uchar> edgeTrack(CImg<uchar>);

};