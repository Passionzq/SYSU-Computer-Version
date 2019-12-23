#include <iostream>
#include <cmath>
#include <algorithm>
#include "CImg.h"
#include "Hough.h"
//#include "Canny.h"

#define CIRCLE_HOUGH_INTERVAL 1
#define THRESHLOW 20
#define THRESHHIGH 150
#define MIN_R  20	
#define MAX_R 100
#define DISTANCE 1

using namespace std;
using namespace cimg_library;


double dist(node a, node b){
    return sqrt(pow(a.x-b.x,2)+pow(a.y-b.y,2));
}

Hough::Hough(string filename) {
	img.load_bmp(filename.c_str());
    if(img.width()<700){
        img.resize_tripleXY();
    }
	edgeDetect(img);
	houghCircles(iEdge, MIN_R, MAX_R);
	
}

CImg<unsigned char> Hough::toGrayScale(CImg<unsigned char> src)
{
	CImg<unsigned char> grayscaled= CImg<unsigned char>(src.width(), src.height()); //To one channel
	cimg_forXY(img, x, y) {
		int r = src(x, y, 0);
		int g = src(x, y, 1);
		int b = src(x, y, 2);

		double newValue = (r * 0.2126 + g * 0.7152 + b * 0.0722);
		grayscaled(x, y) = newValue;
	}
	return grayscaled;
}

CImg<unsigned char> Hough::sobel(CImg<unsigned char> src) {
	int sobelX[3][3] = { { -1,0,1 },{ -2,0,2 },{ -1,0,1 } };
	int sobelY[3][3] = { { 1,2,1 },{ 0,0,0 },{ -1,-2,-1 } };
	int width = src.width();
	int height = src.height();
	CImg<unsigned char> newImg(width - 2, height - 2);
	newImg.fill(0);
	angles = CImg<unsigned char>(width - 2, height - 2);
	for (int i = 1; i < width - 1; i++) {
		for (int j = 1; j < height - 1; j++) {
			double sumX = 0.0;
			double sumY = 0.0;
			for (int row = 0; row < 3; row++) {
				for (int col = 0; col < 3; col++) {
					sumX += sobelX[row][col] * src(i - 1 + row, j - 1 + col);
					sumY += sobelY[row][col] * src(i - 1 + row, j - 1 + col);
				}
			}

			double sq2 = sqrt(sumX * sumX + sumY * sumY);
			if (sq2 > 255) //Unsigned Char Fix
				sq2 = 255;
			newImg(i - 1, j - 1) = sq2;

			if (sumX == 0) //Arctan Fix
				angles(i - 1, j - 1) = 90;
			else
				angles(i - 1, j - 1) = atan(sumY / sumX);

		}
	}
	return newImg;
}

CImg<unsigned char> Hough::nonMaxSupp(CImg<unsigned char> sFiltered)
{
	CImg<unsigned char> nonMaxSupped = CImg<unsigned char>(sFiltered.width(), sFiltered.height());
	for (int i = 1; i< sFiltered.width() - 1; i++) {
		for (int j = 1; j<sFiltered.height() - 1; j++) {
			float Tangent = angles(i, j);//??????????

			nonMaxSupped(i - 1, j - 1) = sFiltered(i, j);
			//Horizontal Edge
			if (((-22.5 < Tangent) && (Tangent <= 22.5)) || ((157.5 < Tangent) && (Tangent <= -157.5)))
			{
				if ((sFiltered(i, j) < sFiltered(i, j + 1)) || (sFiltered(i, j) < sFiltered(i, j - 1)))
					nonMaxSupped(i - 1, j - 1) = 0;
			}
			//Vertical Edge
			if (((-112.5 < Tangent) && (Tangent <= -67.5)) || ((67.5 < Tangent) && (Tangent <= 112.5)))
			{
				if ((sFiltered(i, j) < sFiltered(i + 1, j)) || (sFiltered(i, j) < sFiltered(i - 1, j)))
					nonMaxSupped(i - 1, j - 1) = 0;
			}

			//-45 Degree Edge
			if (((-67.5 < Tangent) && (Tangent <= -22.5)) || ((112.5 < Tangent) && (Tangent <= 157.5)))
			{
				if ((sFiltered(i, j) < sFiltered(i - 1, j + 1)) || (sFiltered(i, j) < sFiltered(i + 1, j - 1)))
					nonMaxSupped(i - 1, j - 1) = 0;
			}

			//45 Degree Edge
			if (((-157.5 < Tangent) && (Tangent <= -112.5)) || ((22.5 < Tangent) && (Tangent <= 67.5)))
			{
				if ((sFiltered(i, j) < sFiltered(i + 1, j + 1)) || (sFiltered(i, j) < sFiltered(i - 1, j - 1)))
					nonMaxSupped(i - 1, j - 1) = 0;
			}
		}
	}
	return nonMaxSupped;
}

