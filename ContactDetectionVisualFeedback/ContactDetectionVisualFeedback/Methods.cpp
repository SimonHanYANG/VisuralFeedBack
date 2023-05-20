#include <iostream>

#include "Methods.h"

using namespace std;
using namespace cv;

cv::Point roiCenter(-1, -1);

void onMouse(int event, int x, int y, int flags, void* userdata)
{
	if (event == CV_EVENT_LBUTTONDBLCLK) // ���������˫���¼�
	{
		roiCenter = cv::Point(x, y); // ���¸���Ȥ��������ĵ�
	}
}


/************************************************************************/
/* ConvertToSDimg ���������Ҷ�ͼ��ת��Ϊ��׼ƫ��ͼ�����ڼ��ͼ���еı�Ե�����Ƚ�����ĻҶ�ͼ��ת��Ϊ CV_32F ��ʽ��Ȼ��ʹ�� blur ������ͼ�����ģ�������õ�ͼ���ƽ��ֵ�����ţ�ʹ�� blur ����������ͼ���ƽ������ģ�������õ�ƽ��ֵ��ƽ��ֵ����󣬸��ݱ�׼ƫ�ʽ�������׼ƫ��ͼ�񲢽���ת��Ϊ CV_8UC1 ��ʽ�����ء�                                                                     */
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
/* detectPipetteContour ���������ͼ���е���Һ�����������ȶ�����ĻҶ�ͼ����и�˹ģ���������������ŵ�ָ����С��Ȼ��ʹ�� ConvertToSDimg ������ͼ��ת��Ϊ��׼ƫ��ͼ�񣬲�ʹ����ֵ���������ֵ�����õ���ֵͼ�񡣽��ţ�ʹ�� erode �� dilate �����Զ�ֵͼ�������̬ѧ������ȥ��������ƽ��ͼ�����ʹ�� findContours �����ҵ���ֵͼ���е������������������е�һ����ĺ������Ƿ�Ϊ 1 ���ж������Ƿ�Ϊ��ȷ����Һ������������ȷ���������������ͼ���ϣ������䱣�浽 pipContour �����С����ָ���˸���Ȥ���� _Roi ���������� _scaleFactor������Ҫ����⵽���������������Ӧ�����ź�ƽ�Ʋ�����                                                                     */
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
	/* ʹ�� findContours �����Ӷ�ֵͼ������ȡ�����������浽 contours �����С��ú����ĵ��������� CV_RETR_EXTERNAL ��ʾֻ����ⲿ������
	/************************************************************************/
	cv::findContours(binImg, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	/************************************************************************/
	/*  ���� contours �����е�ÿһ������������ÿһ����������ʼ�������� iZeroCount Ϊ 0��Ȼ����������е����е㣬
		�����ĺ�����Ϊ 1���򽫸õ��������ɾ�������� iZeroCount ��������һ����Ҫע����ǣ���ѭ��ʹ���˵��������������б�����
		���������п��ܻ�ɾ��һЩ�㣬�����Ҫʹ�õ������ĺ�׺�����������������ǰ׺�����������                                                */
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
		/*  �ж� iZeroCount �Ƿ���� 20������ǣ�����Ϊ������Ϊ��ȷ����Һ����������������� showImg �ϣ�
			�������������浽 pipContour �����С�                                                                     */
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
		cv::cvtColor(frame, grayFrame, CV_BGR2GRAY); // ��ͼƬ��Ϊ�Ҷ�ͼ

		//cv::imshow("grayFrame", grayFrame);

		// ����û�ָ���˸���Ȥ��������ĵ㣬����ȡ��Ӧ�� ROI
		if (roiCenter.x >= 0 && roiCenter.y >= 0)
		{
			cv::Rect roi(roiCenter.x, roiCenter.y - iROIHeight/2, iROIWidth, iROIHeight);
			cv::rectangle(frame, roi, cv::Scalar(0, 0, 255), 2); // ��ͼ���ϻ��Ƹ���Ȥ����ľ���
			cv::Mat roiFrame = grayFrame(roi).clone(); // ��ȡ����Ȥ�����ͼ�񸱱�
			cv::imshow("ROI", roiFrame);

			std::vector<cv::Point> pipContour = detectPipetteContour(grayFrame, 0.75, roi);
			//if (pipContour.empty())
			//{
			//	std::cout << "Could not detect pipette contour!" << std::endl;
			//	return; // or do something else to handle this error
			//}

		}

		cv::imshow("Frame", frame);
		if (cv::waitKey(25) == 27) // ����ESC���˳�ѭ��
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