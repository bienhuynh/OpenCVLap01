// 1512031_Lab01.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include"Converter.h"
#include"ColorTransformer.h"
using namespace cv;
using namespace std;

#define INPUT "E:\\Projects\\ktmt-project1\\OpenCVLap01\\img\\lena.png"
#define OUTPUT "E:\\Projects\\ktmt-project1\\OpenCVLap01\\img\\lenaOuput.png"
///ten cau lenh
#define rgb2gray "--rgb2gray"
#define gray2rgb  "--gray2rgb"
#define rgb2hsv "--rgb2hsv"
#define hsv2rgb "--hsv2rgb"
#define bright "--bright"
#define contrast "--contrast"
#define hist "--hist"
#define equalhist "--equalhist"


int main(int argc, const char * argv[]) 
{

	if (argc < 3)
	{
		cout << "Chua du tham so." << endl;
		return 0;
	}

	Mat imgSource;//chứa ảnh input
	Mat imgOutput;//chứa ảnh output ra màn hình
	Converter converter;//khai báo đối tượng Converter
	ColorTransformer colorTranfer;
	
	namedWindow("Show Source image", CV_WINDOW_AUTOSIZE);//tạo cửa sổ với tên “Show Source image”
	namedWindow("Show image", CV_WINDOW_AUTOSIZE);////tạo cửa sổ với tên “Show image”
	
	string command = argv[1];//chứa tên câu lệnh từ tham số dòng lệnh
	string inputPath = argv[2];//chứa đường dẫn file ảnh input từ tham số dòng lệnh
	string commandArguments;//chứa Tham số câu lệnh từ tham số dòng lệnh
	
	imgSource = imread(inputPath);//đọc ảnh nguồn lên
	imshow("Show Source image", imgSource);//hiển thị ảnh lên cửa sổ “Show Source image”

	if (command == rgb2gray) // chuyển ảnh màu RGB sang ảnh xám GrayScale
	{
		int res = converter.Convert(imgSource, imgOutput, 0);
		cout << res << endl;
		if (res == 1) //lỗi
		{
			return 0;
		}
	}
	else if(command == gray2rgb) // chuyển ảnh màu GrayScale sang ảnh xám RGB
	{
		int res = converter.Convert(imgSource, imgOutput, 1);
		cout << res << endl;
		if (res == 1) //lỗi
		{
			return 0;
		}
	}
	else if (command == rgb2hsv)//Chuyển ảnh từ hệ màu RGB sang hệ màu HSV
	{
		int res = converter.Convert(imgSource, imgOutput, 2);
		cout << res << endl;
		if (res == 1) //lỗi
		{
			return 0;
		}
	}
	else if (command == hsv2rgb)//Chuyển ảnh từ hệ màu HSV sang hệ màu RGB 
	{
		int res = converter.Convert(imgSource, imgOutput, 3);
		cout << res << endl;
		if (res == 1) //lỗi
		{
			return 0;
		}
	}
	else if (command == bright)
	{
		if (argc == 4)
		{
			commandArguments = argv[3];//lấy độ sáng
		}
		int res = colorTranfer.ChangeBrighness(imgSource, imgOutput, (uchar)atoi(commandArguments.c_str()));
		cout << res << endl;
		if (res == 1)
		{	
			imwrite("/lenabright.png",imgOutput);
		}
		else
		{
			return 0;
		}
	}
	else if (command == contrast)
	{
		if (argc == 4)
		{
			commandArguments = argv[3];//lấy độ tương phản
		}
		int res = colorTranfer.ChangeContrast(imgSource, imgOutput, (uchar)atoi(commandArguments.c_str()));
		cout << res << endl;
		if (res == 1)
		{
			imwrite("/lenacontrast.png", imgOutput);
		}
		else
		{
			return 0;
		}
	}
	else if (command == hist)
	{
		MatNode* matnodea;//con trỏ tham chiếu lược đồ Histogram
		MatNode nodeHistB//Lưu danh sách lược đồ histogram
			, nodeHistG
			, nodeHistR;
		nodeHistB.pnext = &nodeHistG;
		nodeHistG.pnext = &nodeHistR;
		nodeHistR.pnext = NULL;
		matnodea = &nodeHistB;
		int res = colorTranfer.DrawHistogram(imgSource, matnodea);
		int k = 1;

		for (MatNode* node = matnodea; node != NULL; node = node->pnext)
		{
			string name = "Histogram " + k;
			namedWindow(name, CV_WINDOW_AUTOSIZE);
			imshow(name, node->node);
			k++;
		}
		cout << res << endl;
		waitKey();
		return 0;
	}
	else if (command == equalhist)
	{
		int res = colorTranfer.HistogramEqualization(imgSource, imgOutput);
		cout << res << endl;
		if (res == 1)
		{
			
		}
		else
		{
			return 0;
		}
	}

	
	imshow("Show image", imgOutput);//hiển thị ảnh lên cửa sổ “Show Image”
	waitKey();
	return 0;
}
