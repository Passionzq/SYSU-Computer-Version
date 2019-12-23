#include "Canny.hpp"

using namespace std;
using namespace cimg_library;

Canny::Canny()
{

}

Canny::Canny(string filename)
{
	img.load_bmp(filename.c_str());

	assert(img.data());	

	vector<vector<double>> filter = createFilter(3, 3, SIGMA);	

	for (int i = 0; i < filter.size(); ++i) {
		for (int j = 0; j < filter[i].size(); ++j) {
			cout << filter[i][j]<<"\t";
		}
		cout << "\n";
	}

	img.save("./result/Original.bmp");

	grayscaled = toGrayScale();
	grayscaled.save("./result/GrayScaled.bmp");

	gFiltered = useFilter(filter);
	gFiltered.save("./result/GaussianBlur.bmp");

	sFiltered = sobel();
	sFiltered.save("./result/SobelFiltered.bmp");

	non_max_sup = nonMaxSupp();
	non_max_sup.save("./result/NonMaximaSup.bmp");

	thres = threshold(LOW_BOUND, HIGH_BOUND);
	thres.save("./result/Threshold.bmp");

	edge = edgeTrack(thres);
	edge.save("./result/Edge.bmp");
}

Canny::~Canny()
{

}

CImg<uchar> Canny::toGrayScale()
{
	grayscaled = CImg<uchar>(img.width(), img.height(), 1, 1);

	cimg_forXY(grayscaled, x, y) {
		int r = img(x, y, 0);
		int g = img(x, y, 1);
		int b = img(x, y, 2);

		double newValue = (r * 0.2126 + g * 0.7152 + b * 0.0722);
		for (int i = 0; i < 3; ++i)
			grayscaled(x, y) = newValue;
	}
	return grayscaled;
}

vector<vector<double>> Canny::createFilter(int row, int column, double sigmaIn)
{
	vector<vector<double>> filter;

	for (int i = 0; i < row; i++)
	{
		vector<double> col;
		for (int j = 0; j < column; j++)
		{
			col.push_back(-1);
		}
		filter.push_back(col);
	}

	float coordSum = 0;
	float constant = 2.0 * sigmaIn * sigmaIn;

	float sum = 0.0;

	for (int x = -row / 2; x <= row / 2; x++)
	{
		for (int y = -column / 2; y <= column / 2; y++)
		{
			coordSum = (x*x + y * y);
			filter[x + row / 2][y + column / 2] = (exp(-(coordSum) / constant)) / (M_PI * constant);
			sum += filter[x + row / 2][y + column / 2];
		}
	}

	// ��׼���˲���
	for (int i = 0; i < row; i++)
		for (int j = 0; j < column; j++)
			filter[i][j] /= sum;

	return filter;
}

CImg<uchar> Canny::useFilter( vector<vector<double>> filterIn)
{
	int size = (int)filterIn.size() / 2;
	CImg<uchar> filteredImg(grayscaled.width() - 2 * size, grayscaled.height() - 2 * size, 1, 1);
	for (int i = size; i < grayscaled.width() - size; i++)
	{
		for (int j = size; j < grayscaled.height() - size; j++)
		{
			double sum = 0;

			for (int x = 0; x < filterIn.size(); x++)
				for (int y = 0; y < filterIn.size(); y++)
				{
					sum += filterIn[x][y] * (double)(grayscaled(i + x - size, j + y - size));
				}

			filteredImg(i - size, j - size) = sum;
		}

	}
	return filteredImg;
}

