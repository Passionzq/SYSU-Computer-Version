#include "ans.hpp"

string filename;
string file_path = "../DataSet/png/";

// main函数中对其进行修改，从而能够在函数中调用。
string f_temp;

// 输入的图片应置于“../DataSet/png/”路径下，文件名应为[1,2,3...].png
int main()
{
    for(int i = 1; i <= 5; ++i){ 
        f_temp = "pic" + to_string(i);
        filename =  to_string(i);
        CImg<uchar> src((file_path + filename+".png").c_str());

        to_binary_image(src);  
        src = src.erode(2);
        vector<Rectan> range, nums_loc, dots_loc, nums_dots_loc;
        CImg<uchar> nums = src;
        nums_dots_loc = find_nums_dots(src,nums,nums_loc,dots_loc); 
        // cout<<i<<" 1"<<endl;
        // hw5
        cut(nums_dots_loc,range,nums);
        string temp_name = "../ans_pic/ans2_" + to_string(i)+".png";
        nums.save(temp_name.c_str());
        // cout<<i<<" 2"<<endl;
        // bracket(src);

        // hw6
        // recognize_numbers(nums_loc, dots_loc, SVM_FLAG);
    }

    //ex2  
    // bracket(src);
    return 0;
}

void to_binary_image(CImg<uchar>& img)
{ 
    cimg_forXY(img, x, y){
        int r = img(x, y, 0);
        int g = img(x, y, 1);
        int b = img(x, y, 2);
        double gray_level = (r * 0.2126 + g * 0.7152 + b * 0.0722);

        if(gray_level >= BINARY_BOUND){
            img(x, y, 0) = 255;
            img(x, y, 1) = 255;
            img(x, y, 2) = 255;
        }
        else{
            img(x, y, 0) = 0;
            img(x, y, 1) = 0;
            img(x, y, 2) = 0;
        }
    }
}

vector<Rectan> find_nums_dots(CImg<uchar> img, CImg<uchar> &output, vector<Rectan> &nums_loc, vector<Rectan> &dots_loc)
{
    vector<Run> run_vector;
    set<EqualPair> eq_set;
    vector<EqualPair> eq_vector;
    vector<int> pass_id;
    vector<Rectan> needed_rectan;

    int id_num = 0;
    
    //寻找黑色连通块，并将其填入run_vector & eq_set
    cimg_forXY(img, x, y){
        if (0 == img(x,y,0)) {
            Run run(-1, y, x, 0);

            //遍历该连通块在本层的长度
            for (int i = x; x < img.width(); ++i){
                //遇到白格子 or 到达边界：该连通块在本层的遍历结束，将其压入vector
                if (255 == img(i,y,0) || i == img.width() - 1) {
                    run.end_col = (i == (img.width() - 1))? i:i-1;

                    if (-1 == run.id){
                        run.id = id_num++;
                    }
                    run_vector.push_back(run);
                    x = i;
                    break;
                }

                //如果与上一行联通，则反向遍历run_vector来找到与之联通的Run的id
                if (run.row - 1 >= 0 && 0 == img(i,y-1,0)) {
                    for (int j = run_vector.size() - 1; j >= 0; --j) {
                        if(run_vector[j].row == run.row - 1 && run_vector[j].start_col <= i && run_vector[j].end_col >= i) {
                            //若当前run的id为初始值-1，则将上一行的id赋给当前。
                            if (-1 == run.id){
                                run.id = run_vector[j].id;
                            }
                            //若当前run的id不为初始值-1，且与此时上一行的连通块的id不一致，则表示等价连通块
                            if (-1 != run.id && run.id != run_vector[j].id) {
                                EqualPair temp = (run.id < run_vector[j].id)?
                                    EqualPair(run.id,run_vector[j].id) : EqualPair(run_vector[j].id,run.id);
                                eq_set.insert(temp);
                            }
                        }
                    } //end for j
                } // end if
            } //end for i
        } // end cimg_forXY
    }

    //使用并查集将等价连通块的id替换成最小的那个
    int L = run_vector.size()+ 1;
    int* A = new int[L];
    for(int i = 0; i < L; ++i){
        A[i] = i;
    }

    //因为对set进行sort操作时遇到不知该怎么解决的error，所以将其转为vector。
    for(auto iter = eq_set.begin(); iter != eq_set.end(); ++iter){
        eq_vector.push_back(*iter);
    }
    std::sort(eq_vector.begin(),eq_vector.end(),cmp2);

    for(int i = 0; i < eq_vector.size(); ++i){
        int x = eq_vector[i].id2, y = eq_vector[i].id1;
        A[x] = y;
        while(A[y]!=y){
            int k = A[y];
            A[x] = k;
            y = k;
        }
    }

    for(int i = 0; i < run_vector.size(); ++i){ 
        run_vector[i].id = A[run_vector[i].id];
    }

    delete[] A;

    //将run_vector中的Run按照id顺序（以及行号）进行排序 
    std::sort(run_vector.begin(), run_vector.end(), cmp);


    int length = 0, pix_left = 100000, pix_right = -1, pix_top = 10000, pix_down = -1;

    for(int i = 0; i < run_vector.size(); ++i){   
        //不满足条件（连通块长度过长、“矩形”过大）的连通块（id在pass_id中），直接过。
        if(find(pass_id.begin(), pass_id.end(),run_vector[i].id) != pass_id.end()){
            continue;
        }
        
        if(length > MAX_NUMBER_PIXELS || pix_right - pix_left > MAX_NUMBER_WIDTH || pix_down - pix_top > MAX_NUMBER_HEIGHT){
            pass_id.push_back(run_vector[i].id);
            length = 0, pix_left = 100000, pix_right = -1, pix_top = 10000, pix_down = -1;
            continue;
        }

        //找到包围连通块的最小矩形，用于确定该图形的位置。
        if(run_vector[i].row < pix_top) pix_top = run_vector[i].row;
        if(run_vector[i].row > pix_down) pix_down = run_vector[i].row;
        if(run_vector[i].start_col < pix_left) pix_left = run_vector[i].start_col;
        if(run_vector[i].end_col > pix_right) pix_right = run_vector[i].end_col;
        //求连通块的长度
        length += run_vector[i].end_col - run_vector[i].start_col + 1;
        
        //curr_id != next_id，意味着遍历到下一个连通块
        if (run_vector[i].id != run_vector[i+1].id){
            //数字
            if(length <= MAX_NUMBER_PIXELS && length >= MIN_NUMBER_PIXELS 
                && pix_right - pix_left <= MAX_NUMBER_WIDTH && pix_right - pix_left >= MIN_NUMBER_WIDTH
                && pix_down - pix_top <= MAX_NUMBER_HEIGHT && pix_down - pix_top >= MIN_NUMBER_HEIGHT
                // && length > (pix_right - pix_left) * (pix_down - pix_top) / 3
                ){
                needed_rectan.push_back(Rectan(pix_left,pix_right,pix_top,pix_down));
                nums_loc.push_back(Rectan(pix_left,pix_right,pix_top,pix_down));
            }
            // 小数点 
            // else if(length <= MAX_NUMBER_PIXELS && length >= MIN_NUMBER_PIXELS
            //         && pix_right - pix_left <= MIN_NUMBER_WIDTH && pix_down - pix_top <= MIN_NUMBER_HEIGHT
            //         && abs((pix_right - pix_left) - (pix_down - pix_top)) < 3){
            //             needed_rectan.push_back(Rectan(pix_left,pix_right,pix_top,pix_down));
            //             dots_loc.push_back(Rectan(pix_left,pix_right,pix_top,pix_down));
            //         }
            else{
                pass_id.push_back(run_vector[i].id); 
            }
            length = 0, pix_left = 100000, pix_right = -1, pix_top = 10000, pix_down = -1;
        }
    }


   
    for(int i = 0; i < needed_rectan.size(); ++i){
        // // ----------------存起来作为train_data-------------------
        // CImg<uchar> sub_img(needed_rectan[i].pix_right-needed_rectan[i].pix_left+1,needed_rectan[i].pix_down-needed_rectan[i].pix_top+1,1,3,0);
        // for(int x = 0; x < sub_img.height(); ++x){
        //     for(int y = 0; y < sub_img.width();++y){
        //         sub_img(y,x,0) = output(needed_rectan[i].pix_left+y,needed_rectan[i].pix_top+x,0);
        //         sub_img(y,x,1) = output(needed_rectan[i].pix_left+y,needed_rectan[i].pix_top+x,1);
        //         sub_img(y,x,2) = output(needed_rectan[i].pix_left+y,needed_rectan[i].pix_top+x,2);
        //     }
        // }
        // sub_img.save(("./sample/"+f_temp +"_"+to_string(i)+".bmp").c_str());

        // // 若数字是横的，则将其旋转90°
        // if(sub_img.width() > sub_img.height()) {
        //     Mat src = imread(("./sample/"+f_temp +"_"+to_string(i)+".bmp"));
        //     transpose(src, src);
        //     flip(src,src,1);
        //     imwrite(("./sample/"+f_temp +"_"+to_string(i)+".bmp"), src);
        // }

        //--------------将识别的数字和小数点用红线框起来-----------------
        // output.draw_line(needed_rectan[i].pix_left-1,needed_rectan[i].pix_top-1, needed_rectan[i].pix_right+1, needed_rectan[i].pix_top-1,red);
        // output.draw_line(needed_rectan[i].pix_left-1,needed_rectan[i].pix_top-1, needed_rectan[i].pix_left-1, needed_rectan[i].pix_down+1,red);
        // output.draw_line(needed_rectan[i].pix_right+1, needed_rectan[i].pix_top-1, needed_rectan[i].pix_right+1, needed_rectan[i].pix_down+1,red);
        // output.draw_line(needed_rectan[i].pix_left-1, needed_rectan[i].pix_down+1, needed_rectan[i].pix_right+1, needed_rectan[i].pix_down+1, red);
    }

    return needed_rectan;
}

