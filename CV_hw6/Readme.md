# CV_HW6
- Name: Zhang Qi
- ID: 17343153
- Email: zhangq295@mail2.sysu.edu.cn


## 1. Enviorment
- Ubuntu 18.04
- C++11
- OpenCV 3.2
- CImg

## 2. Directory structure
```
.
|---brackets                # 文件夹：存放 作业五 中“括号对应坐标轴上位置”的实验结果
|
|---digits_recognize        # 文件夹：存放 数字识别 的相关文件
|
|   |---adaboost            # 文件夹：存放使用 Adaboost 进行数字识别的实验结果以及产生的xml文件
|   |---knn                 # 文件夹：存放使用 knn 进行数字识别的实验结果。
|   |---sample              # 文件夹：存放用来训练分类器的0~9数据
|   |---svm                 # 文件夹：存放使用 svm 进行数字识别的实验结果以及产生的xml文件。
|
|---nums_dots_blocks        # 文件夹：存放 作业五 中“框住数字、小数点以及分区”的实验结果.
|
|---Ol9b                    # 文件夹：存放程序输入的数据集（为了方便编程，里面的文件名字与原始版本有改动）
|
|---TT035                   # 文件夹：存放程序输入的数据集（为了方便编程，里面的文件名字与原始版本有改动）
|
|---ans.hpp                 # 头文件
|
|---CImg.h                  # CImg库
|
|---main                    # Linux下可执行文件
|
|---main.cpp                # 程序源代码
|
|---Makefile                # Makefile
```

## 3. How to run
```bash
# 可以修改main函数中的注释来选择运行 hw5 或者hw6, 默认使用SVM方法来运行hw6
$ make
$ ./main
# 程序需要输入两个【1~88的整数】，作为输入图片的起始编号和结束编号（对应数据集中文件名）
> int1
> int2
```

## 4. Attention
- Please don't change the directory structure, or it'll cause the unpredictable error.
- You can change the notes in the function main() to execute the hw5 or hw6 (default hw6).
- The result of the hw6 wil be saved in the floder "digits_recognize" and the result of the hw5 will be saved in the floders "brackets" + "nums_dots_blocks".
- If the program doesn't work, plz send me an e-mail and I will repair it asap. Thank u!
