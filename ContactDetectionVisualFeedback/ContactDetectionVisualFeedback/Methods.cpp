#include <iostream>

#include "Methods.h"

using namespace std;
using namespace cv;

cv::Point roiCenter(-1, -1);

void onMouse(int event, int x, int y, int flags, void* userdata)
{
	if (event == CV_EVENT_LBUTTONDBLCLK) // 检测鼠标左键双击事件
	{
		roiCenter = cv::Point(x, y); // 更新感兴趣区域的中心点
	}
}


/************************************************************************/
/* ConvertToSDimg 函数：将灰度图像转换为标准偏差图像，用于检测图像中的边缘。首先将输入的灰度图像转换为 CV_32F 格式，然后使用 blur 函数对图像进行模糊处理，得到图像的平均值。接着，使用 blur 函数对输入图像的平方进行模糊处理，得到平方值的平均值。最后，根据标准偏差公式，计算标准偏差图像并将其转换为 CV_8UC1 格式并返回。                                                                     */
/************************************************************************/
cv::Mat ConvertToSDimg(cv::Mat src_gray8, int KernalSize)
{
	cv::Mat src32_img, mu_img, mu2_img, temp_img, std_gray_img;
	src_gray8.convertTo(src32_img, CV_32F);

	cv::blur(src32_img, mu_img, cv::Size(KernalSize, KernalSize));
	cv::blur(src32_img.mul(src32_img), mu2_img, cv::Size(KernalSize, KernalSize));
	cv::sqrt(mu2_img - mu_img.mul(mu_img), temp_img);
	//temp_img = mu2_img-mu_img.mul(mu_img);
	temp_img.convertTo(std_gray_img, CV_8UC1);

	return std_gray_img;
}