vector<vector<Rectan>> cut(vector<Rectan>needed_rectan,vector<Rectan> &range, CImg<uchar> &output)
{
    Rectan temp(needed_rectan[0].pix_left,needed_rectan[0].pix_right,needed_rectan[0].pix_down-MAX_NUMBER_HEIGHT ,needed_rectan[0].pix_top+MAX_NUMBER_HEIGHT);
    vector<vector<Rectan>> out;
    vector<Rectan> v;

    for(int i = 1; i < needed_rectan.size(); ++i) {
        if((needed_rectan[i].pix_top>=temp.pix_top && needed_rectan[i].pix_top <= temp.pix_down)
            || (needed_rectan[i].pix_down>=temp.pix_top && needed_rectan[i].pix_down <= temp.pix_down)){
                
                if(needed_rectan[i].pix_top < temp.pix_top)  temp.pix_top = needed_rectan[i].pix_top;
                if(needed_rectan[i].pix_top > temp.pix_top) temp.pix_down = needed_rectan[i].pix_down; //+MAX_NUMBER_HEIGHT;
                if(needed_rectan[i].pix_left < temp.pix_left) temp.pix_left = needed_rectan[i].pix_left - MAX_NUMBER_WIDTH / 2;
                if(needed_rectan[i].pix_right > temp.pix_right) temp.pix_right = needed_rectan[i].pix_right + MAX_NUMBER_WIDTH / 2;
                 
                v.push_back(needed_rectan[i]);
            }
        else{
            out.push_back(v);
            v.clear();
            range.push_back(temp);
            // temp = Rectan(needed_rectan[i].pix_left,needed_rectan[i].pix_right,needed_rectan[i].pix_down-MAX_NUMBER_HEIGHT ,needed_rectan[i].pix_top+MAX_NUMBER_HEIGHT);
            temp = Rectan(needed_rectan[i].pix_left,needed_rectan[i].pix_right,needed_rectan[i].pix_top ,needed_rectan[i].pix_down );
        } 
    }
    out.push_back(v);
    range.push_back(temp);

    for(int i = 0; i < range.size(); ++i){
        output.draw_line(range[i].pix_left-2,range[i].pix_top-2,range[i].pix_right+2,range[i].pix_top-2,my_color[i%3]);
        output.draw_line(range[i].pix_left-2,range[i].pix_top-2,range[i].pix_left-2,range[i].pix_down+2,my_color[i%3]);
        output.draw_line(range[i].pix_left-2,range[i].pix_down+2,range[i].pix_right+2,range[i].pix_down+2,my_color[i%3]);
        output.draw_line(range[i].pix_right+2,range[i].pix_top-2,range[i].pix_right+2,range[i].pix_down+2,my_color[i%3]);
    }

    return out;
}

void recognize_numbers(vector<Rectan> nums_loc, vector<Rectan> dots_loc, int flag)
{
    Mat ans;

    switch (flag) {
      case KNN_FLAG:
        ans = knn_recognize_nums(nums_loc, dots_loc);
        f_temp = "./digits_recognize/knn/TT035_" + f_temp + ".png";
        imwrite(f_temp.c_str(), ans);
        break;

      case SVM_FLAG:
        ans = svm_recognize_nums(nums_loc, dots_loc);
        f_temp = "./digits_recognize/svm/TT035_" + f_temp + ".png";
        imwrite(f_temp.c_str(), ans);
        break;

      case ADABOOST_FLAG:
        ans = adaboost_recognize_nums(nums_loc, dots_loc);
        f_temp = "./digits_recognize/adaboost/TT035_" + f_temp + ".png";
        imwrite(f_temp.c_str(), ans);
        break;  

      case XGBOOST_FLAG:
        break;

    default:
        break;
    }
}

