#include "Hough.hpp"
#include "Canny.hpp"

int main() {
	string str;
	cin>>str;
	str = "../Dataset1/"+ str + ".bmp";
	CImg<float> src(str.c_str());
	// args: src sigma gradient_threshold vote_threshold peak_dis
	Hough hough(src, 10.5f, 30, 1000, 60);	//demo: (src, 10.5f, 30, 1000, 60)
	Canny canny(str);
	CImg<float> result = hough.houghProcess(src);
	// result.display();
	result.save("../result/result.bmp");
	return 0;
}