/************************************************************************/
/* detectPipetteContour 函数：检测图像中的移液管轮廓。首先对输入的灰度图像进行高斯模糊处理，并将其缩放到指定大小。然后使用 ConvertToSDimg 函数将图像转换为标准偏差图像，并使用阈值函数将其二值化，得到二值图像。接着，使用 erode 和 dilate 函数对二值图像进行形态学操作以去除噪声和平滑图像。最后使用 findContours 函数找到二值图像中的轮廓，并根据轮廓中第一个点的横坐标是否为 1 来判断轮廓是否为正确的移液管轮廓。将正确的轮廓绘制在输出图像上，并将其保存到 pipContour 向量中。如果指定了感兴趣区域 _Roi 或缩放因子 _scaleFactor，则需要将检测到的轮廓坐标进行相应的缩放和平移操作。                                                                     */
/************************************************************************/
std::vector<cv::Point> detectPipetteContour(cv::Mat _src_gray, float _scaleFactor, cv::Rect _Roi) // _scaleFactor: 0.75
{
	cv::Mat blurImg, binImg, sdImg, src_gray;
	cv::Mat showImg;
	if (_Roi.area() > 0)
	{
		_src_gray(_Roi).copyTo(src_gray);
	}
	else
		_src_gray.copyTo(src_gray);

	cv::GaussianBlur(src_gray, blurImg, cv::Size(5, 5), 1.0, 0.0);
	cv::resize(blurImg, blurImg, cv::Size(src_gray.cols*_scaleFactor, src_gray.rows*_scaleFactor));
	showImg = cv::Mat::zeros(blurImg.size(), CV_8UC3);
	cv::cvtColor(blurImg, showImg, CV_GRAY2BGR);

	// 	cv::Scalar scalarMean = cv::mean(blurImg);
	// 	double dThreshold = scalarMean.val[0]*0.75;
	// 	cv::threshold(blurImg, binImg, dThreshold, 255, cv::THRESH_BINARY_INV);
		//cv::Canny(blur_src, bin_mat, dThreshold*0.5, dThreshold);	//canny with this threshold also works well

	sdImg = ConvertToSDimg(blurImg, 5);
	cv::threshold(sdImg, binImg, 2, 255, cv::THRESH_BINARY);

	// add a line for contour detection
	for (int iRow = 0; iRow < binImg.rows; ++iRow)
	{
		//binImg.at<uchar>(iRow, 0) = 255;
		binImg.at<uchar>(iRow, 1) = 255;	//starts from the second row for contour detection
	}

	cv::erode(binImg, binImg, cv::Mat());
	cv::dilate(binImg, binImg, cv::Mat());

	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Point> pipContour;

	/************************************************************************/
	/* 使用 findContours 函数从二值图像中提取轮廓，并保存到 contours 向量中。该函数的第三个参数 CV_RETR_EXTERNAL 表示只检测外部轮廓。
	/************************************************************************/
	cv::findContours(binImg, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	/************************************************************************/
	/*  遍历 contours 向量中的每一个轮廓，对于每一个轮廓，初始化计数器 iZeroCount 为 0。然后遍历轮廓中的所有点，
		如果点的横坐标为 1，则将该点从轮廓中删除，并将 iZeroCount 计数器加一。需要注意的是，该循环使用了迭代器对轮廓进行遍历，
		遍历过程中可能会删除一些点，因此需要使用迭代器的后缀自增运算符，而不是前缀自增运算符。                                                */
	/************************************************************************/
	for (int ii = 0; ii < contours.size(); ii++)
	{
		//std::cout<<"contour "<<ii<<" size: "<<contours[ii].size()<<std::endl;
		int iZeroCount = 0;
		std::vector<cv::Point>::iterator it;

		for (it = contours[ii].begin(); it < contours[ii].end(); )
		{
			if ((*it).x == 1)
			{
				iZeroCount++;
				std::cout<<iZeroCount<<" : "<<(*it).x<<",  "<<(*it).y<<std::endl;
				contours[ii].erase(it);
			}
			else
				++it;
		}
		/************************************************************************/
		/*  判断 iZeroCount 是否大于 20，如果是，则认为该轮廓为正确的移液管轮廓，将其绘制在 showImg 上，
			并将该轮廓保存到 pipContour 向量中。                                                                     */
		/************************************************************************/
		if (iZeroCount > 20)
		{
			cv::drawContours(showImg, contours, ii, cv::Scalar(0, 0, 255));
			pipContour = contours[ii];
		}
	}

	cv::imshow("showImg", showImg);
	cv::waitKey(1);

	if (_scaleFactor != 1 || _Roi.area() > 0)
	{
		for (int ii = 0; ii < pipContour.size(); ++ii)
		{
			pipContour[ii].x = cvRound((float)(pipContour[ii].x) / _scaleFactor + _Roi.tl().x);
			pipContour[ii].y = cvRound((float)(pipContour[ii].y) / _scaleFactor + _Roi.tl().y);
		}
	}

	return pipContour;
	// Save contour points for algorithm development
// 	std::ofstream outputFile;
// 	outputFile.open("../SavedData/pipette_2.csv");
// 
// 	for (int i = 0; i<pipContour.size(); i++)
// 	{
// 		//std::cout<<i<<": "<<pipContour[i].x<<",   "<<pipContour[i].y<<std::endl;
// 		outputFile<<pipContour[i].x<<","<<pipContour[i].y<<std::endl;
// 	}
// 	outputFile.close();

}

void findPipContour(cv::Mat &src, std::vector<cv::Point> &pipContour)
{
	cv::Mat sdImg;
	cv::Mat binImg;
	cv::Mat kernel;

	// Gaussian blur and resize
	cv::GaussianBlur(src, sdImg, cv::Size(51, 51), 0, 0, cv::BORDER_DEFAULT);
	cv::resize(sdImg, sdImg, cv::Size(300, 300), 0, 0, cv::INTER_LINEAR);

	// Convert to standard deviation image
	cv::subtract(src, sdImg, sdImg);
	cv::normalize(sdImg, sdImg, 0, 255, cv::NORM_MINMAX);

	// Threshold and morphological operations
	cv::threshold(sdImg, binImg, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
	kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	cv::erode(binImg, binImg, kernel);
	cv::dilate(binImg, binImg, kernel);

	// Find contours and filter pip contour
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(binImg, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	for (size_t i = 0; i < contours.size(); i++)
	{
		cv::Rect rect = cv::boundingRect(contours[i]);
		if (rect.width > 100 && rect.height > 100 && contours[i][0].x == 1)
		{
			pipContour = contours[i];
			cv::drawContours(src, contours, i, cv::Scalar(255, 0, 0), 2);
			break;
		}
	}
}

void processVideo(std::string videoPath)
{
	cv::VideoCapture cap(videoPath);
	if (!cap.isOpened())
	{
		std::cerr << "Failed to open video file!" << std::endl;
		return;
	}

	cv::namedWindow("Frame");
	cv::setMouseCallback("Frame", onMouse, NULL);

	cv::Mat frame;
	std::vector<cv::Point> pipContour;
	cv::Point center;

	while (cap.read(frame))
	{
		cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);

		if (center.x != 0 && center.y != 0)
		{
			cv::Rect roi(center.x - 150, center.y - 300, 300, 600);
			cv::rectangle(frame, roi, cv::Scalar(0, 255, 0), 2);

			cv::Mat roiImg = frame(roi).clone();
			findPipContour(roiImg, pipContour);
			if (!pipContour.empty())
			{
				cv::Point offset(center.x - 150, center.y - 300);
				for (size_t i = 0; i < pipContour.size(); i++)
				{
					pipContour[i] += offset;
				}
				cv::drawContours(frame, std::vector<std::vector<cv::Point>>(1, pipContour), 0, cv::Scalar(0, 0, 255), 2);
			}
		}

		cv::imshow("Frame", frame);

		char key = cv::waitKey(10);
		if (key == 27)
		{
			break;
		}
		else if (key == ' ')
		{
			cv::waitKey();
		}
	}

	cap.release();
	cv::destroyAllWindows();
}

void FindROI(string video_path)
{
	int iROIWidth;
	int iROIHeight;

	iROIWidth = 300;
	iROIHeight = 600;

	cv::VideoCapture cap(video_path);
	if (!cap.isOpened())
	{
		std::cout << "Failed to open video" << std::endl;
		return ;
	}

	cv::namedWindow("Frame");
	cv::setMouseCallback("Frame", onMouse, NULL);

	cv::Mat frame;
	while (cap.read(frame))
	{
		cv::Mat grayFrame;
		cv::cvtColor(frame, grayFrame, CV_BGR2GRAY); // 将图片变为灰度图

		//cv::imshow("grayFrame", grayFrame);

		// 如果用户指定了感兴趣区域的中心点，则提取相应的 ROI
		if (roiCenter.x >= 0 && roiCenter.y >= 0)
		{
			cv::Rect roi(roiCenter.x, roiCenter.y - iROIHeight/2, iROIWidth, iROIHeight);
			cv::rectangle(frame, roi, cv::Scalar(0, 0, 255), 2); // 在图像上绘制感兴趣区域的矩形
			cv::Mat roiFrame = grayFrame(roi).clone(); // 提取感兴趣区域的图像副本
			cv::imshow("ROI", roiFrame);

			std::vector<cv::Point> pipContour = detectPipetteContour(grayFrame, 0.75, roi);
			//if (pipContour.empty())
			//{
			//	std::cout << "Could not detect pipette contour!" << std::endl;
			//	return; // or do something else to handle this error
			//}

		}

		cv::imshow("Frame", frame);
		if (cv::waitKey(25) == 27) // 按下ESC键退出循环
		{
			break;
		}
	}
}

//int main()
//{
//	string video_path = "D:\\Simon_workspace\\ContactDetection_Video\\contactResult_newest.mp4";
//	
//	FindROI(video_path);
//
//	//processVideo(video_path);
//
//	return 0;
//}