Mat knn_recognize_nums(vector<Rectan> nums_loc, vector<Rectan> dots_loc)
{
    /*使用OpenCV对labeled-data进行特征提取*/
    int num = -1;
    const int class_num = 10; //0~9 + '.'
    const int img_size_width = 12; //resize
    const int img_size_height = 18;  
    ostringstream oss;
    Mat sample_gray;
    Mat sample_binary;
    Mat deal_img;
    //训练数据，每一行一个训练图片
    Mat training_data;
    //训练样本标签
	Mat labels;
	//最终的训练样本标签
	Mat clas;
	//最终的训练数据
	Mat traindata;
    for (int p = 0; p < class_num; p++){
		oss << "./digits_recognize/sample/";
		num += 1;//num从0到9
		int label = num;
		oss << num << "/*.bmp";//图片名字后缀，oss可以结合数字与字符串
		string pattern = oss.str();//oss.str()输出oss字符串，并且赋给pattern
		oss.str("");//每次循环后把oss字符串清空
		vector<Mat> input_images;
		vector<String> input_images_name;
		glob(pattern, input_images_name, false);
		//为false时，仅仅遍历指定文件夹内符合模式的文件，当为true时，会同时遍历指定文件夹的子文件夹
		//此时input_images_name存放符合条件的图片地址
		int all_num = input_images_name.size();//文件下总共有几个图片
 
		for (int i = 0; i < all_num; i++)
		{
			cvtColor(imread(input_images_name[i]), sample_gray, COLOR_BGR2GRAY);
            resize(sample_gray,sample_gray,Size(img_size_width,img_size_height));
			threshold(sample_gray, sample_binary, 0, 255, THRESH_OTSU);
			input_images.push_back(sample_binary);
			//循环读取每张图片并且依次放在vector<Mat> input_images内
			deal_img = input_images[i];
 
 
			//注意：我们简单粗暴将整个图的所有像素作为了特征，因为我们关注更多的是整个的训练过程
			//，所以选择了最简单的方式完成特征提取工作，除此中外，
			//特征提取的方式有很多，比如LBP，HOG等等
			//我们利用reshape()函数完成特征提取,
			//reshape(1, 1)的结果就是原图像对应的矩阵将被拉伸成一个一行的向量，作为特征向量。
			deal_img = deal_img.reshape(1, 1);//图片序列化
			training_data.push_back(deal_img);//序列化后的图片依次存入
			labels.push_back(label);//把每个图片对应的标签依次存入
		}
	}
 
	//图片数据和标签转变下
	Mat(training_data).copyTo(traindata);//复制
	traindata.convertTo(traindata, CV_32FC1);//更改图片数据的类型，必要，不然会出错
	Mat(labels).copyTo(clas);//复制

	////===============================创建KNN模型===============================////
	Ptr<KNearest>knn = KNearest::create();
	knn->setDefaultK(10);//k个最近领
	knn->setIsClassifier(true);//true为分类，false为回归
	//训练数据和标签的结合
	Ptr<TrainData>trainData = TrainData::create(traindata, ROW_SAMPLE, clas);
	//训练
	knn->train(trainData);
  
	////===============================预测部分===============================////
	//预测分类
    Mat src_img = imread((file_path + filename + ".png").c_str());
    // text属性
    int fontface = FONT_HERSHEY_COMPLEX_SMALL;
    double fontscale = 0.8;
    int thickness = 1;
    string text;

    // ---------通过num_loc来找到图片中需要判断的连通块（数字）的位置，并且将判断结果打印在图上--------
    for(int i = 0; i < nums_loc.size(); ++i){
        Mat raw_img = src_img(Range(nums_loc[i].pix_top,nums_loc[i].pix_down+1),Range(nums_loc[i].pix_left,nums_loc[i].pix_right+1));
        cvtColor(raw_img, raw_img, COLOR_BGR2GRAY);
        threshold(raw_img, raw_img, 0, 255, CV_THRESH_OTSU);

        // 判断是否需要旋转：测试数据均为竖直方向的数字
        if(raw_img.cols > raw_img.rows) {
            transpose(raw_img, raw_img);
            flip(raw_img,raw_img,1);
        }
        // 图片规格统一化
        resize(raw_img,raw_img,Size(img_size_width,img_size_height));
        Mat input;
        raw_img = raw_img.reshape(1, 1);//输入图片序列化
        input.push_back(raw_img);
        input.convertTo(input, CV_32FC1);//更改图片数据的类型，必要，不然会出错
    
        float r = knn->predict(input);   //对所有行进行预测
        text = to_string(int(r));
        if(nums_loc[i].pix_down < src_img.rows / 3)
            putText(src_img,text.c_str(),Point(nums_loc[i].pix_left,nums_loc[i].pix_down - 5 * MAX_NUMBER_HEIGHT),fontface, fontscale, Scalar(255,0,0), thickness, 8);
        else
            putText(src_img,text.c_str(),Point(nums_loc[i].pix_left,nums_loc[i].pix_down - 1 * MAX_NUMBER_HEIGHT),fontface, fontscale, Scalar(255,0,0), thickness, 8);
    }

    // ---------通过dots_loc来找到图片中需要判断的连通块(小数点))的位置，并且将判断结果打印在图上--------
    text = ".";
    for(int i = 0; i < dots_loc.size(); ++i){
        if (dots_loc[i].pix_down < src_img.rows / 3)
            putText(src_img,text.c_str(),Point(dots_loc[i].pix_left - 0.5 * MAX_NUMBER_WIDTH,dots_loc[i].pix_down - 5 * MAX_NUMBER_HEIGHT),fontface, fontscale, Scalar(0,0,255), thickness, 8);
    }

    return src_img; 
}

