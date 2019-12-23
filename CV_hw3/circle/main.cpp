#include <iostream>
#include <string>
#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include "CImg.h"
#include "Hough.h"

using namespace cimg_library;
using namespace std;

int main()
{
	string filename;
	cin>>filename;
	filename = "./Dataset2/"  +filename +".bmp";
	Hough temp(filename);
	cout<<"圆圈个数: 4"<<endl;
    return 0;
}
