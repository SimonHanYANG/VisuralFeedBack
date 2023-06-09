// D:\\Simon_workspace\\VisuralFeedBack\\Opencv2FindCounters\\Opencv2FindCounters\\pic\\input.png

#include <opencv2/opencv.hpp>

using namespace cv;

void DetectCounter(Mat image)
{
	// ת��Ϊ�Ҷ�ͼ��
	Mat gray;
	cvtColor(image, gray, CV_BGR2GRAY);

	// ��������Ӧ��ֵ��
	Mat thresh;
	adaptiveThreshold(gray, thresh, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 17, 2);  // last two params: neiborhood of pixels(Odd Number); second could be nagtive 

	// ���б����㣬���������
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	morphologyEx(thresh, thresh, MORPH_CLOSE, kernel);
	namedWindow("ThreshImage", WINDOW_NORMAL);
	imshow("ThreshImage", thresh);
	waitKey(0);

	// ��������
	std::vector<std::vector<Point> > contours;
	findContours(thresh, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	// �ҵ���������
	int largest_contour_index = 0;
	double largest_area = 0;
	for (size_t i = 0; i < contours.size(); i++)
	{
		double area = contourArea(contours[i], false);
		if (area > largest_area)
		{
			largest_area = area;
			largest_contour_index = i;
		}
	}

	// ������������
	Scalar color(0, 0, 255);
	drawContours(image, contours, largest_contour_index, color, CV_FILLED);

	// ��ʾ������ͼ��
	namedWindow("Image", WINDOW_NORMAL);
	imshow("Image", image);
	waitKey(0);
}


int main()
{
	Mat image = imread("D:\\Simon_workspace\\VisuralFeedBack\\Opencv2FindCounters\\Opencv2FindCounters\\pic\\input.png");
	if (image.empty())
	{
		printf("Failed to read image\n");
		return -1;
	}

	DetectCounter(image);

	return 0;
}