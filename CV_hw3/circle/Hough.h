#ifndef _hough_
#define _hough_
#include "CImg.h"
#include <vector>

using namespace std;
using namespace cimg_library;

struct node {
    int x, y, value;
	node(){
		x = 0;
		y = 0;
		value = 0;
	}
	node(int a,int b,int c){
		x = a;
		y = b;
		value = c;
	}
    bool operator < (const node &a)const
    {
        return value < a.value;
    }
};

struct Point {
	int x, y;
	
	Point(int _x, int _y) {
		x = _x;
		y = _y;
	}
};


class Hough {
private:
	CImg<unsigned char> img;
	CImg<unsigned char> iEdge;
	CImg<unsigned char> angles;
	CImg<unsigned char> houghS;


	CImg<unsigned char> I2; //line image


	vector<node> circles;
public :
	Hough(string filename);
	void houghCircles(CImg<unsigned char>& img, int minR, int maxR);

	// tools
	CImg<unsigned char> toGrayScale(CImg<unsigned char> src);
	CImg<unsigned char> edgeDetect(CImg<unsigned char>); 
	CImg<unsigned char> sobel(CImg<unsigned char>);
	CImg<unsigned char> nonMaxSupp(CImg<unsigned char>); 
	CImg<unsigned char> threshold(CImg<unsigned char> imgin, int low, int high);
	CImg<unsigned char> edgeTrack(CImg<unsigned char> Edge);
};



#endif