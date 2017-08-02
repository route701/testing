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
// cv
//#include <opencv/cv.h>

using namespace std;
using namespace cv;

Mat src, dst;
int thresh = 100;
int max_thresh = 255;
int contrast = 10;
int max_contrast = 30;
int brightness = 0;
int max_brightness = 100;
RNG rng(12345);

// Function header
void setLabel(Mat& im, const string label, vector<Point>& contour);
static double angle(Point pt1, Point pt2, Point pt0);
void thresh_callback(int, void*);
void contrast_callback(int, void*);
void brightness_callback(int, void*);
bool hasNearbyBigRect(vector<int> vector_tmp, int index);
void overlay(Mat& mat_tmp);
cv::Mat ResizeButKeepAspectRatio(cv::Mat& img, int target_width, int target_height);
void overlayImage(Mat* src, Mat* overlay, const Point& location);
float CheckTamperScore(cv::Mat frame);
void DetectColorImage(cv::Mat frame);
//void ChangeColorSapce(IplImage* Ipl);
void ChangeColorSapce(cv::Mat frame);
float GetGrayScore(cv::Mat img, cv::Rect roi);

int main(int argc, char* argv[])
{
	if (argc < 2) return -1;

	vector<unsigned char> output;
	vector<int> para;

	char* filename = argv[1];
	src = imread(argv[1], -1);

	//Mat src2 = imread(argv[2], -1);

	dst = src.clone();
	Mat blur;

	// 18/05/2017 add alpha channel to original image and overlay an image
	//{
	//	vector<Mat> matChannels;
	//	split(src, matChannels);

	//	Mat src_alpha = matChannels.at(0) + matChannels.at(1) + matChannels.at(2);
	//	matChannels.push_back(src_alpha);

	//	merge(matChannels, dst);

	//	overlayImage(&dst, &src2, Point(0, -800));
	//}

	// 25/03/2017 try blut bottom half image
	//overlay(dst);

	// Increase contrast;
	//src.convertTo(dst, -1, 4, -400);

	// Convert to grayscale
	//cvtColor(dst, dst, CV_BGR2GRAY);

	//medianBlur(dst, dst, 11);
	//GaussianBlur(dst, blur, cv::Size(31, 31), 0);

	// 12/04/2017 try white mat padding
	//dst = ResizeButKeepAspectRatio(src, 600, 450);	

	// Create Window srouce
	//char* source_window = "Source";
	//namedWindow(source_window, CV_WINDOW_AUTOSIZE);
	//imshow(source_window, src);

	// Create Window grey
	//char* modified_window = "grey";
	//namedWindow(modified_window, CV_WINDOW_AUTOSIZE);
	//imshow(modified_window, dst);

	// Create Window blur
	//char* modified_window_2 = "blur";
	//namedWindow(modified_window_2, CV_WINDOW_AUTOSIZE);
	//imshow(modified_window_2, blur);

	//31/07/2017
	CheckTamperScore(src);
	cout << endl;
	cout << "source" << endl;
	DetectColorImage(src);
	cout << endl;

	//01/08/2017
	//IplImage * Ipl = cvLoadImage(argv[1]);
	//ChangeColorSapce(Ipl);
	ChangeColorSapce(src);

	//createTrackbar(" Canny thresh:", "Source", &thresh, max_thresh, thresh_callback);
	//createTrackbar(" contrast: ", "Source", &contrast, max_contrast, contrast_callback);
	//createTrackbar(" brightness: ", "Source", &brightness, max_brightness, brightness_callback);

	//thresh_callback(0, 0);
	//contrast_callback(0, 0);
	//brightness_callback(0, 0);

	if (src.data)
	{
		cv::Rect head = cv::Rect(0, 0, 1000, 1000);
		std::cout << filename << endl << "gray score: " << GetGrayScore(src, head) << std::endl;
		//cv::imshow(filename, src);
		//cv::waitKey();
	}

	waitKey(0);
	return(0);

}

float GetGrayScore(cv::Mat img, cv::Rect roi)
{
	cv::Mat thumb;
	cv::Rect rect = roi & cv::Rect(0, 0, img.cols, img.rows);
	cv::resize(img(rect), thumb, cv::Size(50, 50), 0.0, 0.0, cv::INTER_NEAREST);

	std::vector<cv::Mat> yuv;
	cv::cvtColor(thumb, thumb, cv::COLOR_RGB2YUV);
	cv::split(thumb, yuv);

	const float lambda = 0.4f;
	cout << "U: " << cv::mean(yuv[1])[0] << endl;
	cout << "V: " << cv::mean(yuv[2])[0] << endl;

	float exp1 = exp(lambda * cv::mean(yuv[1])[0]);
	cout << "exp1: " << exp1 << endl;
	
	float exp2 = exp(lambda * cv::mean(yuv[2])[0]);
	cout << "exp2: " << exp2 << endl;

	float score = exp1 / (exp1 + exp2);
	return score;
}