CImg<uchar> Canny::sobel()
{
	//ˮƽ������ݶ�Gx
	double x1[] = { 1.0, 0, -1.0 };
	double x2[] = { 2.0, 0, -2.0 };
	double x3[] = { 1.0, 0, -1.0 };

	vector<vector<double>> xFilter(3);
	xFilter[0].assign(x1, x1 + 3);
	xFilter[1].assign(x2, x2 + 3);
	xFilter[2].assign(x3, x3 + 3);

	//��ֱ������ݶ�Gy
	double y1[] = { -1.0, -2.0, -1.0 };
	double y2[] = { 0, 0, 0 };
	double y3[] = { 1.0, 2.0, 1.0 };

	vector<vector<double>> yFilter(3);
	yFilter[0].assign(y1, y1 + 3);
	yFilter[1].assign(y2, y2 + 3);
	yFilter[2].assign(y3, y3 + 3);

	//Limit Size
	int size = (int)xFilter.size() / 2;

	CImg<uchar> filteredImg(gFiltered.width() - 2 * size, gFiltered.height() - 2 * size, 1, 1);

	angles = CImg<float>(gFiltered.width() - 2 * size, gFiltered.height() - 2 * size, 1, 1); //AngleMap

	for (int i = size; i < gFiltered.width() - size; i++)
	{
		for (int j = size; j < gFiltered.height() - size; j++)
		{
			double sumx = 0;
			double sumy = 0;

			for (int x = 0; x < xFilter.size(); x++)
				for (int y = 0; y < yFilter.size(); y++)
				{
					sumx += yFilter[x][y] * (double)(gFiltered(i + x - size, j + y - size)); //Sobel_X �˲�����ֵ 
					sumy += xFilter[x][y] * (double)(gFiltered(i + x - size, j + y - size)); //Sobel_Y �˲�����ֵ
				}
			double sumxsq = sumx * sumx;
			double sumysq = sumy * sumy;

			double sq2 = sqrt(sumxsq + sumysq);

			if (sq2 > 255) //uchar���
				sq2 = 255;
			filteredImg(i - size, j - size) = sq2;

			if (sumx == 0) //arctan ���
				angles(i - size, j - size) = 90;
			else
				angles(i - size, j - size) = atan(sumy / sumx)*(180.0 / M_PI);
		}
	}

	return filteredImg;
}

CImg<uchar> Canny::nonMaxSupp()
{
	CImg<uchar> nonMaxSupped(sFiltered.width() - 2, sFiltered.height() - 2, 1, 1);
	for (int i = 1; i < sFiltered.width() - 1; i++) {
		for (int j = 1; j < sFiltered.height() - 1; j++) {
			float Tangent = angles(i, j);
			//cout << Tangent << " ";
			nonMaxSupped(i - 1, j - 1) = sFiltered(i, j);
			//ˮƽ��
			if (((-22.5 < Tangent) && (Tangent <= 22.5)) || ((157.5 < Tangent) && (Tangent <= -157.5)))
			{
				if ((sFiltered(i, j) < sFiltered(i + 1, j)) || (sFiltered(i, j) < sFiltered(i - 1, j)))
					nonMaxSupped(i - 1, j - 1) = 0;
			}
			//��ֱ��
			if (((-112.5 < Tangent) && (Tangent <= -67.5)) || ((67.5 < Tangent) && (Tangent <= 112.5)))
			{
				if ((sFiltered(i, j) < sFiltered(i, j + 1)) || (sFiltered(i, j) < sFiltered(i, j - 1)))
					nonMaxSupped(i - 1, j - 1) = 0;
			}

			//-45���
			if (((-67.5 < Tangent) && (Tangent <= -22.5)) || ((112.5 < Tangent) && (Tangent <= 157.5)))
			{
				//if ((sFiltered(i, j) < sFiltered(i - 1, j + 1)) || (sFiltered(i, j) < sFiltered(i + 1, j - 1)))
				if ((sFiltered(i, j) < sFiltered(i + 1, j + 1)) || (sFiltered(i, j) < sFiltered(i - 1, j - 1)))
					nonMaxSupped(i - 1, j - 1) = 0;
			}

			//45���
			if (((-157.5 < Tangent) && (Tangent <= -112.5)) || ((22.5 < Tangent) && (Tangent <= 67.5)))
			{
				//if ((sFiltered(i, j) < sFiltered(i + 1, j + 1)) || (sFiltered(i, j) < sFiltered(i - 1, j - 1)))
				if ((sFiltered(i, j) < sFiltered(i - 1, j + 1)) || (sFiltered(i, j) < sFiltered(i + 1, j - 1)))
					nonMaxSupped(i - 1, j - 1) = 0;
			}
		}
	}
	return nonMaxSupped;
}

