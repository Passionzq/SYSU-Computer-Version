#include <iostream>
#include <vector>
#include <fstream>
#include <set>
#include <string>
#include <algorithm>
#include "../CImg.h"

#define MIN_NUMBER_PIXELS 50
#define MAX_NUMBER_PIXELS 300

typedef unsigned char uchar;

using namespace std;
using namespace cimg_library;

struct Run{
    int id, row, start_col, end_col;
    Run(int _id, int _row, int _start, int _end){
        id = _id, row = _row, start_col = _start, end_col = _end;
    }
};

struct EqualPair{
    int id1, id2;
    EqualPair(int _x, int _y){
        id1 = _x, id2 = _y;
    }

    friend bool operator < (const EqualPair& n1, const EqualPair& n2)
    {
        return n1.id1 < n2.id1 || n1.id2 < n2.id2;
    }
};

int id_num = 0;

CImg<uchar> Delate(CImg<uchar> img);
void find_numbers(CImg<uchar> img, CImg<uchar> &output);
bool cmp(const Run r1, const Run r2);

int main()
{
    CImg<uchar> src("../H-Image.bmp");
    CImg<uchar> delated_img = Delate(src);
    delated_img.save("delated_img.bmp");
    CImg<uchar> ans2 = delated_img;
    find_numbers(delated_img, ans2);
    string filename = "ans2_" + to_string(MIN_NUMBER_PIXELS)+"_" + to_string(MAX_NUMBER_PIXELS)+".bmp";
    ans2.save(filename.c_str());
    
    return 0;
}

CImg<uchar> Delate(CImg<uchar> img)
{
    CImg<uchar> binary_img = img;

    cimg_forXY(img, x, y){
        int r = img(x, y, 0);
        int g = img(x, y, 1);
        int b = img(x, y, 2);
        double gray_level = (r * 0.2126 + g * 0.7152 + b * 0.0722);

        if(gray_level >=128){
            binary_img(x, y, 0) = 255;
            binary_img(x, y, 1) = 255;
            binary_img(x, y, 2) = 255;
        }
        else{
            binary_img(x, y, 0) = 0;
            binary_img(x, y, 1) = 0;
            binary_img(x, y, 2) = 0;
        }
    }
    
    CImg<uchar> delated_img = binary_img;

    cimg_forXY(binary_img, x, y) {
        if(binary_img(x,y,0) == 0){
            if(x-1 >=0 && y - 1 >=0 && x + 1 < binary_img.width() && y + 1 <binary_img.height()){
                for(int i = 0; i < 3; ++i){
                    if(delated_img(x,y-2,i) !=0)
                        delated_img(x,y-1,i) = 0;
                    if(delated_img(x,y+2,i) != 0)
                        delated_img(x,y+1,i) = 0;
                    if(delated_img(x - 2,y,i ) != 0)
                        delated_img(x - 1,y,i ) = 0;
                    if(delated_img(x + 2,y,i ) != 0)
                        delated_img(x+1,y,i) = 0;
                }
            }
        }
    }
    return delated_img;
}

void find_numbers(CImg<uchar> img, CImg<uchar> &output)
{
    vector<Run> run_vector;
    set<EqualPair> eq_set;
    vector<EqualPair> eq_vector;

    cimg_forXY(img, x, y){
        if(img(x,y,0) == 0){
            Run run(-1,y,x,0);
            //遍历本层的连通块的长度
            for(int i = x; i < img.width(); ++i){
                
                //遇到白格子：终止
                if(img(i,y,0) == 255 || i == img.width() - 1){
                    run.end_col = i -1;
                    if(run.id == -1){
                        run.id = id_num++;
                    }
                    run_vector.push_back(run);
                    x = i;
                    break;   
                }

                //如果与上一行连通，则遍历run_vector中，找出对应的连通块的id
                if(run.row -1 >= 0 && img(i,y-1,0) == 0){
                    for(int j = 0; j < run_vector.size(); ++j){
                        if(run_vector[j].row == y-1 && run_vector[j].start_col <= i && run_vector[j].end_col >= i){
                            //如果当前的id是初始值-1，则将上一行的id赋值给当前。
                            if(run.id == -1){
                                run.id = run_vector[j].id;
                                // cout<<run.id<<"\tYes!\n";
                            }
                            //如果当前的id不是初始值（表示之前已经与上一行的某些联通），则这之前的连通块与现在上面这一行的是等价的
                            if(run.id != -1 && run.id != run_vector[j].id){
                                EqualPair eq(run.id, run_vector[j].id);
                                if(run.id > run_vector[j].id){
                                    int temp = eq.id1;
                                    eq.id1 = eq.id2;
                                    eq.id2 = temp;
                                }
                                eq_set.insert(eq);
                            }
                        }
                    }
                }
            }
        }
    }

    for( auto iter = eq_set.begin(); iter != eq_set.end(); ++iter){
        for(int i = 0; i < run_vector.size(); ++i){
            if((*iter).id2 == run_vector[i].id){
                run_vector[i].id = (*iter).id1;
            }
        }
    }

    sort(run_vector.begin(), run_vector.end(), cmp);

    vector<int> pass_id;
    vector<int> needed;
    int length = 0;

    for(int i = 0; i < run_vector.size(); ++i){
        if(find(pass_id.begin(), pass_id.end(),run_vector[i].id) != pass_id.end()){
            continue;
        }
        if(length > MAX_NUMBER_PIXELS){
            pass_id.push_back(run_vector[i].id);
            length = 0;
        }
        length += run_vector[i].end_col - run_vector[i].start_col + 1;
        if(run_vector[i].id != run_vector[i+1].id && length < MAX_NUMBER_PIXELS && length > MIN_NUMBER_PIXELS){
            needed.push_back(run_vector[i].id);
            length = 0;
        }
    }

    uchar red[] = {255,0,0};

    for(int i = 0; i < needed.size(); ++i){
        int left_pixel = 10000, right_pixel = -1, top_pixel = 10000, down_pixel = -1;
        for(int j = 0; j < run_vector.size(); ++j){
            if(needed[i] == run_vector[j].id){
                if(run_vector[j].row > down_pixel){
                    down_pixel = run_vector[j].row;
                }
                if(run_vector[j].row < top_pixel){
                    top_pixel = run_vector[j].row;
                }
                if(run_vector[j].start_col < left_pixel){
                    left_pixel = run_vector[j].start_col;
                }
                if(run_vector[j].end_col > right_pixel){
                    right_pixel = run_vector[j].end_col;
                }
            }
        }
        if(down_pixel - top_pixel > 2 && right_pixel - left_pixel > 2){
            output.draw_line(left_pixel- 1,top_pixel-1, right_pixel+1, top_pixel-1,red);
            output.draw_line(left_pixel- 1,top_pixel-1, left_pixel-1, down_pixel+1,red);
            output.draw_line(right_pixel+1, top_pixel-1, right_pixel+1, down_pixel+1,red);
            output.draw_line(left_pixel-1, down_pixel+1, right_pixel+1, down_pixel+1, red);
        }
    }
}

bool cmp(const Run r1, const Run r2)
{
    if(r1.id < r2.id){
        return true;
    }
    else if (r1.id == r2.id){
        if(r1.row < r2.row){
            return true;
        }
        else{
            return false;
        }
    }
    else{
        return false;
    }
}

