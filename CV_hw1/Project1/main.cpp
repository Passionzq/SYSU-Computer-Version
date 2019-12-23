#include <iostream>
#include "CImg.h"
#include "ans.hpp"

using namespace std;
using namespace cimg_library;

int main()
{
	Ans test;
	bool judge = false;
	do {
		CImg<uchar> img("1.bmp");
		test.print_message();
		judge = test.choose(img);
		system("cls");
	} while (judge);

	return 0;
}