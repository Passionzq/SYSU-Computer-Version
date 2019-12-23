# CV PROJECT-Part1

- Name: Zhang Qi
- ID: 17343153
- Email: zhangq295@mail2.sysu.edu.cn

## 1. Enviorment
- Ubuntu 18.04LST
- g++ 7.4.0
- OpenCV 3.2
- CImg

## 2. Directory structure
```
.
|---ans_pic             # 文件夹，存放实验结果截图
|   |
|   |---part1           # 文件夹，存放【名片校正】的相关截图
|   |---part2           # 文件夹，存放【主要内容切割】的实验结果截图
|   |---*.jpg/*.png     # 文件，程序运行的结果会保存到该文件夹下
|    
|---DataSet             # 文件夹，存放程序输入数据（因该文件夹过大，已将部分内容删除）
|
|---src_part1           # 文件夹，存放【名片校正】的源码
|
|---src_part2           # 文件夹，存放【主要内容切割】的源码
|
|---Readme
|---Report
```

## 3. How to run
```bash
$ cd src_part1
$ make && ./main
```

## 4. Attention
- Please don't change the directory structure, or it'll cause the unpredictable error.
- You can change the `for(int i = xx ; i < yy; ++i)` at the beginning of `main()` to run different input.
- If the program doesn't work, plz send me an e-mail and I will repair it asap. Thank u!  