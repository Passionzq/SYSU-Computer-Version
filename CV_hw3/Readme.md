# Enviorment
### Ubuntu 18.04, g++ 7.4.0

# How to run?
1. line
    ```
    $ cd ./line/src
    $ g++ Hough.cpp Canny.cpp main.cpp -o main -O2 -L/usr/X11R6/lib -lm -lpthread -lX11
    $ ./main
    $     #input a number in [1,6] means the id of image u want to test
    ```

2. circle
   ```
   $ cd ./circle/
   $ g++ Hough.cpp main.cpp -o main -O2 -L/usr/X11R6/lib -lm -lpthread -lX11
   $ ./main
   $     #input a number in [1,6] means the id of image u want to test
   ```

# Attention
### Plz don't change the names of floder/files.