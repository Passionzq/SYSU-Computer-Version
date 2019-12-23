#include <iostream>
#include "../CImg.h"

typedef unsigned char uchar; 

using namespace std;
using namespace cimg_library;

CImg<uchar> my_OTSU(CImg<uchar> img);

int main()
{
    //You can change the path of input image here.
    CImg<uchar> img("test.bmp");
    CImg<uchar> temp = my_OTSU(img);
    // temp.display();
    temp.save("output.bmp");
    return 0;
}

CImg<uchar> my_OTSU(CImg<uchar> img)
{
    int max_gray_level = -1;
    int min_gray_level = 256;

    CImg<uchar> gray_img = img;  

    // trun the src_img into gray image
    cimg_forXY(img, x, y){
        int r = img(x, y, 0);
        int g = img(x, y, 1);
        int b = img(x, y, 2);
        double gray_level = (r * 0.2126 + g * 0.7152 + b * 0.0722);
        gray_img(x, y, 0) = gray_level;
        gray_img(x, y, 1) = gray_level;
        gray_img(x, y, 2) = gray_level;

        if(gray_level > max_gray_level){
            max_gray_level = gray_level;
        }
        if(gray_level < min_gray_level){
            min_gray_level = gray_level;
        }
    }

    gray_img.save("gray.bmp");
    // gray_img.display();

    int T = -1, foreground_num = 0, background_num = 0,  foreground_avg = 0, background_avg = 0, total = gray_img.width() * gray_img.height();
    double foreground_rate = 0, background_rate = 0, max_variance = -1;

    for (int i = min_gray_level + 1; i < max_gray_level; ++i){

        //calculate the number of  two kinds of pixels
        foreground_num = 0, background_num = 0, foreground_avg = 0, background_avg = 0;
        cimg_forXY(gray_img, x, y){
            if(gray_img(x,y,0) >= i){
                foreground_num++;
                foreground_avg += gray_img(x,y,0);
            }
            else {
                background_num++;
                background_avg += gray_img(x,y,0);
            }   
        }

        // if there is no foreground pixel or background pixel, continue
        if(foreground_num == 0 || background_num == 0){
            continue;
        }

        foreground_avg /= foreground_num;
        background_avg /= background_num;
        foreground_rate = static_cast<double>(foreground_num) / total;
        background_rate = static_cast<double>(background_num) / total;

        double variance = background_rate * foreground_rate * (foreground_avg - background_avg) * (foreground_avg - background_avg);

        if(max_variance < variance){
            max_variance = variance, T = i;
        }
    }

    cout << "The best T is "<<T<<endl;

    cimg_forXY(gray_img, x, y){
        if(gray_img(x,y,0) >= T){
            gray_img(x,y,0) = 255;
            gray_img(x,y,1) = 255;
            gray_img(x,y,2) = 255;
        }
        else{
            gray_img(x,y,0) = 0;
            gray_img(x,y,1) = 0;
            gray_img(x,y,2) = 0;
        }
    }

    return gray_img;
}