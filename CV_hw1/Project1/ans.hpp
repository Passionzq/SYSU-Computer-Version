#include <iostream>
#include <cmath>
#include <string>
#include "CImg.h"

#define uchar unsigned char
#define pi 3.14159

using namespace cimg_library;

class Ans {
public:
	void print_message();
	bool choose(CImg<uchar>& img);

private:
	void ans1(CImg<uchar>& img);
	void ans2(CImg<uchar>& img);
	void ans3(CImg<uchar>& img);
	void ans4(CImg<uchar>& img);
	void ans5(CImg<uchar>& img);
	void ans6(CImg<uchar>& img);
	
};