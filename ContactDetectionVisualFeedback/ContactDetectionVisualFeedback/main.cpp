#include <iostream>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

// Һ�ܼ��ʶ����
Point detect_tip(Mat img) {

	// Ԥ����
	Mat img_gray, img_blur, img_thresh;
	cvtColor(img, img_gray, COLOR_BGR2GRAY);
	GaussianBlur(img_gray, img_blur, Size(5, 5), 0);
	threshold(img_blur, img_thresh, 100, 255, THRESH_BINARY);

	// ��Ե���
	Mat img_canny;
	Canny(img_thresh, img_canny, 100, 200);
	imshow("img_canny", img_canny);

	// ��������
	vector<vector<Point> > contours(100000);
	findContours(img_canny, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);



	// ����任
	vector<Vec2f> lines;
	HoughLines(img_canny, lines, 1, CV_PI / 180, 120, 0, 0);

	// ��ʾ��⵽��������
	/*Mat img_lines = img.clone();
	for (size_t i = 0; i < lines.size(); i++) {
		float rho = lines[i][0], theta = lines[i][1];
		Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a * rho, y0 = b * rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		line(img_lines, pt1, pt2, Scalar(0, 0, 255), 2, CV_AA);
	}
	imshow("Lines", img_lines);*/

	// Ѱ��Һ��
	//vector<Point> pts;
	//for (size_t i = 0; i < lines.size(); i++) {
	//	float rho = lines[i][0], theta = lines[i][1];
	//	Point pt1, pt2;
	//	double a = cos(theta), b = sin(theta);
	//	double x0 = a * rho, y0 = b * rho;
	//	pt1.x = cvRound(x0 + 1000 * (-b));
	//	pt1.y = cvRound(y0 + 1000 * (a));
	//	pt2.x = cvRound(x0 - 1000 * (-b));
	//	pt2.y = cvRound(y0 - 1000 * (a));
	//	pts.push_back(pt1);
	//	pts.push_back(pt2);
	//}
	//Rect rect = boundingRect(pts);
	//rect.x = max(rect.x, 0); // �����ϽǺ�������Ϊ 0��������ָ���
	//if (rect.y + rect.height > img_gray.rows) {
	//	rect.height = img_gray.rows - rect.y; // �������θ߶ȣ����ⳬ����Χ
	//}
	//cout << "Rect X: " << rect.x << ", Y: " << rect.y << "." << endl;

	//// ���Һ�ܼ��
	//Mat img_roi = img_gray(rect);
	//
	//imshow("ROI", img_roi);
	//waitKey(0);
	//destroyAllWindows();

	//threshold(img_roi, img_roi, 50, 255, THRESH_BINARY);
	//
	//Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
	//dilate(img_roi, img_roi, kernel);
	//vector<vector<Point>> contours;
	//findContours(img_roi, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	//if (contours.empty()) {
	//	return Point(-1, -1);
	//}
	//Moments mu = moments(contours[0], false);
	//if (mu.m00 == 0) {
	//	return Point(-1, -1);
	//}
	//Point tip(mu.m10 / mu.m00, mu.m01 / mu.m00);
	//tip.x += rect.x;
	//tip.y += rect.y;


	// ����Һ�ܼ��λ��
	//return tip;
	return Point(-1, -1);
}


Point findMicroPipetteTip(Mat frame, bool drawTip = false) {
	// ���ԭʼ֡
	Mat frameCopy = frame.clone();

	// ��֡ת��Ϊ�Ҷ�ͼ��
	cvtColor(frameCopy, frameCopy, CV_BGR2GRAY);

	// ��ͼ��Ӧ�ø�˹ģ����ȥ������
	GaussianBlur(frameCopy, frameCopy, Size(3, 3), 0, 0);

	// ��ͼ��Ӧ������Ӧ��ֵ�Է���Һ�ܼ�˺ͱ���
	Mat binaryImage;
	adaptiveThreshold(frameCopy, binaryImage, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 11, 2);
	//imshow("binaryImage", binaryImage);


	// Ѱ��ͼ���е�����
	vector<vector<Point>> contours;
	findContours(binaryImage, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	//// Ѱ����������������������Һ�ܼ��
	//double maxArea = 0;
	//int maxAreaIdx = -1;
	//for (int i = 0; i < contours.size(); i++) {
	//	double area = contourArea(contours[i]);
	//	if (area > maxArea) {
	//		maxArea = area;
	//		maxAreaIdx = i;
	//	}
	//}

	//// ��ȡ������������ı߽��
	//Rect bbox;
	//if (maxAreaIdx != -1) {
	//	bbox = boundingRect(contours[maxAreaIdx]);
	//}
	//else {
	//	bbox = Rect(0, 0, 0, 0);
	//}

	//// ���߽���������ΪҺ�ܼ������
	//Point tip(bbox.x + bbox.width / 2, bbox.y + bbox.height / 2);

	//// �ͷŶ�ֵ��ͼ�������ڴ�
	//binaryImage.release();

	//// �����Ҫ����ͼ���ϻ���Һ�ܼ��
	//if (drawTip) {
	//	circle(frameCopy, tip, 5, Scalar(0, 255, 0), 2);
	//}

	// �ͷ������ԭʼ֡������ڴ�
	frameCopy.release();

	return Point(-1,-1);
}

int main(int argc, char** argv)
{
	string video_path = "D:\\Simon_workspace\\ContactDetection_Video\\contactResult_newest.mp4";

	// ����Ƶ�ļ�
	VideoCapture cap(video_path);

	// �����Ƶ�Ƿ��
	if (!cap.isOpened()) {
		cerr << "Error opening video file" << endl;
		return -1;
	}

	// ��ȡÿһ֡������
	Mat frame;
	while (cap.read(frame)) {

		Point tip = detect_tip(frame);
		cout << "Tip X: " << tip.x << ", Y: " << tip.y << "." << endl;
		if (tip.x != -1 && tip.y != -1) {
			circle(frame, tip, 5, Scalar(0, 0, 255), 2);
		}

		//findMicroPipetteTip(frame, true);

		imshow("Frame", frame);
		if (waitKey(25) == 27) {
			break;
		}
	}

	// �ͷ���Ƶ�ļ�
	cap.release();

	return 0;
}