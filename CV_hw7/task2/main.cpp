#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <tesseract/baseapi.h>

using namespace std;
using namespace cv;
using namespace cimg;

string file_name;

vector<vector<Point>> dilate_contours(Mat img, int x, int y);
void save_single(Mat img, int num);
void recoginze_phone(Mat img, int num);
void recognize_name(Mat img, int num);

int main()
{
    for (int num = 1; num <= 21; ++num) {
        file_name = "./data/" + to_string(num);
        Mat src_img = imread(file_name+".jpg");

        // 图像预处理
        Mat gray_img, binary_img;
        cvtColor(src_img,gray_img,COLOR_RGB2GRAY);
        adaptiveThreshold(gray_img, binary_img, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 9, 5);

        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        
        findContours(binary_img, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_NONE, Point());
        for (int i = 0; i < contours.size(); ++i) {
            if(contourArea(contours[i]) < 6) {
                drawContours(binary_img, contours, i, Scalar(0), FILLED);
            }
        }

        // imwrite(to_string(num)+"_bin1.jpg",binary_img);  
        
        // ------Part1------
        // Mat test = binary_img.clone();
        // save_single(test, num);
        

        // ------Part2------
        Mat test2 = binary_img.clone();
        recoginze_phone(test2, num);
        recognize_name(test2, num);
        
    }
}

vector<vector<Point>> dilate_contours(Mat img, int x, int y)
{
    Mat temp, element = getStructuringElement( MORPH_ELLIPSE, Size(x,y) );
    dilate(img,temp,element);
    // imshow("",temp);
    // waitKey(0);
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(temp, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point());
    return contours;
}

// 垂直投影分割
void save_single(Mat img, int num)
{
    vector<vector<Point>>cnts = dilate_contours(img, 5, 3);
    for(int i = 0 ; i < cnts.size(); ++i) {
        Mat sub_img;
        if(contourArea(cnts[i]) > 50 && contourArea(cnts[i]) < 2000){
            Rect temp = boundingRect(cnts[i]);
            if(temp.height / temp.width > 3 || temp.width / temp.height > 3)    continue;
            if(temp.x > 0)  temp.x -= 1;
            if(temp.y > 0)  temp.y -= 1;
            if(temp.x + temp.width < img.cols - 2) temp.width += 2;
            if(temp.y + temp.height < img.rows -2) temp.height += 2;
            sub_img = img(temp);
            // imwrite("./ans_pic/part1/"+to_string(num)+"_"+to_string(i)+".jpg",sub_img);
        }
        else
        {
            continue;
        }
        
                   
        int* hist = new int[sub_img.cols];
        
        for(int j = 0; j < sub_img.cols; ++j)   hist[j] = 0;

        for(int x = 0; x < sub_img.rows; ++x){
            for(int y = 0; y < sub_img.cols; ++y){
                if(sub_img.at<uchar>(x,y)>128){
                    hist[y]++;
                }
            }
        }

        int left = 0, right = 0;
        bool white = false;
        int count = 0;
        
        for(int j = 0; j < sub_img.cols; ++j) {
            if(hist[j]){
                white = true;
                if (j == sub_img.cols - 1){
                    right = j;
                    Rect r(left,0,right-left,sub_img.rows);
                    imwrite("./ans_pic/part1/"+to_string(num)+"_" + to_string(i) + "_"+to_string(count)+".jpg",sub_img(r));
                    count++;
                }
            }
            else {
                if(white){
                    right = j;
                    Rect r(left,0,right-left,sub_img.rows);
                    imwrite("./ans_pic/part1/"+to_string(num)+"_" + to_string(i) + "_"+to_string(count)+".jpg",sub_img(r));
                    count++;

                }
                left = j;
                white = false;
            }
        }
        delete[] hist;
    }
}

void recoginze_phone(Mat img, int num)
{
    vector<vector<Point>> cnts = dilate_contours(img,21,3);
    Mat sub_img;
    for(int i = 0; i < cnts.size(); ++i){
        Rect temp = boundingRect(cnts[i]);
        if(temp.width / temp.height >=5 && temp.width / temp.height <= 8 && temp.height > 15){    
            if(temp.x > 0)  temp.x -= 1;
            if(temp.y > 0)  temp.y -= 1;
            if(temp.x + temp.width < img.cols - 2) temp.width += 2;
            if(temp.y + temp.height < img.rows -2) temp.height += 2;
            sub_img = img(temp);
            imwrite(to_string(num)+"_"+to_string(i)+".jpg",sub_img);
        
            tesseract::TessBaseAPI tess;
            tess.Init(NULL, "eng", tesseract::OEM_DEFAULT);
            tess.SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
            tess.SetImage((uchar*)sub_img.data, sub_img.cols, sub_img.rows, 1, sub_img.cols);

            char* out = tess.GetUTF8Text();
            cout << num<<","<<i<<"\t"<<out << endl;
        }
    }


}

void recognize_name(Mat img, int num)
{
    vector<vector<Point>> cnts = dilate_contours(img,21,3);
    Mat sub_img;
    for(int i = 0; i < cnts.size(); ++i){
        Rect temp = boundingRect(cnts[i]);
        if(temp.width / temp.height >=2 && temp.width / temp.height <= 4 && temp.height > 30){    
            if(temp.x > 0)  temp.x -= 1;
            if(temp.y > 0)  temp.y -= 1;
            if(temp.x + temp.width < img.cols - 2) temp.width += 2;
            if(temp.y + temp.height < img.rows -2) temp.height += 2;
            sub_img = img(temp);
            imwrite(to_string(num)+"_"+to_string(i)+".jpg",sub_img);
            
            tesseract::TessBaseAPI tess;
            tess.Init(NULL, "chi_sim", tesseract::OEM_DEFAULT);
            tess.SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
            tess.SetImage((uchar*)sub_img.data, sub_img.cols, sub_img.rows, 1, sub_img.cols);

            char* out = tess.GetUTF8Text();
            cout << num<<","<<i<<"\t"<<out << endl;
        }
    }
}