CImg<unsigned char> Hough::threshold(CImg<unsigned char> imgin, int low, int high)
{
	if (low > 255)
		low = 255;
	if (high > 255)
		high = 255;

	CImg<unsigned char> EdgeMat = CImg<unsigned char>(imgin.width(), imgin.height());

	for (int i = 0; i<imgin.width(); i++)
	{
		for (int j = 0; j<imgin.height(); j++)
		{
			EdgeMat(i, j) = imgin(i, j);
			if (EdgeMat(i, j) > high)
				EdgeMat(i, j) = 255;
			else if (EdgeMat(i, j) < low)
				EdgeMat(i, j) = 0;
			else
			{
				bool anyHigh = false;
				bool anyBetween = false;
				for (int x = i - 1; x < i + 2; x++)
				{
					for (int y = j - 1; y<j + 2; y++)
					{
						if (x <= 0 || y <= 0 || EdgeMat.width() || y > EdgeMat.height()) //Out of bounds
							continue;
						else
						{
							if (EdgeMat(x, y) > high)
							{
								EdgeMat(i, j) = 255;
								anyHigh = true;
								break;
							}
							else if (EdgeMat(x, y) <= high && EdgeMat(x, y) >= low)
								anyBetween = true;
						}
					}
					if (anyHigh)
						break;
				}
				if (!anyHigh && anyBetween)
					for (int x = i - 2; x < i + 3; x++)
					{
						for (int y = j - 1; y<j + 3; y++)
						{
							if (x < 0 || y < 0 || x > EdgeMat.width() || y > EdgeMat.height()) //Out of bounds
								continue;
							else
							{
								if (EdgeMat(x, y) > high)
								{
									EdgeMat(i, j) = 255;
									anyHigh = true;
									break;
								}
							}
						}
						if (anyHigh)
							break;
					}
				if (!anyHigh)
					EdgeMat(i, j) = 0;
			}
		}
	}
	return EdgeMat;
}

CImg<unsigned char> Hough::edgeTrack(CImg<unsigned char> Edge)
{
	// 8邻域
	const Point directions[8] = { { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 },  { -1, -1 }, { 0, -1 },{ 1, -1 } };

	vector<Point> edge_t;
	vector<vector<Point>> edges;

	// 边缘跟踪
	int i, j, counts = 0, curr_d = 0;
	for (i = 1; i < Edge.width() - 1; i++) {
		for (j = 1; j < Edge.height() - 1; j++)
		{
			// 起始点及当前点
			Point b_pt = Point(i, j);
			Point c_pt = Point(i, j);

			// 如果当前点为前景点
			if (255 == Edge(c_pt.x, c_pt.y))
			{
				edge_t.clear();
				bool tra_flag = false;
				// 存入
				edge_t.push_back(c_pt);
				Edge(c_pt.x, c_pt.y) = 0;    // 用过的点直接给设置为0

				// 进行跟踪
				while (!tra_flag)
				{
					// 循环八次
					for (counts = 0; counts < 8; counts++)
					{
						// 防止索引出界
						if (curr_d >= 8)
						{
							curr_d -= 8;
						}
						if (curr_d < 0)
						{
							curr_d += 8;
						}

						// 当前点坐标
						// 跟踪的过程，应该是个连续的过程，需要不停的更新搜索的root点
						c_pt = Point(b_pt.x + directions[curr_d].x, b_pt.y + directions[curr_d].y);

						// 边界判断
						if ((c_pt.x > 0) && (c_pt.x < Edge.width() - 1) &&
							(c_pt.y > 0) && (c_pt.y < Edge.height() - 1))
						{
							// 如果存在边缘
							if (255 == Edge(c_pt.x, c_pt.y))
							{
								curr_d -= 2;   // 更新当前方向
								edge_t.push_back(c_pt);
								Edge(c_pt.x, c_pt.y) = 0;

								// 更新b_pt:跟踪的root点
								b_pt = c_pt;

								break;   // 跳出for循环
							}
						}
						curr_d++;
					}   // end for
					// 跟踪的终止条件：如果8邻域都不存在边缘
					if (8 == counts)
					{
						// 清零
						curr_d = 0;
						tra_flag = true;
						edges.push_back(edge_t);

						break;
					}

				}  // end if
			}  // end while

		}
	}

	CImg<unsigned char> trace_edge_color(Edge.width(), Edge.height(), 1, 1, 0);

	for (i = 0; i < edges.size(); i++)
	{
		// 过滤掉较小的边缘 
		if (edges[i].size() > DISTANCE)
		{
			for (j = 0; j < edges[i].size(); j++)
			{
				trace_edge_color(edges[i][j].x, edges[i][j].y) = 255;
			}
		}

	}

	return trace_edge_color;
}

