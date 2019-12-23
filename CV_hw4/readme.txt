- 运行环境：
    Ubuntu 18.04, g++ 7.4.0

- 目录结构：
.
|-data：数据集
|   |-ex1_test1：任务一的实验结果1
|   |-ex1_test2：任务一的实验结果2
|   |-ex1_test3：任务一的实验结果3
|   |-ex2_ans：任务二的实验结果（图片命名说明：ans2_low_high.bmp 其中low < T < high）
|   |-ex3_ans：任务三的实验结果
|   |-ex3_std：任务三的数字识别的比照标准
|-src:源码
|   |-exi.cpp：(i = 1,2,3)任务i对应的源码
|   |-exi：(i = 1,2,3)linux下的可执行文件
|   |-Readme.md：运行前请看此文件
|-CImg.h：头文件
|-实验报告.pdf
|-H-Image.bmp：实验数据