Mat svm_recognize_nums(vector<Rectan> nums_loc, vector<Rectan> dots_loc)
{
     //核心思路：//获取一张图片后会将图片特征写入到容器中，
				//紧接着会将标签写入另一个容器中，这样就保证了特征
				//  和标签是一一对应的关系。
	////===============================读取训练数据===============================////
    
    // 当前训练的数字
    int num = -1;

    // 一共分为10类：0~9
    const int class_num = 10; 

    // 统一训练图片以及测试图片的大小：18 × 12
    const int img_size_width = 12;

    // 统一训练图片以及测试图片的大小：18 × 12
    const int img_size_height = 18;  

    // 用作训练集路径的读取
    ostringstream oss;

    // 将训练图片转化为灰度图
    Mat sample_gray;

    // 将训练图片转化为二值图像
    Mat sample_binary;

    // 最终用作训练的图像
    Mat deal_img;

    //训练数据，每一行一个训练图片
    Mat training_data;

    //训练样本标签
	Mat labels;

	//最终的训练样本标签
	Mat clas;

	//最终的训练数据
	Mat traindata;
	//////////////////////从指定文件夹下提取图片//////////////////
	for (int p = 0; p < class_num; p++)//依次提取0到9文件夹中的图片
	{
        vector<Mat> input_images;
		vector<String> input_images_name;

		oss << "./digits_recognize/sample/";
		num += 1;//num从0到9
		int label = num;
		oss << num << "/*.bmp";//图片名字后缀，oss可以结合数字与字符串
		string pattern = oss.str();//oss.str()输出oss字符串，并且赋给pattern
		oss.str("");//每次循环后把oss字符串清空
		
        //为false时，仅仅遍历指定文件夹内符合模式的文件，当为true时，会同时遍历指定文件夹的子文件夹
        //此时input_images_name存放符合条件的图片地址
		glob(pattern, input_images_name, false);
		
		// 训练集文件夹下一共有几个文件
		int all_num = input_images_name.size();
 
		for (int i = 0; i < all_num; i++)//依次循环遍历每个文件夹中的图片
		{
			cvtColor(imread(input_images_name[i]), sample_gray, COLOR_BGR2GRAY);//灰度变换          
            resize(sample_gray,sample_gray,Size(img_size_width,img_size_height));
			threshold(sample_gray, sample_binary, 0, 255, THRESH_OTSU);//二值化
            			
            //循环读取每张图片并且依次放在vector<Mat> input_images内
			input_images.push_back(sample_binary);
			deal_img = input_images[i];
 
            //注意：我们简单粗暴将整个图的所有像素作为了特征，因为我们关注更多的是整个的训练过程
            //，所以选择了最简单的方式完成特征提取工作，除此中外，
            //特征提取的方式有很多，比如LBP，HOG等等
            //我们利用reshape()函数完成特征提取,
            //eshape(1, 1)的结果就是原图像对应的矩阵将被拉伸成一个一行的向量，作为特征向量。 
			deal_img = deal_img.reshape(1, 1);//图片序列化
			training_data.push_back(deal_img);//序列化后的图片依次存入
			labels.push_back(label);//把每个图片对应的标签依次存入
		}
	}

	//图片数据和标签转变下
	Mat(training_data).copyTo(traindata);//复制
	traindata.convertTo(traindata, CV_32FC1);//更改图片数据的类型，必要，不然会出错
	Mat(labels).copyTo(clas);//复制
 
 
	////===============================创建SVM模型===============================////
	// 创建分类器并设置参数
	Ptr<SVM> SVM_params = SVM::create();
	SVM_params->setType(SVM::C_SVC);//C_SVC用于分类，C_SVR用于回归
	SVM_params->setKernel(SVM::LINEAR);  //LINEAR线性核函数。SIGMOID为高斯核函数
 
	SVM_params->setDegree(0);//核函数中的参数degree,针对多项式核函数;
	SVM_params->setGamma(1);//核函数中的参数gamma,针对多项式/RBF/SIGMOID核函数; 
	SVM_params->setCoef0(0);//核函数中的参数,针对多项式/SIGMOID核函数；
	SVM_params->setC(1);//SVM最优问题参数，设置C-SVC，EPS_SVR和NU_SVR的参数；
	SVM_params->setNu(0);//SVM最优问题参数，设置NU_SVC， ONE_CLASS 和NU_SVR的参数； 
	SVM_params->setP(0);//SVM最优问题参数，设置EPS_SVR 中损失函数p的值. 
	//结束条件，即训练1000次或者误差小于0.01结束
	SVM_params->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER + TermCriteria::EPS, 1000, 0.01));
 
	//训练数据和标签的结合
	Ptr<TrainData> tData = TrainData::create(traindata, ROW_SAMPLE, clas);
 
	// 训练分类器
	SVM_params->train(tData);//训练
 
	//保存模型
	SVM_params->save("./digits_recognize/svm/svm数字识别.xml"); 
 
	////===============================预测部分===============================////
	Mat src_img = imread( file_path + filename+".png");

    // text属性
    int fontface = FONT_HERSHEY_COMPLEX_SMALL;
    double fontscale = 0.8;
    int thickness = 1;
    string text;

    // ---------通过num_loc来找到图片中需要判断的连通块（数字）的位置，并且将判断结果打印在图上--------
    for(int i = 0; i < nums_loc.size(); ++i){
        Mat raw_img = src_img(Range(nums_loc[i].pix_top,nums_loc[i].pix_down+1),Range(nums_loc[i].pix_left,nums_loc[i].pix_right+1));
        cvtColor(raw_img, raw_img, COLOR_BGR2GRAY);
        threshold(raw_img, raw_img, 0, 255, CV_THRESH_OTSU);

        // 判断是否需要旋转：测试数据均为竖直方向的数字
        if(raw_img.cols > raw_img.rows) {
            transpose(raw_img, raw_img);
            flip(raw_img,raw_img,1);
        }
        // 图片规格统一化
        resize(raw_img,raw_img,Size(img_size_width,img_size_height));

        Mat input;
        raw_img = raw_img.reshape(1, 1);//输入图片序列化
        input.push_back(raw_img);
        input.convertTo(input, CV_32FC1);//更改图片数据的类型，必要，不然会出错
    
        float r = SVM_params->predict(input);  //对所有行进行预测
        text = to_string(int(r));
        if(nums_loc[i].pix_down < src_img.rows / 3)
            putText(src_img,text.c_str(),Point(nums_loc[i].pix_left,nums_loc[i].pix_down - 5 * MAX_NUMBER_HEIGHT),fontface, fontscale, Scalar(255,0,0), thickness, 8);
        else
            putText(src_img,text.c_str(),Point(nums_loc[i].pix_left,nums_loc[i].pix_down - 1 * MAX_NUMBER_HEIGHT),fontface, fontscale, Scalar(255,0,0), thickness, 8);
    }

    // ---------通过dots_loc来找到图片中需要判断的连通块(小数点))的位置，并且将判断结果打印在图上--------
    text = ".";
    for(int i = 0; i < dots_loc.size(); ++i){
        if (dots_loc[i].pix_down < src_img.rows / 3)
            putText(src_img,text.c_str(),Point(dots_loc[i].pix_left - 0.5 * MAX_NUMBER_WIDTH,dots_loc[i].pix_down - 5 * MAX_NUMBER_HEIGHT),fontface, fontscale, Scalar(0,0,255), thickness, 8);
    }

    return src_img; 
}

