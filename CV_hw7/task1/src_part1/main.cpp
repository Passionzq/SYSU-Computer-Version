#include <iostream>
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <vector>
#include <string>

#define INF 10000

using namespace std;
using namespace cv;

bool cmp(vector<Point> a, vector<Point> b)
{
    return a.size() > b.size();
}

bool cmp2(Point a, Point b)
{
    if (a.y < b.y){
        return true;
    }
    else if (a.y == b.y){
        if(a.x <= b.x){
            return true;
        }
        return false;
    }
    return false;
}

// 输入的图片应置于“../DataSet/”路径下，文件名应为[1,2,3...].jpg
int main()
{
    // int low, high;
    // cin>> low >> high;
    for(int i = 1; i < 22; ++i){
        string path = "../DataSet/" + to_string(i);
        string ans_path = "../ans_pic/" + to_string(i);
        Mat image, imageSource=imread(path+".jpg");

        resize(imageSource,imageSource,Size(imageSource.cols/3,imageSource.rows/3)); 

        Mat img_src = imageSource.clone();

        int spatialRad = 30;        //空间窗口大小  
        int colorRad = 30;          //色彩窗口大小  
        int maxPyrLevel = 2;        //金字塔层数  
        pyrMeanShiftFiltering(imageSource, image, spatialRad, colorRad, maxPyrLevel); //色彩聚类平滑滤波  

        cvtColor(image,image,CV_RGB2GRAY);
        
        Canny(image,image,50,230,3,true);
        // imwrite(ans_path+"_canny.jpg", image);

        int dilation_size = 1;
        Mat element = getStructuringElement(   MORPH_ELLIPSE,
                       Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                       Point( dilation_size, dilation_size ) );
        dilate(image,image,element);
        
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(image,contours,hierarchy,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE,Point());
        Mat imageContours=Mat::zeros(image.size(),CV_8UC1);
        Mat Contours=Mat::zeros(image.size(),CV_8UC1);  //绘制
        sort(contours.begin(), contours.end(), cmp);
        drawContours(imageContours,contours,0,Scalar(255),1,8,hierarchy);
        // imwrite(ans_path + "_contour.jpg", imageContours);

        Mat hough_img = imageContours;
        vector<Vec2f> lines;//定义一个矢量结构lines用于存放得到的线段矢量集合  
        HoughLines(hough_img, lines, 1, CV_PI/180, 120, 0, 0 );  
        //【4】依次在图中绘制出每条线段  
        vector<pair<double,double>> line_kb;
        for( size_t j = 0; j < lines.size(); j++ )  
        {  
            float rho = lines[j][0], theta = lines[j][1];  
            Point pt1, pt2;  
            double a = cos(theta), b = sin(theta);  
            double x0 = a*rho, y0 = b*rho;  
            pt1.x = cvRound(x0 + 2000*(-b));  
            pt1.y = cvRound(y0 + 2000*(a));  
            pt2.x = cvRound(x0 - 2000*(-b));  
            pt2.y = cvRound(y0 - 2000*(a));  

            // line( imageSource, pt1, pt2, Scalar(0,0,255), 2, CV_AA);

            double y1 = static_cast<double>(pt1.y), x1 = static_cast<double>(pt1.x);
            double y2 = static_cast<double>(pt2.y), x2 = static_cast<double>(pt2.x);
            if(abs(x1-x2)>1e-10)
                line_kb.push_back(make_pair((y2-y1)/(x2-x1), (y2*x1-y1*x2)/(x1-x2)));
            else
                line_kb.push_back(make_pair(double(INF), x1));
            
        }

        vector<Point> intersections;
        for (int n = 0; n < line_kb.size(); ++n) {
            double k1 = line_kb[n].first, b1 = line_kb[n].second;
            for (int j = n + 1; j < line_kb.size(); ++j) {
                double k2 = line_kb[j].first, b2 = line_kb[j].second;
                double x, y;
                if(k1 == INF && k2 == INF) continue;
                if(k1 == INF){
                    x = b1;
                    y = k2 * x + b2;
                }
                else if (k2 == INF){
                    x = b2;
                    y = k1 * x + b1;
                }
                else {
                    x = (b1 - b2) / (k2 - k1); 
                    y = (k2*b1 - k1*b2) / (k2-k1);
                }
                if(x>0 && x < imageSource.cols && y >0 && y < imageSource.rows)
                    intersections.push_back(Point(cvRound(x),cvRound(y)));
            }
        }
        
        sort(intersections.begin(), intersections.end(),cmp2);
        // std::cout<<i<<"\t"<<(int)lines.size()<<"\t"<<(int)intersections.size()<<endl;  
        // for(int j = 0; j < intersections.size(); ++j) {
        //     cout<<intersections[j].x<<", "<<intersections[j].y<<"\t";
        // }
        // cout<<endl<<endl;
        
        Point left_top(0,0), right_top(0,0), left_down(0,0), right_down(0,0);

        
        for(int j = 0; j < intersections.size(); ++j) {
            Point p = intersections[j];
            if(abs(p.x - imageSource.cols / 2) > imageSource.cols / 5 
            && abs(p.y - imageSource.rows / 2) > imageSource.rows / 15){
                if(p.x < imageSource.cols / 2 && p.y < imageSource.rows/2){
                    if(left_top!= Point(0,0)){
                        left_top = p.x > left_top.x || p.y > left_top.y ? left_top : p;
                    }
                    else {
                        left_top = p; 
                    }
                }
                if(p.x < imageSource.cols / 2 && p.y > imageSource.rows/2){
                    if(left_down!= Point(0,0)){
                        left_down = p.x > left_down.x || p.y < left_down.y ? left_down : p;
                    }
                    else {
                        left_down = p;
                    } 
                }
                if(p.x > imageSource.cols / 2 && p.y < imageSource.rows/2){
                    if(right_top!= Point(0,0)){
                        right_top = p.x < right_top.x || p.y > right_top.y ? right_top : p;
                    }
                    else {
                        right_top = p; 
                    }
                }
                if(p.x > imageSource.cols / 2 && p.y > imageSource.rows/2){
                    if(right_down!= Point(0,0)){
                        right_down = p.x < right_down.x || p.y < right_down.y ? right_down : p;
                    }
                    else {
                        right_down = p; 
                    }
                }
            }
        }
        
        //     cout<<left_top.x<<", "<<left_top.y<<"\t"
        //         <<right_top.x<<", "<<right_top.y<<"\t"
        //         <<left_down.x<<", "<<left_down.y<<"\t"
        //         <<right_down.x<<", "<<right_down.y<<"\n";
        // circle(imageSource, left_top,3,Scalar(0,255,0),2);
        // circle(imageSource, left_down,3,Scalar(0,255,0),2);
        // circle(imageSource, right_top,3,Scalar(0,255,0),2);
        // circle(imageSource, right_down,3,Scalar(0,255,0),2);
        // imwrite(ans_path+"_hough.jpg",imageSource);

        int len = 500, high = 300;  // 变换后名片的大小
        Mat draw(Size(len,high), imageSource.type());
        Point2f src[4] = {left_top,right_top, left_down, right_down};
        Point2f dst[4] = {Point(0,0), Point(len-1,0),Point(0,high-1), Point(len-1,high-1)};
        Mat warpMatrix = getPerspectiveTransform(src,dst);
        warpPerspective(imageSource, draw, warpMatrix, draw.size(), INTER_LINEAR, BORDER_CONSTANT);
        imwrite(ans_path+"_ans1.jpg",draw);
    }
    return 0;
}