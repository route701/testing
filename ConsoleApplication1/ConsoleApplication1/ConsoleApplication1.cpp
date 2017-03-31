#include "stdafx.h"
#include <windows.h>
#include <Shlobj.h>
//#include <iostream.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <conio.h>
//shape-detect
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cmath>
#include <iostream>

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

//Function header
void setLabel(Mat& im, const string label, vector<Point>& contour);
static double angle(Point pt1, Point pt2, Point pt0);
void thresh_callback(int, void*);
void contrast_callback(int, void*);
void brightness_callback(int, void*);
bool hasNearbyBigRect(vector<int> vector_tmp, int index);
void overlay(Mat& mat_tmp);

int main(int argc, char* argv[])
{
	vector<unsigned char> output;
	vector<int> para;

	src = imread(argv[1], 1);
	dst = src.clone();

	//overlay(dst);

	//Increase contrast;
	src.convertTo(dst, -1, 4, -400);

	//Convert to grayscale
	cvtColor(dst, dst, CV_BGR2GRAY);
	medianBlur(dst, dst, 11);

	//Create Window
	char* source_window = "Source";
	namedWindow(source_window, CV_WINDOW_AUTOSIZE);
	imshow(source_window, src);

	char* modified_window = "Modified";
	namedWindow(modified_window, CV_WINDOW_AUTOSIZE);
	imshow(modified_window, dst);

	createTrackbar(" Canny thresh:", "Source", &thresh, max_thresh, thresh_callback);
	//createTrackbar(" contrast: ", "Source", &contrast, max_contrast, contrast_callback);
	//createTrackbar(" brightness: ", "Source", &brightness, max_brightness, brightness_callback);

	thresh_callback(0, 0);
	//contrast_callback(0, 0);
	//brightness_callback(0, 0);

	waitKey(0);
	return(0);
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
	//Find contours
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

	for (int i = 0; i < contours.size(); i = hierarchy[i][0]) //iterate through each contour.
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		//drawContours(dst, contours, i, color, 2, 8, hierarchy, 0, Point());

		//Skip small or non-convex objects 
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
			if (hierarchy[i][2] < 0) //Check if there is a child contour
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

	//Show in a window
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