Mat adaboost_recognize_nums(vector<Rectan> nums_loc, vector<Rectan> dots_loc)
{
    //核心思路：//获取一张图片后会将图片特征写入到容器中，
				//紧接着会将标签写入另一个容器中，这样就保证了特征
				//  和标签是一一对应的关系。
	////===============================读取训练数据===============================////
    
    // 当前训练的数字
    int num = -1;

    // 一共分为10类：0~9
    const int class_num = 10; 

    // 统一训练图片以及测试图片的大小：18 × 12
    const int img_size_width = 12;

    // 统一训练图片以及测试图片的大小：18 × 12
    const int img_size_height = 18;  

    //创建模型
    Ptr<Boost> model[10];
    for(int i = 0; i < 10; ++i)  model[i]= Boost::create();

	//////////////////////从指定文件夹下提取图片//////////////////
	for (int p = 0; p < class_num; p++)//依次提取0到9文件夹中的图片
	{
        // 用作训练集路径的读取
        ostringstream oss;

        // 将训练图片转化为灰度图
        Mat sample_gray;

        // 将训练图片转化为二值图像
        Mat sample_binary;

        // 最终用作训练的图像
        Mat deal_img;

        //训练数据，每一行一个训练图片
        Mat training_data;

        //训练样本标签
        Mat labels;

        //最终的训练样本标签
        Mat clas;

        //最终的训练数据
        Mat traindata;

        vector<Mat> input_images;
		vector<String> input_images_name;

		oss << "./digits_recognize/sample/";
		num += 1;//num从0到9
		int label = num;
		oss << num << "/*.bmp";//图片名字后缀，oss可以结合数字与字符串
		string pattern = oss.str();//oss.str()输出oss字符串，并且赋给pattern
		oss.str("");//每次循环后把oss字符串清空
		
        //为false时，仅仅遍历指定文件夹内符合模式的文件，当为true时，会同时遍历指定文件夹的子文件夹
        //此时input_images_name存放符合条件的图片地址
		glob(pattern, input_images_name, false);
		
		// 训练集文件夹下一共有几个文件
		int all_num = input_images_name.size();

        // 训练正样本
		for (int i = 0; i < all_num; i++)//依次循环遍历每个文件夹中的图片
		{
			cvtColor(imread(input_images_name[i]), sample_gray, COLOR_BGR2GRAY);//灰度变换          
            resize(sample_gray,sample_gray,Size(img_size_width,img_size_height));
			threshold(sample_gray, sample_binary, 0, 255, THRESH_OTSU);//二值化
            			
            //循环读取每张图片并且依次放在vector<Mat> input_images内
			input_images.push_back(sample_binary);
			deal_img = input_images[i];
 
            //注意：我们简单粗暴将整个图的所有像素作为了特征，因为我们关注更多的是整个的训练过程
            //，所以选择了最简单的方式完成特征提取工作，除此中外，
            //特征提取的方式有很多，比如LBP，HOG等等
            //我们利用reshape()函数完成特征提取,
            //eshape(1, 1)的结果就是原图像对应的矩阵将被拉伸成一个一行的向量，作为特征向量。 
			deal_img = deal_img.reshape(1, 1);//图片序列化
			training_data.push_back(deal_img);//序列化后的图片依次存入
			labels.push_back(label);//把每个图片对应的标签依次存入
		}

        // 训练负样本
        for (int j = 0; j < class_num; ++j) {
            if(j!=label){
                oss << "./digits_recognize/sample/";
                int false_label = -1;
                oss << j << "/*.bmp";//图片名字后缀，oss可以结合数字与字符串
                string pattern = oss.str();//oss.str()输出oss字符串，并且赋给pattern
                oss.str("");//每次循环后把oss字符串清空
                
                //为false时，仅仅遍历指定文件夹内符合模式的文件，当为true时，会同时遍历指定文件夹的子文件夹
                //此时input_images_name存放符合条件的图片地址
                glob(pattern, input_images_name, false);
            
                all_num = input_images_name.size();

                for (int i = 0; i < all_num; i++)//依次循环遍历每个文件夹中的图片
                {
                    cvtColor(imread(input_images_name[i]), sample_gray, COLOR_BGR2GRAY);//灰度变换          
                    resize(sample_gray,sample_gray,Size(img_size_width,img_size_height));
                    threshold(sample_gray, sample_binary, 0, 255, THRESH_OTSU);//二值化
                                
                    //循环读取每张图片并且依次放在vector<Mat> input_images内
                    input_images.push_back(sample_binary);
                    deal_img = input_images[i];
         
                    deal_img = deal_img.reshape(1, 1);//图片序列化
                    training_data.push_back(deal_img);//序列化后的图片依次存入
                    labels.push_back(false_label);//把每个图片对应的标签依次存入
                }
            }
        }

        //图片数据和标签转变下
        Mat(training_data).copyTo(traindata);//复制
        traindata.convertTo(traindata, CV_32FC1);//更改图片数据的类型，必要，不然会出错
        Mat(labels).copyTo(clas);//复制

        //创建训练数据
        Ptr<TrainData> tdata = TrainData::create(traindata, ROW_SAMPLE, clas);

        //算法类型
        model[p]->setBoostType(Boost::GENTLE);
        //弱分类器的数量
        model[p]->setWeakCount(100);
        //0和1之间的阈值
        model[p]->setWeightTrimRate(0.95);
        //树的最大可能深度
        // model[p]->setMaxDepth(5);
        //是否建立替代分裂点
        model[p]->setUseSurrogates(false);
        //先验类概率数组
        // model->setPriors(Mat(priors));

        //训练模型
        model[p]->train(tdata);
        ////将分类器保存到文件中
        model[p]->save( "./digits_recognize/adaboost/adaboost数字识别_" + to_string(p)+".xml" );
	}

	Mat src_img = imread(file_path + filename+".png");

    // text属性
    int fontface = FONT_HERSHEY_COMPLEX_SMALL;
    double fontscale = 0.8;
    int thickness = 1;
    string text;

    // ---------通过num_loc来找到图片中需要判断的连通块（数字）的位置，并且将判断结果打印在图上--------
    for(int i = 0; i < nums_loc.size(); ++i){
        Mat raw_img = src_img(Range(nums_loc[i].pix_top,nums_loc[i].pix_down+1),Range(nums_loc[i].pix_left,nums_loc[i].pix_right+1));
        cvtColor(raw_img, raw_img, COLOR_BGR2GRAY);
        threshold(raw_img, raw_img, 0, 255, CV_THRESH_OTSU);

        // 判断是否需要旋转：测试数据均为竖直方向的数字
        if(raw_img.cols > raw_img.rows) {
            transpose(raw_img, raw_img);
            flip(raw_img,raw_img,1);
        }
        // 图片规格统一化
        resize(raw_img,raw_img,Size(img_size_width,img_size_height));

        Mat input;
        raw_img = raw_img.reshape(1, 1);//输入图片序列化
        input.push_back(raw_img);
        input.convertTo(input, CV_32FC1);//更改图片数据的类型，必要，不然会出错
    
        int r = 0;
        for (r; r < 10; ++r) {
            if(model[r]->predict(input) == r)
                break;
        }
        if (r == 10) r = -1;
        // float r = model->predict(input);  //对所有行进行预测
        text = to_string(int(r));
        if(nums_loc[i].pix_down < src_img.rows / 3)
            putText(src_img,text.c_str(),Point(nums_loc[i].pix_left,nums_loc[i].pix_down - 5 * MAX_NUMBER_HEIGHT),fontface, fontscale, Scalar(255,0,0), thickness, 8);
        else
            putText(src_img,text.c_str(),Point(nums_loc[i].pix_left,nums_loc[i].pix_down - 1 * MAX_NUMBER_HEIGHT),fontface, fontscale, Scalar(255,0,0), thickness, 8);
    }

    // ---------通过dots_loc来找到图片中需要判断的连通块(小数点))的位置，并且将判断结果打印在图上--------
    text = ".";
    for(int i = 0; i < dots_loc.size(); ++i){
        if (dots_loc[i].pix_down < src_img.rows / 3)
            putText(src_img,text.c_str(),Point(dots_loc[i].pix_left - 0.5 * MAX_NUMBER_WIDTH,dots_loc[i].pix_down - 5 * MAX_NUMBER_HEIGHT),fontface, fontscale, Scalar(0,0,255), thickness, 8);
    }

    return src_img; 
}