void ChangeColorSapce(cv::Mat frame)
{
	cv::Mat hsv, yuv, lab, test;

	//cvtColor(frame, hsv, CV_BGR2HSV);
	cvtColor(frame, yuv, CV_BGR2YCrCb);
	//cvtColor(frame, lab, CV_BGR2Lab);

	namedWindow("Source-RGB", 0);
	imshow("Source-RGB", frame);
	//namedWindow("HSV", 0);
	//imshow("HSV", hsv);
	namedWindow("YUV", 0);
	imshow("YUV", yuv);
	//namedWindow("Lab", 0);
	//imshow("Lab", lab);

	//cvtColor(lab, test, CV_Lab2RGB);
	//namedWindow("test", 0);
	//imshow("test", test);

	//imwrite("HSV.jpg", hsv);
	imwrite("YUV.jpg", yuv);
	//imwrite("Lab.jpg", lab);
	//imwrite("test.jpg", test);

	cout << "yuv" << endl;
	DetectColorImage(yuv);
}

//float CheckTamperScore(std::vector<unsigned char> &image)
float CheckTamperScore(cv::Mat frame)
{
	//cv::Mat frame = cv::imdecode(image, CV_LOAD_IMAGE_COLOR);
	cv::Mat gray, blur, err;
	cv::Scalar mean, stddev;
	cv::cvtColor(frame, gray, CV_BGR2GRAY);
	cv::GaussianBlur(gray, blur, cv::Size(31, 31), 0);
	cv::absdiff(gray, blur, err);
	cv::meanStdDev(err, mean, stddev);

	cout << "mean: " << mean[0] << endl;
	cout << "stddev: " << stddev[0] << endl;
	return stddev[0];
}

void DetectColorImage(cv::Mat frame)
{
	if (frame.channels() > 1)
	{
		cv::Mat dst, bgr[3];
		cv::Scalar mean, stddev;

		cv::split(frame, bgr);

		//cout << "bgr[0]: " << bgr[0] << "bgr[1]: " << bgr[1] << "bgr[2]: " << bgr[2] << endl;

		cv::absdiff(bgr[0], bgr[1], dst);
		
		{
			cv::meanStdDev(dst, mean, stddev);
			cout << "mean 1: " << mean[0] << endl;
			cout << "stddev 1: " << stddev[0] << endl;
		}

		if (cv::countNonZero(dst))
		{
			cout << "not same 1" << endl;
		}
		else
		{
			cout << "same 1" << endl;
		}

		cout << endl;
		cv::absdiff(bgr[0], bgr[2], dst);

		{
			cv::meanStdDev(dst, mean, stddev);
			cout << "mean 2: " << mean[0] << endl;
			cout << "stddev 2: " << stddev[0] << endl;
		}

		if (cv::countNonZero(dst))
		{
			cout << "not same 2" << endl;
		}
		else
		{
			cout << "same 2" << endl;
		}
		
		cout << endl;
		cv::absdiff(bgr[1], bgr[2], dst);

		{
			cv::meanStdDev(dst, mean, stddev);
			cout << "mean 3: " << mean[0] << endl;
			cout << "stddev 3: " << stddev[0] << endl;
		}

		if (cv::countNonZero(dst))
		{
			cout << "not same 3" << endl;
		}
		else
		{
			cout << "same 3" << endl;
		}
	}
	else
	{
		cout << "1 channel only" << endl;
	}
}

void overlayImage(Mat* src, Mat* overlay, const Point& location)
{
	for (int y = max(location.y, 0); y < src->rows; ++y)
	{
		int fY = y - location.y;

		if (fY >= overlay->rows)
			break;

		for (int x = max(location.x, 0); x < src->cols; ++x)
		{
			int fX = x - location.x;

			if (fX >= overlay->cols)
				break;

			// determine the opacity of the overlay pixel, using its fourth (alpha) channel.
			double opacity = ((double)overlay->data[fY * overlay->step + fX * overlay->channels() + 3]) / 255;

			for (int c = 0; opacity > 0 && c < src->channels(); ++c)
			{
				unsigned char overlayPx = overlay->data[fY * overlay->step + fX * overlay->channels() + c];
				unsigned char srcPx = src->data[y * src->step + x * src->channels() + c];
				src->data[y * src->step + src->channels() * x + c] = srcPx * (1. - opacity) + overlayPx * opacity;
			}
		}
	}
}

