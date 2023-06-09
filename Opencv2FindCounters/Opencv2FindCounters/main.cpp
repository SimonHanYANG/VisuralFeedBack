// D:\\Simon_workspace\\VisuralFeedBack\\Opencv2FindCounters\\Opencv2FindCounters\\pic\\input.png

#include <opencv2/opencv.hpp>

using namespace cv;

void DetectCounter(Mat image)
{
	// 转换为灰度图像
	Mat gray;
	cvtColor(image, gray, CV_BGR2GRAY);

	// 进行自适应阈值化
	Mat thresh;
	adaptiveThreshold(gray, thresh, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 17, 2);  // last two params: neiborhood of pixels(Odd Number); second could be nagtive 

	// 进行闭运算，填补断连部分
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	morphologyEx(thresh, thresh, MORPH_CLOSE, kernel);
	namedWindow("ThreshImage", WINDOW_NORMAL);
	imshow("ThreshImage", thresh);
	waitKey(0);

	// 查找轮廓
	std::vector<std::vector<Point> > contours;
	findContours(thresh, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	// 找到最大的轮廓
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

	// 画出最大的轮廓
	Scalar color(0, 0, 255);
	drawContours(image, contours, largest_contour_index, color, CV_FILLED);

	// 显示处理后的图像
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