// Opencv Version 3.4.1

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/core/types_c.h>

using namespace std;

void FindCircle(cv::Mat img)
{
	// set return result
	// int* res_pos;
	// set ROI
	cv::Rect roi = selectROI(img);
	// get the cropped image by ROI 
	cv::Mat cropped_img = img(roi);
	// set the gray image
	cv::Mat gray_img;
	cvtColor(cropped_img, gray_img, CV_BGR2GRAY);

	vector<cv::Vec3f> circles;
	HoughCircles(gray_img, circles, CV_HOUGH_GRADIENT, 1, gray_img.rows / 8, 200, 100);

	if (circles.size() > 0) {
		cv::Point center(cvRound(circles[0][0]) + roi.x, cvRound(circles[0][1]) + roi.y);
		cout << "Circle center: " << center << endl;
	}
	else {
		cout << "No circle detected." << endl;
	}

}

void VisualFeedback(cv::Mat img)
{
	// Get image width and height
	// Get image width and height
	int width = img.cols;
	int height = img.rows;

	// Display image dimensions 
	// cout << "Image width: " << width << endl;	// 1936
	// cout << "Image height: " << height << endl; // 1216

	// Define ROI region
	// cv::Rect roiRect(100, 100, 500, 500);  // example: x, y, width, height
	cv::Rect roiRect = selectROI(img);
	cv::Mat roi = img(roiRect);

	// Convert ROI to gray scale
	cv::Mat gray;
	cvtColor(roi, gray, CV_BGR2GRAY);
	cv::imshow("Gray image", gray);

	// binary image using thresholding
	cv::Mat binary;
	threshold(gray, binary, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	cv::imshow("Binary image", binary);   

	// Find contours of binary image
	vector<vector<cv::Point>> contours;
	findContours(binary, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	// Find center of mass for each contour
	for (int i = 0; i < contours.size(); i++)
	{
		// compute moments of contour
		cv::Moments m = moments(contours[i], true);

		// Compute center of mass
		cv::Point2f center(m.m10 / m.m00, m.m01 / m.m00);

		// Shift center of mass to ROI coordinates
		// center.x += roiRect.x;
		// center.y += roiRect.y;

		// Display center coordinates
		cout << "Center of object " << i << ": " << center << endl;

		// Draw contour and center on input image
		cv::drawContours(img, contours, i, cv::Scalar(0, 0, 255), 2);
		cv::circle(img, center, 5, cv::Scalar(0, 255, 0), -1);

	}

	// Display input image with contours and centers
	cv::imshow("Contours image", img);
	cv::waitKey(0);
	cv::destroyAllWindows();

}


void FindContours(cv::Mat img)
{
	cv::Mat fliter_img;
	// Gaussian
	GaussianBlur(img, fliter_img, cv::Size(3, 3), 0);
	// Canny
	// Canny(fliter_img, fliter_img, 100, 250);

	// Define ROI region
	cv::Rect roiRect = selectROI(fliter_img);
	cv::Mat roi = img(roiRect);
	// cout << "roi x: " << roiRect.x << " roi y: " << roiRect.y << endl;
	
	// Convert ROI to gray scale
	cv::Mat gray;
	cvtColor(roi, gray, CV_BGR2GRAY);
	// cv::imshow("Gray image", gray);

	// binary image using thresholding
	cv::Mat binary;
	threshold(gray, binary, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	// cv::imshow("Binary image", binary);

	// Find contours of binary image
	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarchy;
	findContours(binary, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
	// cout << "ROI number: " << contours.size() << endl;

	// Find the max area contour
	vector< cv::Point > contour;
	unsigned long long max_contour = 0;
	double max_area = cv::contourArea(cv::InputArray(contours[1]), false);

	for (unsigned long long i = 1; i < contours.size(); i++) {
		double temp_area = cv::contourArea(cv::InputArray(contours[i]), false);
		if (max_area < temp_area) {
			max_area = temp_area;
			max_contour = i;
		}
	}

	contour = contours[max_contour];
	cout << "Max Contour Number: " << max_contour << endl;

	// Draw contours on input image
	cv::Mat output = roi.clone();
	

	for (size_t i = 1; i < contours.size(); i++)
	{	
		// Draw contour on input image
		drawContours(output, contours, i, cv::Scalar(0, 0, 255), 2, cv::LINE_8, hierarchy, 0);
		cout << "Contour Number: " << i << endl;
	}

	// compute moments of contour
	cv::Moments m = moments(contour, true);

	// Compute center of mass
	cv::Point2f center(m.m10 / m.m00, m.m01 / m.m00);
	cv::Point2f drawCenter(m.m10 / m.m00, m.m01 / m.m00);

	// Shift center of mass to ROI coordinates
	center.x += roiRect.x;
	center.y += roiRect.y;

	// Display center coordinates
	cout << "Center of object " << ": " << center << endl;

	// Draw the center on input image
	cv::circle(output, drawCenter, 10, cv::Scalar(0, 255, 0), -1);

	// Display input and output image
	// cv::imshow("Input", img);
	cv::imshow("Output", output);

	cv::waitKey(0);
	cv::destroyAllWindows();
}


void OpenVideo() 
{
	// open mp4 video
	cv::VideoCapture cap("D:\\Simon_workspace\\visualFeedback\\src\\test1.mp4");

	// read mp4 video frame by frame, then change frame to IplImage format
	while (true)
	{
		// read every frame
		cv::Mat frame;
		cap.read(frame);

		// if read video done, then exist loop
		if (frame.empty())
		{
			break;
		}

		// show frame
		// cv::imshow("frame", frame);


		// Visual Feedback
		// FindCircle(frame);	// Cannot detect the little circle
		// VisualFeedback(frame);
		FindContours(frame);



		// press q to exist
		if (cv::waitKey(1) == 'q')
		{
			break;
		}
	}

	// release resource
	cap.release();
	cv::destroyAllWindows();

}

int Test()
{
	// Load input image
	cv::Mat image = cv::imread("D:\\Simon_workspace\\visualFeedback\\src\\test.png");
	//cv::Mat image = cv::imread("D:\\Simon_workspace\\visualFeedback\\src\\test.jpg");
	if (image.empty())
	{
		cout << "Could not open or find the image" << endl;
		return -1;
	}

	// VisualFeedback(image);

	FindContours(image);

	return 0;

}

int main()
{
	// open video
	OpenVideo();

	// Test();
	
}