CImg<unsigned char> Hough::edgeDetect(CImg<unsigned char> src){
	CImg<unsigned char> grayscaled = toGrayScale(src);
	CImg<unsigned char> blur = grayscaled.blur(1);
	CImg<unsigned char> sobleimg = sobel(blur);
	CImg<unsigned char> non = nonMaxSupp(sobleimg);
	iEdge = threshold(non, THRESHLOW, THRESHHIGH);
	iEdge = edgeTrack(iEdge);
	iEdge.save("edge.bmp");
	return iEdge;
}

void Hough::houghCircles(CImg<unsigned char>& img, int minR, int maxR)
{
	int  bestR = 0;
	CImg<unsigned char> houghS2;
	I2 = CImg<unsigned char>(iEdge.width(), iEdge.height(), 1, 3, 0);
    vector<CImg<unsigned char> > houghs((maxR-minR)/CIRCLE_HOUGH_INTERVAL);
    for (int i = 0; i < houghs.size(); i++){
        houghs[i] = CImg<unsigned char>(img.width(), img.height(), 1, 1, 0);
    }

	cimg_forXY(I2, x, y) {
		I2(x, y, 0) = iEdge(x, y);
		I2(x, y, 1) = iEdge(x, y);
		I2(x, y, 2) = iEdge(x, y);
	}
	for (int i = minR; i < maxR; i += CIRCLE_HOUGH_INTERVAL) {
        int radius = i;
		houghS2 = CImg<unsigned char>(img.width(), img.height(), 1, 1, 0);
		cimg_forXY(img, x, y) {
			if (img(x, y) == 255)
			{
				for (int j = 0; j < 360 ; j++)
				{
					int x0 = x - i * cos(cimg::PI * j / 180);
					int y0 = y - i * sin(cimg::PI * j / 180);
					if (x0 > 0 && x0 < img.width() && y0 > 0 && y0 < img.height()) {
						houghs[(i-minR)/CIRCLE_HOUGH_INTERVAL](x0, y0)++;
					}
				}
			}
		}
	}
    unsigned char red[3] = { 255, 0, 0 };
    unsigned char white[3] = {255,255,255};
    CImg<unsigned char> available(img.width(), img.height(), 1, 1, 0);
    for(int i = houghs.size()-1; i >= 0 ; i--){
        for(int x = 0 ; x < houghs[i].width(); x++){
            for(int y = 0; y < houghs[i].height(); y++){
                if((int)available(x,y,0,0) != 0){
                    continue;
                }
                if(houghs[i](x,y)/360.0 > 0.3){
                    circles.push_back(node(x,y,i*CIRCLE_HOUGH_INTERVAL+minR));
                    houghs[i].save("hough.bmp");
                    for(int a = 0 ; a < available.width(); a++){
                        for (int b = 0; b < available.height(); b++){
                            if ((int)available(a,b) != 0) continue;
                            if (dist(node(x,y,0),node(a,b,0)) < i*CIRCLE_HOUGH_INTERVAL+minR){
                                available(a,b) = (unsigned char)255;
                            }
                        }
                    }
                }
            }
        }
    }
    for(int i = 0 ; i < circles.size(); i++){
        I2.draw_circle(circles[i].x, circles[i].y, circles[i].value, red, 1.0f, 0xFFFFFFF);
    }
    cout << "圆形个数：" << circles.size()<<endl;
    I2.save("result.bmp");
}