cv::Mat ResizeButKeepAspectRatio(cv::Mat& img, int target_width, int target_height)
{
	cv::Mat outMat = cv::Mat::zeros(target_height, target_width, img.type());
	outMat.setTo(cv::Scalar(255, 255, 255));

	int width = img.cols, height = img.rows;
	float ratio = (float)width / (float)height;
	float target_ratio = (float)target_width / (float)target_height;
	float scale;
	cv::Rect roi;

	if (ratio >= target_ratio)
	{
		scale = (float)target_width / width;
		roi.width = target_width;
		roi.x = 0;
		roi.height = height * scale;
		roi.y = (target_height - roi.height) / 2;
	}
	else
	{
		scale = (float)target_height / height;
		roi.y = 0;
		roi.height = target_height;
		roi.width = width * scale;
		roi.x = (target_width - roi.width) / 2;
	}

	cv::resize(img, outMat(roi), roi.size());
	return outMat;
}

void overlay(Mat& mat_tmp)
{
	Point topleft, bottomright;
	topleft.x = 0;
	topleft.y = src.rows * 0.5;
	bottomright.x = src.cols;
	bottomright.y = src.rows;
	Scalar colour(0, 0, 0);
	float alpha = 0.5;

	Mat overlay = mat_tmp.clone();
	rectangle(overlay, topleft, bottomright, colour, -1);

	addWeighted(overlay, alpha, mat_tmp, 1 - alpha, 0, mat_tmp);
}

void contrast_callback(int, void*)
{
	Mat dst = Mat::zeros(src.size(), src.type());

	float contrast_float = (float)contrast / 10;

	src.convertTo(dst, -1, contrast_float, brightness);

	namedWindow("Contours", CV_WINDOW_AUTOSIZE);
	imshow("Contours", dst);
}

void brightness_callback(int, void*)
{
	Mat dst = Mat::zeros(src.size(), src.type());

	float contrast_float = (float)contrast / 10;

	src.convertTo(dst, -1, contrast_float, brightness);

	namedWindow("Contours", CV_WINDOW_AUTOSIZE);
	imshow("Contours", dst);
}

void thresh_callback(int, void*)
{
	Mat canny_output;
	// Find contours
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	//Canny(dst, canny_output, 0, 50, 5);
	Canny(dst, canny_output, thresh, thresh * 2, 3);

	//findContours(canny_output, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	vector<Point> approx;
	//Mat dst = canny_output.clone();
	Mat dst = Mat::zeros(canny_output.size(), CV_8UC3);

	printf("contours.size: %d\n", contours.size());

	vector<Point> merged_contours;
	//vector<double> area;
	vector<int> boundRectArea(contours.size(), 0);

	// iterate through each contour.
	for (int i = 0; i < contours.size(); i = hierarchy[i][0])
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		//drawContours(dst, contours, i, color, 2, 8, hierarchy, 0, Point());

		// Skip small or non-convex objects 
		double area = fabs(contourArea(contours[i]));
		//area[i] = fabs(contourArea(contours[i]));

		if (area < 10 /*|| !isContourConvex(approx)*/)
		{
			//printf("contours[%d] area: %f is too small, skip!\n", i, area);
			continue;
		}
		//else
		//{
			printf("contours[%d] area: %f\n", i, area);
		//}

		Rect r = boundingRect(contours[i]);
		//int boundRectArea = r.area
		//printf("boundingRect[%d] area: %d\n", i, r.area());
		boundRectArea[i] = r.area();
		//drawContours(dst, contours, i, color, 2, 8, hierarchy, 0, Point());

		if (r.area() > 20000)
		{
			// Check if there is a child contour
			if (hierarchy[i][2] < 0)
			{
				printf("contours[%d] is an opened contour!\n", i);
				rectangle(dst, Point(r.x - 10, r.y - 10), Point(r.x + r.width + 10, r.y + r.height + 10), Scalar(0, 0, 255), 2, 8, 0); //Opened contour
			}
			else
			{
				printf("contours[%d] is a closed contour!\n", i);
				rectangle(dst, Point(r.x - 10, r.y - 10), Point(r.x + r.width + 10, r.y + r.height + 10), Scalar(0, 255, 0), 2, 8, 0); //closed contour
			}
			setLabel(dst, to_string(i), contours[i]);
			drawContours(dst, contours, i, color, 2, 8, hierarchy, 0, Point());

			printf("contours[%d] bound rect area: %d\n", i, r.area());
		}
	}

	for (int i = 0; i < contours.size(); i = hierarchy[i][0])
	{
		//printf("boundRectArea[%d]: %d\n", i, boundRectArea[i]);
		if (boundRectArea[i] > 20000 && hasNearbyBigRect(boundRectArea, i))
		{
			for (int j = 0; j < contours[i].size(); j++)
			{
				merged_contours.push_back(contours[i][j]);
			}
		}
	}

	if (!merged_contours.empty())
	{
		vector<Point> hull;
		convexHull((Mat)merged_contours, hull);
		//Mat hull_points(hull);
		//RotatedRect r_2 = minAreaRect(hull_points);

		//Point2f vertices[4];
		//r_2.points(vertices);

		//for (int i = 0; i < 4; ++i)
		//{
		//	line(dst, vertices[i], vertices[(i + 1) % 4],::Scalar(0, 255, 0), 8, CV_AA);
		//}

		Rect r_2 = boundingRect(hull);

		rectangle(dst, Point(r_2.x - 10, r_2.y - 10), Point(r_2.x + r_2.width + 10, r_2.y + r_2.height + 10), Scalar(0, 255, 0), 2, 8, 0); //closed contour
	}

	// Show in a window
	namedWindow("Contours", CV_WINDOW_AUTOSIZE);
	imshow("Contours", dst);
}

