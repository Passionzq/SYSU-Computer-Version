#include <iostream>
#include <vector>
#include<algorithm>
#include <set>
#include <vector>
#include "../CImg.h"

#define MIN_NUMBER_PIXELS 50
#define MAX_NUMBER_PIXELS 300

typedef unsigned char uchar;

using namespace std;
using namespace cimg_library;

int id_num=0;
vector<pair<CImg<uchar>,int>> cutted_numbers;

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

CImg<uchar> find_coordinates();
void find_numbers(CImg<uchar> img, CImg<uchar> &output);
void recognize_number();
bool cmp(const Run r1, const Run r2);
bool cmp2(const pair<CImg<uchar>,int> a, const pair<CImg<uchar>, int> b);

int main()
{
    CImg<uchar> temp = find_coordinates();
    temp.save("cut.bmp");
    CImg<uchar> mid = temp;
    find_numbers(temp, mid);
    mid.save("temp.bmp");
    recognize_number();
    return 0;
}

CImg<uchar> find_coordinates()
{
    CImg<uchar> img("../H-Image.bmp");
    CImg<uchar> binary_img = img;

    // turn the image into binary image
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

    // delate the binary image
    // cimg_forXY(binary_img, x, y) {
    //     if(binary_img(x,y,0) == 0){
    //         if(x-1 >=0 && y - 1 >=0 && x + 1 < binary_img.width() && y + 1 <binary_img.height()){
    //             for(int i = 0; i < 3; ++i){
    //                 if(delated_img(x,y-2,i) !=0)
    //                     delated_img(x,y-1,i) = 0;
    //                 if(delated_img(x,y+2,i) != 0)
    //                     delated_img(x,y+1,i) = 0;
    //                 if(delated_img(x - 2,y,i ) != 0)
    //                     delated_img(x - 1,y,i ) = 0;
    //                 if(delated_img(x + 2,y,i ) != 0)
    //                     delated_img(x+1,y,i) = 0;
    //             }
    //         }
    //     }
    // }
    
    int max_count = -1, max_i;

    for(int i = 0; i < delated_img.height(); ++i){
        int temp = 0;
        for(int j = 0; j < delated_img.width(); ++j){
            if(delated_img(j,i,0) == 0){
                temp ++;
            }
        }
        if(temp > max_count){
            max_i = i, max_count = temp;
        }
    }
    
    CImg<uchar> output(delated_img.width(), 50, 1,3); 
    max_i  = max_i - 5;

    for(int i =  0; i < 50; ++i){
        for(int j = 0; j < delated_img.width(); ++j){
            output(j,i,0) = delated_img(j,i+max_i, 0);
            output(j,i,1) = delated_img(j,i+max_i, 1);
            output(j,i,2) = delated_img(j,i+max_i, 2);
        }
    }

    return output;
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
        if(down_pixel - top_pixel > 16 && right_pixel - left_pixel > 5){
            
            CImg<uchar> temp(right_pixel - left_pixel +3, down_pixel - top_pixel + 3, 1,3);
            for(int m = 0; m < temp.height(); ++m){
                for(int n = 0; n < temp.width(); ++n){
                    temp(n,m,0) = output(n+left_pixel-1, m+top_pixel-1,0);
                    temp(n,m,1) = output(n+left_pixel-1, m+top_pixel-1,1);
                     temp(n,m,2) = output(n+left_pixel-1, m+top_pixel-1,2); 
                }
            }
            cutted_numbers.push_back(make_pair(temp,left_pixel));

            output.draw_line(left_pixel- 1,top_pixel-1, right_pixel+1, top_pixel-1,red);
            output.draw_line(left_pixel- 1,top_pixel-1, left_pixel-1, down_pixel+1,red);
            output.draw_line(right_pixel+1, top_pixel-1, right_pixel+1, down_pixel+1,red);
            output.draw_line(left_pixel-1, down_pixel+1, right_pixel+1, down_pixel+1, red);
        }
    }

    sort(cutted_numbers.begin(), cutted_numbers.end(), cmp2);
    // for(int x = 0; x < cutted_numbers.size(); ++x){
    //     string fn = to_string(x)+".bmp";
    //     cutted_numbers[x].first.save(fn.c_str());
    // }
}

void recognize_number()
{
    string path = "../data/ex3_std/";
    CImg<uchar> example[6];
    for(int i = 0; i < 6; ++i){
        string fn = path + to_string(i)+".bmp"; 
        CImg<uchar> temp(fn.c_str());
        example[i] = temp;
    }

    for(int i = 0; i < cutted_numbers.size(); ++i){
        int num = -1;
        double similar_rate = 0;
        CImg<uchar> img = cutted_numbers[i].first;
        for(int j = 0; j < 6; ++j){
            int black_num = 0;
            int rows = example[j].height() > img.height()? img.height() : example[j].height();
            int cols = example[j].width() > img.width() ? img.width() : example[j].width();

            for(int x = 0; x < rows; ++x){
                for(int y = 0; y < cols; ++y){
                    if(example[j](y,x,0) ==0 && img(y,x,0) == 0){
                        ++black_num;
                    }
                }
            }
            if(static_cast<double>(black_num) / (rows * cols) > similar_rate){
                similar_rate = static_cast<double>(black_num) / (rows * cols);
                num = i == 12? 2:j;
            }
        }
        if((i+1)%2==1){
            cout<<num<<" ";
        } else{
            cout<<num<<"\t";
        }
    }
    cout<<endl;
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

bool cmp2(const pair<CImg<uchar>,int> a, const pair<CImg<uchar>, int> b)
{
    return a.second<b.second;
}