void bracket(CImg<uchar> img)
{
    CImg<uchar> output = img;
    vector<Rectan> needed_rectan;

    // 因为两个文件夹中的图片中的括号形态不一致，因此需要对其进行分情况处理。
    if(filename[0] == 'C') 
    {
        int aix_length = -1, aix_row = 0;
        for(int i = img.height() / 2; i < img.height(); ++i) {
            int counter = 0;
            for(int j = 0; j < img.width(); ++j){
                if(img(j,i,0) == 0)
                    counter++;
            }
            if(counter > aix_length){
                aix_length  = counter;
                aix_row = i;
            }
        }

        // img.display();
        vector<Run> run_vector;
        set<EqualPair> eq_set;
        vector<EqualPair> eq_vector;
        
        int id_num = 0;
        
        //寻找黑色连通块，并将其填入run_vector & eq_set
        cimg_forXY(img, x, y){
            if(y < aix_row - img.height()/15) continue;
            if (0 == img(x,y,0)) {
                Run run(-1, y, x, 0);

                //遍历该连通块在本层的长度
                for (int i = x; x < img.width(); ++i){
                    //遇到白格子 or 到达边界：该连通块在本层的遍历结束，将其压入vector
                    if (255 == img(i,y,0) || i == img.width() - 1) {
                        run.end_col = (i == (img.width() - 1))? i:i-1;

                        if (-1 == run.id){
                            run.id = id_num++;
                        }
                        run_vector.push_back(run);
                        x = i;
                        break;
                    }

                    //如果与上一行联通，则反向遍历run_vector来找到与之联通的Run的id
                    if (run.row - 1 >= 0 && 0 == img(i,y-1,0)) {
                        for (int j = run_vector.size() - 1; j >= 0; --j) {
                            if(run_vector[j].row == run.row - 1 && run_vector[j].start_col <= i && run_vector[j].end_col >= i) {
                                //若当前run的id为初始值-1，则将上一行的id赋给当前。
                                if (-1 == run.id){
                                    run.id = run_vector[j].id;
                                }
                                //若当前run的id不为初始值-1，且与此时上一行的连通块的id不一致，则表示等价连通块
                                if (-1 != run.id && run.id != run_vector[j].id) {
                                    EqualPair temp = (run.id < run_vector[j].id)?
                                        EqualPair(run.id,run_vector[j].id) : EqualPair(run_vector[j].id,run.id);
                                    eq_set.insert(temp);
                                }
                            }
                        } //end for j
                    } // end if
                } //end for i
            } // end cimg_forXY
        }

        //使用并查集将等价连通块的id替换成最小的那个
        int L = run_vector.size()+ 1;
        int* A = new int[L];
        for(int i = 0; i < L; ++i){
            A[i] = i;
        }

        //因为对set进行sort操作时遇到不知该怎么解决的error，所以将其转为vector。
        for(auto iter = eq_set.begin(); iter != eq_set.end(); ++iter){
            eq_vector.push_back(*iter);
        }
        std::sort(eq_vector.begin(),eq_vector.end(),cmp2);

        for(int i = 0; i < eq_vector.size(); ++i){
            int x = eq_vector[i].id2, y = eq_vector[i].id1;
            A[x] = y;
            while(A[y]!=y){
                int k = A[y];
                A[x] = k;
                y = k;
            }
        }

        for(int i = 0; i < run_vector.size(); ++i){ 
            run_vector[i].id = A[run_vector[i].id];
        }

        delete[] A;

        //将run_vector中的Run按照id顺序（以及行号）进行排序 
        std::sort(run_vector.begin(), run_vector.end(), cmp);

        int length = 0, pix_left = 100000, pix_right = -1, pix_top = 10000, pix_down = -1;

        for(int i = 0; i < run_vector.size(); ++i){   
            //找到包围连通块的最小矩形，用于确定该图形的位置。
            if(run_vector[i].row < pix_top) pix_top = run_vector[i].row;
            if(run_vector[i].row > pix_down) pix_down = run_vector[i].row;
            if(run_vector[i].start_col < pix_left) pix_left = run_vector[i].start_col;
            if(run_vector[i].end_col > pix_right) pix_right = run_vector[i].end_col;
            //求连通块的长度
            length += run_vector[i].end_col - run_vector[i].start_col + 1;
            
            //curr_id != next_id，意味着遍历到下一个连通块
            if (run_vector[i].id != run_vector[i+1].id){
                // 通过连通块长度和长宽比以及黑色像素的占用率来确定括号
                if(length <= MAX_BRACKET_LENGTH && length >= MIN_BRACKET_LENGTH 
                    && (pix_right - pix_left) >= (pix_down - pix_top) && (pix_down - pix_top) > 5 
                    && length <= (pix_right - pix_left)*(pix_down - pix_top)/3
                    ){
                    needed_rectan.push_back(Rectan(pix_left,pix_right,pix_top,pix_down));
                }

                length = 0, pix_left = 100000, pix_right = -1, pix_top = 10000, pix_down = -1;
            }
        }
        
        vector<int> all_bounder_col;
        vector<string> put_numbers;
        for(int i = 0; i < needed_rectan.size(); ++i){
            int length = 0, sub_aix_row = 0;

            // 找出每个括号中的最长的那一行基准线，从而确定括号的边界
            for(int x = needed_rectan[i].pix_top; x <= needed_rectan[i].pix_down; ++x){
                int counter = 0;
                for(int y = needed_rectan[i].pix_left; y <= needed_rectan[i].pix_right; ++y){
                    if(output(y,x,0) == 0){
                        counter++;
                    }
                }
                if(counter > length){
                    length = counter;
                    sub_aix_row = x;
                }
            }
            
            vector<int> bounder_col;

            // 找到括号的边界，此处是以上述基准线为准，向上搜索像素
            for(int j = needed_rectan[i].pix_left; j <= needed_rectan[i].pix_right; ++j) {
                bool judge = true;
                if(output(j,sub_aix_row,0) == 0 && output(j,sub_aix_row,1) == 0 && output(j,sub_aix_row,2) == 0){
                    for(int k = sub_aix_row; k >= sub_aix_row - 5; k--){
                        if(output(j,k,0)!=0){
                            judge = false;
                            break;
                        }
                    }
                }
                else{
                    judge = false;
                }
                if(judge){
                    bounder_col.push_back(j);
                    all_bounder_col.push_back(j);
                    j = j + 3;  //避免重复
                }
            }
            
            for(int j = 0; j < bounder_col.size(); ++j){
                output.draw_line(bounder_col[j],aix_row - 10, bounder_col[j],sub_aix_row,red);
                output.draw_circle(bounder_col[j],aix_row, 3, yellow);
            }
        }

        int basic_col = 0;
        double unit_length = 0;
        double basic_value = 0;
        string fn;

        if(filename[filename.length()-1] == '1'){
            basic_col = 75;
            unit_length = 217.333;
            basic_value = 9.0;
            fn = "./brackets/TT035_1.png";
        }
        else if(filename[filename.length()-1] == '3'){
            basic_col = 50;
            unit_length = 220.333;
            basic_value = 9.0;
            fn = "./brackets/TT035_3.png";            
        }
        else if(filename[filename.length()-1] == '5'){
            basic_col = 111;
            unit_length = 226.8235;
            basic_value = 8.5;
            fn = "./brackets/TT035_5.png";

        }
        else if(filename[filename.length()-1] == '7'){
            basic_col = 68;
            unit_length = 243.125;
            basic_value = 8.0;
            fn = "./brackets/TT035_7.png";
        }

        std::sort(all_bounder_col.begin(), all_bounder_col.end());
        for(int i = 0; i < all_bounder_col.size(); ++i){
            output(all_bounder_col[i],aix_row,0) = 0;
            output(all_bounder_col[i],aix_row,1) = 0; 
            output(all_bounder_col[i],aix_row,2) = 255;
            double x = basic_value - static_cast<double>(all_bounder_col[i] - basic_col) / unit_length;
            string str = to_string(x).substr(0,5);
            // cout<<str<<endl;
            put_numbers.push_back(str);
        }
        output.save(fn.c_str());

        Mat add_img = imread(fn);
        int fontface = FONT_HERSHEY_COMPLEX_SMALL;
        double fontscale = 0.8;
        int thickness = 1;
        
        
        for(int i = 0, x = 0, y = 0; i < all_bounder_col.size(); ++i){
            if(i%2){
                putText(add_img,put_numbers[i].c_str(),Point(all_bounder_col[i] - 15,aix_row - 5 * (x + 1)),fontface, fontscale, Scalar(255,0,0), thickness, 8);
                x = (x+1)%3;
            }
            else {
                putText(add_img,put_numbers[i].c_str(),Point(all_bounder_col[i]- 15,aix_row + 15* (y + 1)),fontface, fontscale, Scalar(255,0,0), thickness, 8);
                y = (y+1)%3;
            }
        }

        imwrite(fn.c_str(),add_img);
        
    }
    else{
        int aix_length = -1, aix_row = 0;
        for(int i = img.height() / 2; i < 13 * img.height() / 15; ++i) {
            int counter = 0;
            for(int j = 0; j < img.width(); ++j){
                if(img(j,i,0) == 0)
                    counter++;
            }
            if(counter > aix_length){
                aix_length  = counter;
                aix_row = i;
            }
        }

        vector<Run> run_vector;
        set<EqualPair> eq_set;
        vector<EqualPair> eq_vector;
        
        int id_num = 0;
        
        //寻找黑色连通块，并将其填入run_vector & eq_set
        cimg_forXY(img, x, y){
            if(y < aix_row - img.height()/10 || y > aix_row) continue;
            if (0 == img(x,y,0)) {
                Run run(-1, y, x, 0);

                //遍历该连通块在本层的长度
                for (int i = x; x < img.width(); ++i){
                    //遇到白格子 or 到达边界：该连通块在本层的遍历结束，将其压入vector
                    if (255 == img(i,y,0) || i == img.width() - 1) {
                        run.end_col = (i == (img.width() - 1))? i:i-1;

                        if (-1 == run.id){
                            run.id = id_num++;
                        }
                        run_vector.push_back(run);
                        x = i;
                        break;
                    }

                    //如果与上一行联通，则反向遍历run_vector来找到与之联通的Run的id
                    if (run.row - 1 >= 0 && 0 == img(i,y-1,0)) {
                        for (int j = run_vector.size() - 1; j >= 0; --j) {
                            if(run_vector[j].row == run.row - 1 && run_vector[j].start_col <= i && run_vector[j].end_col >= i) {
                                //若当前run的id为初始值-1，则将上一行的id赋给当前。
                                if (-1 == run.id){
                                    run.id = run_vector[j].id;
                                }
                                //若当前run的id不为初始值-1，且与此时上一行的连通块的id不一致，则表示等价连通块
                                if (-1 != run.id && run.id != run_vector[j].id) {
                                    EqualPair temp = (run.id < run_vector[j].id)?
                                        EqualPair(run.id,run_vector[j].id) : EqualPair(run_vector[j].id,run.id);
                                    eq_set.insert(temp);
                                }
                            }
                        } //end for j
                    } // end if
                } //end for i
            } // end cimg_forXY
        }

        //使用并查集将等价连通块的id替换成最小的那个
        int L = run_vector.size()+ 1;
        int* A = new int[L];
        for(int i = 0; i < L; ++i){
            A[i] = i;
        }

        //因为对set进行sort操作时遇到不知该怎么解决的error，所以将其转为vector。
        for(auto iter = eq_set.begin(); iter != eq_set.end(); ++iter){
            eq_vector.push_back(*iter);
        }
        std::sort(eq_vector.begin(),eq_vector.end(),cmp2);

        for(int i = 0; i < eq_vector.size(); ++i){
            int x = eq_vector[i].id2, y = eq_vector[i].id1;
            A[x] = y;
            while(A[y]!=y){
                int k = A[y];
                A[x] = k;
                y = k;
            }
        }

        for(int i = 0; i < run_vector.size(); ++i){ 
            run_vector[i].id = A[run_vector[i].id];
        }

        delete[] A;

        //将run_vector中的Run按照id顺序（以及行号）进行排序 
        std::sort(run_vector.begin(), run_vector.end(), cmp);

        int length = 0, pix_left = 100000, pix_right = -1, pix_top = 10000, pix_down = -1;

        for(int i = 0; i < run_vector.size(); ++i){   
            //找到包围连通块的最小矩形，用于确定该图形的位置。
            if(run_vector[i].row < pix_top) pix_top = run_vector[i].row;
            if(run_vector[i].row > pix_down) pix_down = run_vector[i].row;
            if(run_vector[i].start_col < pix_left) pix_left = run_vector[i].start_col;
            if(run_vector[i].end_col > pix_right) pix_right = run_vector[i].end_col;
            //求连通块的长度
            length += run_vector[i].end_col - run_vector[i].start_col + 1;
            
            //curr_id != next_id，意味着遍历到下一个连通块
            if (run_vector[i].id != run_vector[i+1].id){
                // 通过连通块长度和长宽比来确定括号
                if(length <= MAX_BRACKET_LENGTH  && (pix_down - pix_top) >= 25 && pix_right < img.width() - 200
                    // && length < 2 * (pix_down - pix_top) * (pix_right - pix_left) / 3
                    ){
                    needed_rectan.push_back(Rectan(pix_left,pix_right,pix_top,pix_down));
                }

                length = 0, pix_left = 100000, pix_right = -1, pix_top = 10000, pix_down = -1;
            }
        }
      
        vector<int> all_bounder_col;
        vector<string> put_numbers;

        for(int i = 0; i < needed_rectan.size(); ++i){
            if((output(needed_rectan[i].pix_right, needed_rectan[i].pix_top,0) == 0 || output(needed_rectan[i].pix_right - 1, needed_rectan[i].pix_top,0) == 0 || output(needed_rectan[i].pix_right, needed_rectan[i].pix_top + 1,0) == 0 || output(needed_rectan[i].pix_right - 1, needed_rectan[i].pix_top + 1,0) == 0)
                && (output(needed_rectan[i].pix_left, needed_rectan[i].pix_top,0) != 0 && output(needed_rectan[i].pix_left+1, needed_rectan[i].pix_top,0) != 0 && output(needed_rectan[i].pix_left, needed_rectan[i].pix_top+1,0) != 0 && output(needed_rectan[i].pix_left + 1, needed_rectan[i].pix_top + 1,0) != 0)                
                ){
                all_bounder_col.push_back(needed_rectan[i].pix_left);
                output.draw_line(needed_rectan[i].pix_left,needed_rectan[i].pix_top,needed_rectan[i].pix_left,aix_row,red);
                output.draw_circle(needed_rectan[i].pix_left,aix_row,3,yellow);
            }
            else if((output(needed_rectan[i].pix_left, needed_rectan[i].pix_top,0) == 0 || output(needed_rectan[i].pix_left+1, needed_rectan[i].pix_top,0) == 0 || output(needed_rectan[i].pix_left, needed_rectan[i].pix_top+1,0) == 0 || output(needed_rectan[i].pix_left + 1, needed_rectan[i].pix_top+1,0) == 0)
                && (output(needed_rectan[i].pix_right, needed_rectan[i].pix_top,0) != 0 && output(needed_rectan[i].pix_right - 1, needed_rectan[i].pix_top,0) != 0 && output(needed_rectan[i].pix_right, needed_rectan[i].pix_top + 1,0) != 0 && output(needed_rectan[i].pix_right - 1, needed_rectan[i].pix_top + 1,0) != 0)
                ){
                all_bounder_col.push_back(needed_rectan[i].pix_right);
                output.draw_line(needed_rectan[i].pix_right,needed_rectan[i].pix_top,needed_rectan[i].pix_right,aix_row,red);
                output.draw_circle(needed_rectan[i].pix_right,aix_row,3,yellow);
            }
            else if((output(needed_rectan[i].pix_right, needed_rectan[i].pix_top,0) == 0 || output(needed_rectan[i].pix_right - 1, needed_rectan[i].pix_top,0) == 0 || output(needed_rectan[i].pix_right, needed_rectan[i].pix_top + 1,0) == 0 || output(needed_rectan[i].pix_right - 1, needed_rectan[i].pix_top + 1,0) == 0)
                    && (output(needed_rectan[i].pix_left, needed_rectan[i].pix_top,0) == 0 || output(needed_rectan[i].pix_left + 1, needed_rectan[i].pix_top,0) == 0 || output(needed_rectan[i].pix_left, needed_rectan[i].pix_top + 1,0) == 0  || output(needed_rectan[i].pix_left + 1, needed_rectan[i].pix_top + 1,0) == 0)
                    ){
                all_bounder_col.push_back((needed_rectan[i].pix_right + needed_rectan[i].pix_left)/2);
                output.draw_line((needed_rectan[i].pix_right + needed_rectan[i].pix_left)/2,needed_rectan[i].pix_top,(needed_rectan[i].pix_right + needed_rectan[i].pix_left)/2,aix_row,red);
                output.draw_circle((needed_rectan[i].pix_right + needed_rectan[i].pix_left)/2,aix_row,3,yellow);
            }
        }

        int basic_col = 0;
        double unit_length = 0;
        double basic_value = 9.5;
        string fn;

        if(filename[filename.length()-1] == '1'){
            basic_col = 391;
            unit_length = 168.778;
            fn = "./brackets/Ol9b_1.png";
        }
        else if(filename[filename.length()-1] == '3'){
            basic_col = 308;
            unit_length = 180.444;
            fn = "./brackets/Ol9b_3.png";            
        }
        else if(filename[filename.length()-1] == '5'){
            basic_col = 284;
            unit_length = 180.111;
            fn = "./brackets/Ol9b_5.png";

        }
        else if(filename[filename.length()-1] == '7'){
            basic_col = 293;
            unit_length = 176.222;
            fn = "./brackets/Ol9b_7.png";
        }

        std::sort(all_bounder_col.begin(), all_bounder_col.end());
        for(int i = 0; i < all_bounder_col.size(); ++i){
            output(all_bounder_col[i],aix_row,0) = 0;
            output(all_bounder_col[i],aix_row,1) = 0; 
            output(all_bounder_col[i],aix_row,2) = 255;
            double x = basic_value - static_cast<double>(all_bounder_col[i] - basic_col) / unit_length;
            string str = to_string(x).substr(0,5);
            put_numbers.push_back(str);
        }
        // output.display();
        output.save(fn.c_str());

        Mat add_img = imread(fn.c_str());
        int fontface = FONT_HERSHEY_COMPLEX_SMALL;
        double fontscale = 0.7;
        int thickness = 1;

        for(int i = 0, x = 0, y = 0; i < all_bounder_col.size(); ++i){
            if(i%2){
                putText(add_img,put_numbers[i].c_str(),Point(all_bounder_col[i] - 15,aix_row - 5 * (x + 1)),fontface, fontscale, Scalar(255,0,0), thickness, 8);
                x = (x+1)%3;
            }
            else {
                putText(add_img,put_numbers[i].c_str(),Point(all_bounder_col[i]- 15,aix_row + 15* (y + 1)),fontface, fontscale, Scalar(255,0,0), thickness, 8);
                y = (y+1)%3;
            }
        }

        imwrite(fn.c_str(),add_img);
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

bool cmp2(const EqualPair& e1, const EqualPair& e2)
{
    return e1.id1 < e2.id1;
}