/**
* Helper function to find a cosine of angle between vectors
* from pt0->pt1 and pt0->pt2
*/
static double angle(Point pt1, Point pt2, Point pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

/**
* Helper function to display text in the center of a contour
*/
void setLabel(Mat& im, const string label, vector<Point>& contour)
{
	int fontface = FONT_HERSHEY_SIMPLEX;
	double scale = 0.4;
	int thickness = 1;
	int baseline = 0;

	Size text = getTextSize(label, fontface, scale, thickness, &baseline);
	Rect r = boundingRect(contour);

	Point pt(r.x + ((r.width - text.width) / 2), r.y + ((r.height + text.height) / 2));
	rectangle(im, pt + Point(0, baseline), pt + Point(text.width, -text.height), CV_RGB(255, 255, 255), CV_FILLED);
	putText(im, label, pt, fontface, scale, CV_RGB(0, 0, 0), thickness, 8);
}

bool hasNearbyBigRect(vector<int> vector_tmp, int index)
{
	for (int i = 1; i < 5; i++)
	{
		if (index - i > 0)
		{
			if (vector_tmp[index - i] > 20000)
			{
				printf("contours[%d] has nearby big rect\n", index);
				return true;
			}
		}
	}

	for (int i = 1; i < 5; i++)
	{
		if (vector_tmp[index + i] > 20000)
		{
			printf("contours[%d] has nearby big rect\n", index);
			return true;
		}
	}
	return false;
}

///======================================== blur demo ==============================================
///// Global Variables
//int DELAY_CAPTION = 1000;
//int DELAY_BLUR = 200;
////int MAX_KERNEL_LENGTH = 31;
//
////Mat src; 
//Mat dst;
//char window_name[] = "Filter Demo 1";
//
///// Function headers
//int display_caption(char* caption);
//int display_dst(int delay);
//
//int main(int argc, char* argv[])
//{
//	namedWindow(window_name, CV_WINDOW_AUTOSIZE);
//
//	/// Load the source image
//	src = imread(argv[1], 1);
//
//	cvtColor(src, src, CV_BGR2GRAY);
//
//	if (display_caption("Original Image") != 0) { return 0; }
//
//	dst = src.clone();
//	if (display_dst(DELAY_CAPTION) != 0) { return 0; }
//
//	/// Applying Homogeneous blur
//	if (display_caption("Homogeneous Blur") != 0) { return 0; }
//
//	for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
//	{
//		blur(src, dst, Size(i, i), Point(-1, -1));
//		if (display_dst(DELAY_BLUR) != 0) { return 0; }
//	}
//
//	/// Applying Gaussian blur
//	if (display_caption("Gaussian Blur") != 0) { return 0; }
//
//	for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
//	{
//		GaussianBlur(src, dst, Size(i, i), 0, 0);
//		if (display_dst(DELAY_BLUR) != 0) { return 0; }
//	}
//
//	/// Applying Median blur
//	if (display_caption("Median Blur") != 0) { return 0; }
//
//	for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
//	{
//		medianBlur(src, dst, i);
//		if (display_dst(DELAY_BLUR) != 0) { return 0; }
//	}
//
//	/// Applying Bilateral Filter
//	if (display_caption("Bilateral Blur") != 0) { return 0; }
//
//	for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
//	{
//		bilateralFilter(src, dst, i, i * 2, i / 2);
//		if (display_dst(DELAY_BLUR) != 0) { return 0; }
//	}
//
//	/// Wait until user press a key
//	display_caption("End: Press a key!");
//
//	waitKey(0);
//	return 0;
//}
//
//int display_caption(char* caption)
//{
//	dst = Mat::zeros(src.size(), src.type());
//	putText(dst, caption,
//		Point(src.cols / 4, src.rows / 2),
//		CV_FONT_HERSHEY_COMPLEX, 1, Scalar(255, 255, 255));
//
//	imshow(window_name, dst);
//	int c = waitKey(DELAY_CAPTION);
//	if (c >= 0) { return -1; }
//	return 0;
//}
//
//int display_dst(int delay)
//{
//	imshow(window_name, dst);
//	int c = waitKey(delay);
//	if (c >= 0) { return -1; }
//	return 0;
//}

///======================================== find folder ==============================================
//int FindFolder(string, int);

//#define FOLD 16

//typedef struct
//{
//	string FolderName;
//	int Foldersize;
//	int FolderFiles;
//}DeskFold;
//
//int files;

//int main(int argc, char *argv[])
//{
//	/* DELCARATIONS */
//	WIN32_FIND_DATA FindFileData;
//	char cArr[1000];
//	int icount = 0;
//	int i;
//	string sTmp;
//
//	HWND hwnd;
//	HANDLE hFind;
//
//	vector<DeskFold> MyVect;
//	DeskFold SObj;
//	/* Finds the path of desktop */
//	//SHGetSpecialFolderPath(hwnd, cArr, CSIDL_DESKTOP, 0);
//	sprintf_s(cArr, "%s/*.*", "C:\\Users\\Vince\\Desktop\\_TestPhoto");
//
//	//sTmp.assign(cArr);
//	//sTmp.append("\\");
//	//string sPath;
//	//sPath.assign(sTmp);
//	//sPath.append("*");
//
//	printf("path: %s\n", cArr);
//	/* Finds the Files and Folder inside the specified desktop */
//	hFind = FindFirstFile(cArr, &FindFileData);
//	do
//	{
//		if (FindFileData.dwFileAttributes == FOLD) // Get access only on folders
//		{
//			icount++;
//			if (icount > 2)
//			{
//				SObj.FolderName.assign(FindFileData.cFileName);
//				files = 0;
//				SObj.Foldersize = FindFolder(FindFileData.cFileName, 0);
//				SObj.FolderFiles = files;
//				MyVect.push_back(SObj);
//			}
//		}
//	} while (FindNextFile(hFind, &FindFileData));
//
//	FindClose(hFind);
//
//	for (i = 0; i<MyVect.size(); i++)
//	{
//		printf("Folder name: %s\n", MyVect.at(i).FolderName.data());
//		printf("Folder size: %d\n", MyVect.at(i).Foldersize);
//		printf("Folder files: %d\n", MyVect.at(i).FolderFiles);
//	}
//	printf("\nTotal number of folders in %s : %d\n", cArr, icount - 2);
//
//	return 0;
//}

//int FindFolder(string FileName, int foldersize)
//{
//	int icount = 0;
//	WIN32_FIND_DATA FindFileData;
//	HANDLE hFind;
//	char cArr[1000];
//	sprintf_s(cArr, "%s/*.*", FileName.c_str());
//
//	printf("sub path: %s\n", cArr);
//	hFind = FindFirstFile(cArr, &FindFileData);
//	do
//	{
//		foldersize += FindFileData.nFileSizeLow;
//		if (FindFileData.dwFileAttributes == FOLD)
//		{
//			icount++;
//			if (icount > 2)
//				FindFolder(FindFileData.cFileName, FindFileData.nFileSizeLow);
//		}
//		else
//			files++;
//	} while (FindNextFile(hFind, &FindFileData));
//	FindClose(hFind);
//
//	printf("\nTotal number of folders in %s : %d\n", cArr, icount - 2);
//
//	return foldersize;
//}