#pragma once

#include <iostream>
#include <algorithm>
#include <set>
#include <vector>
#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv/cv.h> 
#include <opencv2/ml/ml.hpp> 
#include "CImg.h"

// 二值化时的界限 hw6:156, hw5:192
#define BINARY_BOUND 156    

// 寻找数字：连通块的长度的下阈值
#define MIN_NUMBER_PIXELS 5

// 寻找数字：连通块的长度上阈值
#define MAX_NUMBER_PIXELS 400

// 寻找数字：包围住连通块的最小矩形的宽度的下阈值
#define MIN_NUMBER_WIDTH 5

// 寻找数字：包围住连通块的最小矩形的宽度的上阈值
#define MAX_NUMBER_WIDTH 30

// 寻找数字：包围住连通块的最小矩形的高度的下阈值
#define MIN_NUMBER_HEIGHT 4

// 寻找数字：包围住连通块的最小矩形的高度的上阈值
#define MAX_NUMBER_HEIGHT 25

// 寻找括号：连通块的长度的下阈值
#define MIN_BRACKET_LENGTH 100

// 寻找括号：连通块的长度的下阈值
#define MAX_BRACKET_LENGTH 800

// recognize_numbers中的flag值
#define KNN_FLAG 0
#define SVM_FLAG 1
#define ADABOOST_FLAG 2
#define XGBOOST_FLAG 3

typedef unsigned char uchar;

using namespace std;
using namespace cimg_library;
using namespace cv;
using namespace ml;

uchar white[] = {255,255,255};
uchar red[] = {255,0,0};
uchar green[] = {0,255,0};
uchar blue[] = {0,0,255};
uchar yellow[] = {255,255,0};
uchar light_red[] = {128,0,0};
uchar* my_color[] = {yellow,green,blue};

// two-pass寻找连通块方法中记录“单行连通块”id与坐标的结构体
struct Run {
    int id, row, start_col, end_col;

    Run(int _id = -1, int _row = 0, int _start_col = 0, int _end_col = 0)
    {
        id = _id, row = _row, start_col = _start_col, end_col = _end_col;
    }
};

// two-pass寻找连通块方法中记录等价id对的结构体
struct EqualPair {
    int id1, id2;

    EqualPair(int _id1, int _id2)
    {
        id1 = _id1, id2 = _id2;
    }

    friend bool operator < (const EqualPair& n1, const EqualPair& n2)
    {
        return n1.id1 < n2.id1 || n1.id2 < n2.id2;
    }
};

// 用来记录包围连通块/区块的最小矩形的坐标
struct Rectan{
    int pix_left, pix_right, pix_top, pix_down;

    Rectan(int a, int b, int c, int d){
        pix_left = a, pix_right = b, pix_top = c, pix_down = d;
    }
};

// 将图像转为二值图像 区分值为127
void to_binary_image(CImg<uchar>& img);

// 使用two-pass法找到满足条件（宏定义+长宽比）的连通块，并且返回这些连通块的坐标 （容器中元素是按id[+row]的顺序
vector<Rectan> find_nums_dots(CImg<uchar> img, CImg<uchar> &output, vector<Rectan> &nums_loc, vector<Rectan> &dots_loc);

// 将找到的连通块进行分区
vector<vector<Rectan>> cut(vector<Rectan>needed_Rectan,vector<Rectan> &range, CImg<uchar> &output);

// 选择不同的方法进行数字识别
void recognize_numbers(vector<Rectan> nums_loc, vector<Rectan> dots_loc, int flag = KNN_FLAG);

// 使用KNN对图像中的数字进行识别
Mat knn_recognize_nums(vector<Rectan> nums_loc, vector<Rectan> dots_loc);

// 使用SVM对图像中的数字进行识别
Mat svm_recognize_nums(vector<Rectan> nums_loc, vector<Rectan> dots_loc);

// 使用Adaboost对图像中的数字进行识别
Mat adaboost_recognize_nums(vector<Rectan> nums_loc, vector<Rectan> dots_loc);

// 寻找括号并将其对应坐标轴上的坐标注释出来。
void bracket(CImg<uchar> img);


bool cmp(const Run r1, const Run r2);
bool cmp2(const EqualPair& e1, const EqualPair& e2);