CImg<uchar> Canny::threshold(int low, int high)
{
	if (low > 255)
		low = 255;
	if (high > 255)
		high = 255;

	CImg<uchar> EdgeMat(non_max_sup.width(), non_max_sup.height(), 1, 1);

	for (int i = 0; i < non_max_sup.width(); i++)
	{
		for (int j = 0; j < non_max_sup.height(); j++)
		{
			EdgeMat(i, j) = non_max_sup(i, j);
			if (EdgeMat(i, j) > high)
				EdgeMat(i, j) = 255;
			else if (EdgeMat(i, j) < low)
				EdgeMat(i, j) = 0;
			else
			{
				bool anyHigh = false;
				bool anyBetween = false;
				for (int x = i - 1; x < i + 2; x++)
				{
					for (int y = j - 1; y < j + 2; y++)
					{
						if (x <= 0 || y <= 0 || x >= EdgeMat.width() || y >= EdgeMat.height()) //Out of bounds
							continue;
						else
						{
							if (EdgeMat(x, y) > high)
							{
								EdgeMat(i, j) = 255;
								anyHigh = true;
								break;
							}
							else if (EdgeMat(x, y) <= high && EdgeMat(x, y) >= low)
								anyBetween = true;
						}
					}
					if (anyHigh)
						break;
				}
				if (!anyHigh && anyBetween)
					for (int x = i - 2; x < i + 3; x++)
					{
						for (int y = j - 1; y < j + 3; y++)
						{
							if (x < 0 || y < 0 || x >= EdgeMat.width() || y >= EdgeMat.height()) //Խ��
								continue;
							else
							{
								if (EdgeMat(x, y) > high)
								{
									EdgeMat(i, j) = 255;
									anyHigh = true;
									break;
								}
							}
						}
						if (anyHigh)
							break;
					}
				if (!anyHigh)
					EdgeMat(i, j) = 0;
			}
		}
	}
	return EdgeMat;
}

CImg<uchar> Canny::edgeTrack(CImg<uchar> Edge)
{
	// 8����
	const Point directions[8] = { { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 },  { -1, -1 }, { 0, -1 },{ 1, -1 } };

	vector<Point> edge_t;
	vector<vector<Point>> edges;

	// ��Ե����
	int i, j, counts = 0, curr_d = 0;
	for (i = 1; i < Edge.width() - 1; i++) {
		for (j = 1; j < Edge.height() - 1; j++)
		{
			// ��ʼ�㼰��ǰ��
			Point b_pt = Point(i, j);
			Point c_pt = Point(i, j);

			// �����ǰ��Ϊǰ����
			if (255 == Edge(c_pt.x, c_pt.y))
			{
				edge_t.clear();
				bool tra_flag = false;
				// ����
				edge_t.push_back(c_pt);
				Edge(c_pt.x, c_pt.y) = 0;    // �ù��ĵ�ֱ�Ӹ�����Ϊ0

				// ���и���
				while (!tra_flag)
				{
					// ѭ���˴�
					for (counts = 0; counts < 8; counts++)
					{
						// ��ֹ��������
						if (curr_d >= 8)
						{
							curr_d -= 8;
						}
						if (curr_d < 0)
						{
							curr_d += 8;
						}

						// ��ǰ������
						// ���ٵĹ��̣�Ӧ���Ǹ������Ĺ��̣���Ҫ��ͣ�ĸ���������root��
						c_pt = Point(b_pt.x + directions[curr_d].x, b_pt.y + directions[curr_d].y);

						// �߽��ж�
						if ((c_pt.x > 0) && (c_pt.x < Edge.width() - 1) &&
							(c_pt.y > 0) && (c_pt.y < Edge.height() - 1))
						{
							// ������ڱ�Ե
							if (255 == Edge(c_pt.x, c_pt.y))
							{
								curr_d -= 2;   // ���µ�ǰ����
								edge_t.push_back(c_pt);
								Edge(c_pt.x, c_pt.y) = 0;

								// ����b_pt:���ٵ�root��
								b_pt = c_pt;

								break;   // ����forѭ��
							}
						}
						curr_d++;
					}   // end for
					// ���ٵ���ֹ���������8���򶼲����ڱ�Ե
					if (8 == counts)
					{
						// ����
						curr_d = 0;
						tra_flag = true;
						edges.push_back(edge_t);

						break;
					}

				}  // end if
			}  // end while

		}
	}

	CImg<uchar> trace_edge_color(Edge.width(), Edge.height(), 1, 1, 0);

	for (i = 0; i < edges.size(); i++)
	{
		// ���˵���С�ı�Ե 
		if (edges[i].size() > DISTANCE)
		{
			for (j = 0; j < edges[i].size(); j++)
			{
				trace_edge_color(edges[i][j].x, edges[i][j].y) = 255;
			}
		}

	}

	return trace_edge_color;
}