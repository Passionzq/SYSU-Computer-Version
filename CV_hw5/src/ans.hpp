#pragma once

#include <iostream>
#include <algorithm>
#include <set>
#include <vector>
#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "./../CImg.h"

#define MIN_NUMBER_PIXELS 5
#define MAX_NUMBER_PIXELS 300
#define MIN_NUMBER_WIDTH 6
#define MAX_NUMBER_WIDTH 25
#define MIN_NUMBER_HEIGHT 6
#define MAX_NUMBER_HEIGHT 25
#define MIN_BRACKET_LENGTH 100
#define MAX_BRACKET_LENGTH 800

typedef unsigned char uchar;

using namespace std;
using namespace cimg_library;
using namespace cv;
using namespace ml;

struct Run {
    int id, row, start_col, end_col;

    Run(int _id = -1, int _row = 0, int _start_col = 0, int _end_col = 0)
    {
        id = _id, row = _row, start_col = _start_col, end_col = _end_col;
    }
};

struct EqualPair
{
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

struct Rectan{
    int pix_left, pix_right, pix_top, pix_down;

    Rectan(int a, int b, int c, int d){
        pix_left = a, pix_right = b, pix_top = c, pix_down = d;
    }
};

void to_binary_image(CImg<uchar>& img);
vector<Rectan> find_nums_dots(CImg<uchar> img, CImg<uchar> &output, vector<Rectan> &nums_loc, vector<Rectan> &dots_loc);
vector<vector<Rectan>> cut(vector<Rectan>needed_Rectan,vector<Rectan> &range, CImg<uchar> &output);
Mat recognize_numbers(vector<Rectan> num_loc, vector<Rectan> dots_loc);
void bracket(CImg<uchar> img);
bool cmp(const Run r1, const Run r2);
bool cmp2(const EqualPair& e1, const EqualPair& e2);
