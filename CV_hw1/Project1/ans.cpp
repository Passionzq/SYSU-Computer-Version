#include "ans.hpp"

using namespace std;
using namespace cimg_library;

int is_official()
{
	int op = 0;
	printf("Which method do u want to use? (0 for mine, 1 for CImg):  ");
	cin >> op;
	switch (op)
	{
	case 0:
		return 0;
	case 1:
		return 1;
	default:
		printf("Invalid input.\n");
		return 2;
	}
}

void Ans::print_message()
{
	printf("Operations:\n");
	printf("0. �˳�����\n");
	printf("1. ���� 1.bmp �ļ������� CImg.display() ��ʾ��\n");
	printf("2. �� 1.bmp �ļ��İ�ɫ�����ɺ�ɫ����ɫ��������ɫ.\n");
	printf("3. ��ͼ�ϻ���һ���ȱ���������������������(50,50)���߳�Ϊ 40�������ɫΪ��ɫ��\n");
	printf("4. ��ͼ�ϻ���һ��Բ������Բ������(50,50)���뾶Ϊ 15�������ɫΪ��ɫ��\n");
	printf("5. ��ͼ�ϻ���һ����Ϊ 100 ��ֱ�߶Σ��������Ϊ(0, 0)�������Ϊ 135 �ȣ�ֱ�ߵ���ɫΪ��ɫ��\n");
	printf("6. ������Ĳ����������Ϊ 2.bmp��\n");
	printf("\nPlease enter the id of question(1 - 6): ");
}

bool Ans::choose(CImg<uchar>& img)
{
	int id = 0, op = 0;
	cin >> id;

	if (id == 0) {
		printf("\nBye~\n");
		return false;
	}
	uchar blue[3] = { 0,0,255 };
	uchar green[3] = { 0,255,0 };
	uchar yellow[3] = { 255,255,0 };

	switch (id)
	{
	case 1:
		ans1(img);
		break;
	case 2:
		ans2(img);
		break;
	case 3:
		op = is_official();
		switch (op)
		{
		case 0:
			ans3(img);
			break;
		case 1:	
			img.draw_triangle(50, 50 - 40 * pow(3, 0.5) / 3, 30, 50 + 20 * pow(3, 0.5) / 3, 70, 50 + 20 * pow(3, 0.5) / 3, blue);
			break;
		default:
			break;
		}
		break;
	case 4:
		op = is_official();
		switch (op)
		{
		case 0:
			ans4(img);
			break;
		case 1:			
			img.draw_circle(50, 50, 15, yellow);
			break;
		default:
			break;
		}
		break;
	case 5:
		op = is_official();
		switch (op)
		{
		case 0:
			ans5(img);
			break;
		case 1:
			img.draw_line(0, 0, 100,100, green);
			break;
		default:
			break;
		}
		break;
	case 6:
		ans6(img);
		break;
	default:
		printf("Invalid input. Please try again.\n");
		break;
	}
	if (id < 6 && id > 0 && op != 2) {
		string str = "Operation " + to_string(id);
		img.display(str.c_str());
	}
	system("pause");
	return true;
}

void Ans::ans1(CImg<uchar>& img)
{
}

void Ans::ans2(CImg<uchar>& img)
{
	cimg_forXY(img, x, y) {
		if (img(x, y, 0) == 255 && img(x, y, 1) == 255 && img(x, y, 2) == 255) {
			img(x, y, 0) = 255;
			img(x, y, 1) = 0;
			img(x, y, 2) = 0;
		}
		else if (img(x, y, 0) == 0 && img(x, y, 1) == 0 && img(x, y, 2) == 0) {
			img(x, y, 0) = 0;
			img(x, y, 1) = 255;
			img(x, y, 2) = 0;
		}
	}
}

void Ans::ans3(CImg<uchar>& img)
{
	double point1[2] = { 50,50 - 40 * pow(3,0.5) / 3 };
	double point2[2] = { 30, 50 + 20 * pow(3, 0.5) / 3 };
	double point3[2] = { 70, 50 + 20 * pow(3, 0.5) / 3 };

	// p1+p2 = l1, p1+p3 = l2, p2+p3 = l3
	double k1 = (point1[1] - point2[1]) / (point1[0] - point2[0]);
	double b1 = point1[1] - k1 * point1[0];
	double k2 = (point1[1] - point3[1]) / (point1[0] - point3[0]);
	double b2 = point1[1] - k2 * point1[0];
	double k3 = 0;
	double b3 = point2[1];

	//judge p3 and l1, p2 and l2, p1 and l3
	int p3_l1 = static_cast<int>(point3[1] - (k1*point3[0] + b1));
	int p2_l2 = static_cast<int>(point2[1] - (k2*point2[0] + b2));
	int p1_l3 = static_cast<int>(point1[1] - (k3*point1[0] + b3));

	for (int x = point2[0] - 1; x < point3[0] + 1; ++x) {
		for (int y = point1[1] - 1; y < point3[1] + 1; ++y) {
			//judge p and l1,l2,l3
			int p_l1 = static_cast<int>(y - (k1*x + b1));
			int p_l2 = static_cast<int>(y - (k2*x + b2));
			int p_l3 = static_cast<int>(y - (k3*x + b3));

			bool judge_l1 = ((p3_l1 > 0 && p_l1 > -1) || (p3_l1 < 0 && p_l1 < 1));
			bool judge_l2 = ((p2_l2 > 0 && p_l2 > -1) || (p2_l2 < 0 && p_l2 < 1));
			bool judge_l3 = ((p1_l3 > 0 && p_l3 > -1) || (p1_l3 < 0 && p_l3 < 1));

			if (judge_l1&&judge_l2&&judge_l3) {
				img(x, y, 0) = 0;
				img(x, y, 1) = 0;
				img(x, y, 2) = 255;
			}
		}
	}
}

void Ans::ans4(CImg<uchar>& img)
{
	cimg_forXY(img, x, y) {
		if (pow(x - 50, 2) + pow(y - 50, 2) <= 15 * 15) {
			img(x, y, 0) = 255;
			img(x, y, 1) = 255;
			img(x, y, 2) = 0;
		}
	}
}

void Ans::ans5(CImg<uchar>& img)
{
	cimg_forXY(img, x, y) {
		if (x >= 0 && x < 100 && y == x) {
			img(x, y, 0) = 0;
			img(x, y, 1) = 255;
			img(x, y, 2) = 0;
		}
	}
}

void Ans::ans6(CImg<uchar>& img)
{
	ans2(img);
	ans3(img);
	ans4(img);
	ans5(img);
	img.save("2.bmp");
	printf("The image(2.bmp) has been saved to the root path of the project.\n");
}
