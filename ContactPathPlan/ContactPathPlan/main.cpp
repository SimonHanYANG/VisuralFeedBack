//"E:\\Contact Detection\\ContactPathPlan\\test.png"

#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;


vector<Point2f> findCellCenters(const Mat& src)
{
	Mat gray, binary, morphed;
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	vector<Point2f> centers;

	// Ԥ����ͼ��ת��Ϊ�Ҷ�ͼ�񲢽��ж�ֵ��
	cvtColor(src, gray, COLOR_BGR2GRAY);
	threshold(gray, binary, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);

	// ������̬ѧ����������������С�ߵ�
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
	morphologyEx(binary, morphed, MORPH_OPEN, kernel);

	imshow("morphed", morphed);
	waitKey(0);

	// ��������
	findContours(morphed, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	// ����ÿ�����������ĵ�
	for (int i = 0; i < contours.size(); i++) {
		Moments mu = moments(contours[i], false);
		Point2f center(mu.m10 / mu.m00, mu.m01 / mu.m00);
		centers.push_back(center);
	}

	return centers;
}

int main()
{
	Mat src = imread("E:\\Contact Detection\\ContactPathPlan\\test.png");
	vector<Point2f> centers = findCellCenters(src);

	// ��ԭͼ���л���ÿ��ϸ�������ĵ�
	for (int i = 0; i < centers.size(); i++) {
		circle(src, centers[i], 3, Scalar(0, 0, 255), -1);
	}

	imshow("result", src);
	waitKey(0);

	// �����������е������
	vector<vector<Point>> contours;
	contours.push_back(vector<Point>(centers.begin(), centers.end()));

	// Ѱ�����·��
	vector<Vec4i> hierarchy;
	vector<Point> approx;
	approxPolyDP(contours[0], approx, 1, true);

	// ����·��
	Mat pathImage = Mat::zeros(src.size(), CV_8UC3);
	drawContours(pathImage, contours, 0, Scalar(255, 0, 0), 2);
	imshow("Path", pathImage);
	waitKey(0);

	drawContours(src, contours, 0, Scalar(255, 0, 0), 2);


	imshow("PathSRC", src);
	waitKey(0);


	return 0;
}