#include "stdafx.h"
#include <windows.h>
#include <Shlobj.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <conio.h>
#include <cmath>
#include <iostream>
// shape-detect
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

Mat src;

// Function header
float GetGrayScoreByYUV(cv::Mat img, cv::Rect roi);
float GetGrayScoreByHSV(cv::Mat img, cv::Rect roi);
cv::Rect convertTagRect2CVRect(int left, int top, int right, int bottom);

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "usage 1: .exe (image_path)" << endl;
		cout << "usage 2: .exe (image_path), (head.left), (head.top), (head.right), (head.bottom)" << endl;
		return -1;
	}

	char* filename = argv[1];
	src = imread(argv[1], -1);

	if (src.data)
	{
		cv::Rect head;

		if (argc == 6)
		{
			head = convertTagRect2CVRect(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
		}
		else
		{
			head = cv::Rect(0, 0, 1000, 1000);
		}
		
		//std::cout << filename << ": " << GetGrayScore(src, head) << std::endl;
		std::cout << filename << std::endl;

		GetGrayScoreByYUV(src, head);
		GetGrayScoreByHSV(src, head);
		//cv::imshow(filename, src);
		cv::waitKey();
	}

	return 0;
}

float GetGrayScoreByYUV(cv::Mat img, cv::Rect roi)
{
	cv::Mat thumb;
	cv::Rect rect = roi & cv::Rect(0, 0, img.cols, img.rows);
	cv::resize(img(rect), thumb, cv::Size(50, 50), 0.0, 0.0, cv::INTER_NEAREST);

	cv::imshow("thumb", thumb);

	std::vector<cv::Mat> yuv;
	cv::cvtColor(thumb, thumb, cv::COLOR_RGB2YUV);
	cv::split(thumb, yuv);

	const float lambda = 0.4f;
	cout << "U mean: " << cv::mean(yuv[1])[0] << endl;
	cout << "V mean: " << cv::mean(yuv[2])[0] << endl;
	float exp1 = exp(lambda * cv::mean(yuv[1])[0]);
	float exp2 = exp(lambda * cv::mean(yuv[2])[0]);
	cout << "exp1: " << exp1 << endl;
	cout << "exp2: " << exp2 << endl;

	float score = exp1 / (exp1 + exp2);
	cout << "score: " << score << endl;
	return score;
}

float GetGrayScoreByHSV(cv::Mat img, cv::Rect roi)
{
	cv::Mat thumb, hsv;
	cv::Rect rect = roi & cv::Rect(0, 0, img.cols, img.rows);
	cv::resize(img(rect), thumb, cv::Size(50, 50), 0.0, 0.0, cv::INTER_NEAREST);

	std::vector<cv::Mat> hsv_s;
	cv::cvtColor(thumb, hsv, cv::COLOR_RGB2HSV);
	cv::split(hsv, hsv_s);

	float var = cv::mean(hsv_s[0])[0];
	cout << "HSV mean: " << var << endl;
	float exp1 = exp((74.0f - var) / 22.0f);
	cout << "exp1: " << exp1 << endl;
	float score = exp1 / (1 + exp1);
	cout << "score: " << score << endl;
	return score;
}

cv::Rect convertTagRect2CVRect(int left, int top, int right, int bottom)
{
	cv::Rect out;
	out.x = left;
	out.y = top;
	out.width = right - left;
	out.height = bottom - top;
	return out;
}
