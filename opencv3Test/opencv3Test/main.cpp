#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>



using namespace std;
using namespace cv;


cv::Point detectMicroPipetteTip(cv::Mat image)
{
	cv::Mat grayImage, binaryImage;
	cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
	cv::threshold(grayImage, binaryImage, 100, 255, cv::THRESH_BINARY);

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	/*函数使用RETR_EXTERNAL参数表示只检测最外层轮廓，而使用CHAIN_APPROX_SIMPLE参数表示仅存储轮廓的端点*/
	cv::findContours(binaryImage, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	// 找到面积最大的轮廓
	int maxIndex = 0;
	double maxArea = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		double contourarea = contourArea(contours[i]);
		if (contourarea > maxArea)
		{
			maxArea = contourarea;
			maxIndex = i;
		}
	}

	// 删除最大轮廓       
	contours.erase(contours.begin() + maxIndex);

	cv::Scalar color(0, 255, 0);
	for (int i = 0; i < contours.size(); i++) {
		cv::drawContours(image, contours, i, color, 2);
		
	}

	/*cv::imshow("Counters", image);
	cv::waitKey(0);*/

	// 找到每个轮廓的最前端点
	std::vector<cv::Point> front_points;
	for (int j = 0; j < contours.size(); j++) {
		cv::Point front_point = contours[j][0];
		for (int i = 0; i < contours[j].size(); i++) {
			if (contours[j][i].x < front_point.x) {
				front_point = contours[j][i];
			}
		}
		front_points.push_back(front_point);
	}

	// 找到所有最前端点中x坐标最大的点
	cv::Point global_front_point = front_points[0];
	for (int i = 0; i < front_points.size(); i++) {
		if (front_points[i].x > global_front_point.x) {
			global_front_point = front_points[i];
		}
	}

	cout << "Global Front Point: X: " << global_front_point.x << ", Y: " << global_front_point.y << "." << endl;

	cv::circle(image, global_front_point, 10, cv::Scalar(0, 0, 255), -1);

	cv::imshow("Show tip", image);
	cv::waitKey(0);

	return global_front_point;
}

int main(int argc, char** argv)
{
	string video_path = "D:\\Simon_workspace\\ContactDetection_Video\\contactResult_newest.mp4";

	VideoCapture cap(video_path);

	if (!cap.isOpened()) {
		cerr << "Error opening video file" << endl;
		return -1;
	}

	Mat frame;
	while (cap.read(frame)) {
		cv::Point tipPoint;

		tipPoint = detectMicroPipetteTip(frame);

		/*imshow("Frame", frame);
		if (waitKey(25) == 27) {
			break;
		}*/
	}

	cap.release();

	return 0;
}