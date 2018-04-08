#include "stdafx.h"
#include"ColorTransformer.h"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

typedef struct RGBColor
{
	char R; //Red
	char G; // green
	char B; //Blue
};

bool CheckImageGrayScale(const Mat& imgage)
{
	bool checkImageGray = true;//cờ ảnh xám, nếu true là ảnh xám nếu false là ảnh màu. mặc định là true
							   //kiểm tra ảnh xám hay ảnh màu
	for (int y = 0; y < imgage.rows; y++) 
	{
		for (int x = 0; x < imgage.cols; x++) 
		{
			int b = imgage.at<Vec3b>(y, x)[0];
			int g = imgage.at<Vec3b>(y, x)[1];
			int r = imgage.at<Vec3b>(y, x)[2];
			if (!(b == g && b == r))
			{
				checkImageGray = false;
				break;
			}
		}
		if (!checkImageGray) break;
	}
	return checkImageGray;
}
/*
-Tạo ma trận ảnh histogram
biểu đồ dạng cột, nền trắng
Tham so: 
	H: mảng histogram một chiều
	size: kích cỡ biều đồ đầu vào
	color: màu của cột
	output: biểu đồ histogram ở dang ảnh vuông (mặc định 255x255)
Return:
	1: thành công
	0: có lỗi
*/
int DrawMatHistogram(int *H, Size size, RGBColor color, Mat& output)
{	
	if (H == NULL)
	{
		return 0;
	}
	Mat hist(Size(size.width,size.width), CV_8UC3);
	//khởi tạo nền trắng
	for (size_t i = 0; i < size.width; i++)
	{
		hist.at<Vec3b>(Point(i, 0))[0] = 255;
		hist.at<Vec3b>(Point(i, 0))[1] = 255;
		hist.at<Vec3b>(Point(i, 0))[2] = 255;
	}

	//vẽ biểu đồ theo màu
	int p = size.height / size.width + 1;//tỉ lệ cột. p chứa tỉ lệ một pixel của ảnh output bằng p giá trị historgram nhằm giảm tỉ lệ ảnh theo tỉ số p
	
	for (size_t i = 0; i < size.width; i++)//duyệt màng H
	{
		float high = H[i] / p;
		for (size_t j = 0; j < high; j++)//vẽ cột trên ảnh output
		{
			hist.at<Vec3b>(Point(j, i))[0] = color.B;
			hist.at<Vec3b>(Point(j, i))[1] = color.G;
			hist.at<Vec3b>(Point(j, i))[2] = color.R;
		}
	}
	output = hist;
	return 1;
}
//Hàm tính Tần số tích lũy
void cumhist(int histogram[], int cumhistogram[])
{
	cumhistogram[0] = histogram[0];

	for (int i = 1; i < 256; i++)
	{
		cumhistogram[i] = histogram[i] + cumhistogram[i - 1];
	}
}

int ColorTransformer::ChangeBrighness(const Mat& sourceImage, Mat& destinationImage, uchar b)
{
	if (sourceImage.empty())//kiểm tra ảnh có null không
	{
		return 0; //anh khong ton tai
	}
	destinationImage = sourceImage.clone();//sao chép ảnh đầu vào sang output
	for (int y = 0; y < sourceImage.rows; y++) 
	{
		for (int x = 0; x < sourceImage.cols; x++)
		{
			for (int channel = 0; channel < 3; channel++) 
			{
				destinationImage.at<Vec3b>(y, x)[channel] = saturate_cast<uchar>//saturate_cast dùng để tránh tràn số cho các kenh màu vì các kênh màu chỉ nhận giá trị trong khoản [0;255]
					((destinationImage.at<Vec3b>(y, x)[channel]) + b);//tính toán độ sáng mới cho điểm ảnh trên ảnh output
			}
		}
	}

	return 1;//tăng giảm độ sáng thành công
}

int ColorTransformer::ChangeContrast(const Mat& sourceImage, Mat& destinationImage, float c)
{
	if (sourceImage.empty())//kiểm tra ảnh có null không
	{
		return 0; //anh khong ton tai
	}
	destinationImage = sourceImage.clone();//sao chép ảnh đầu vào sang output
	for (int y = 0; y < sourceImage.rows; y++) 
	{
		for (int x = 0; x < sourceImage.cols; x++)
		{
			for (int channel = 0; channel < 3; channel++)
			{
				destinationImage.at<Vec3b>(y, x)[channel] = saturate_cast<uchar>//saturate_cast dùng để tránh tràn số cho các kenh màu vì các kênh màu chỉ nhận giá trị trong khoản [0;255]
					(c*(destinationImage.at<Vec3b>(y, x)[channel]));//tính toán độ sáng mới cho điểm ảnh trên ảnh output
			}
		}
	}

	return 1;//tăng giảm độ sáng thành công
}

int ColorTransformer::HistogramEqualization(const Mat& sourceImage, Mat& destinationImage)
{
	if (sourceImage.empty())//kiểm tra ảnh có null không
	{
		return 0; //anh khong ton tai
	}
	destinationImage = sourceImage.clone();
	int histogram[256];
	// khởi tạo giá trị 0 cho mảng histogram
	for (int i = 0; i < 256; i++)
	{
		histogram[i] = 0;
	}

	//Tính lược đồ độ xám của ảnh f, lưu vào Hf
	for (int y = 0; y < destinationImage.rows; y++)
		for (int x = 0; x < destinationImage.cols; x++)
			histogram[(int)destinationImage.at<uchar>(y, x)]++;
	//Tính lược đồ độ xám tích lũy của f, lưu vào T
	
	int size = destinationImage.rows * destinationImage.cols;// tính diện tích ảnh
	float alpha = 255.0 / size;//giá trị alpha : tỉ lệ thu nhỏ biểu đổ Histogram sau khi tinh được, bược này giúp bỏ qua chuẩn hóa giá trị túy lũy

	// Tính toán xác suất của mỗi cường độ điểm ảnh
	float PrRk[256];
	for (int i = 0; i < 256; i++)
	{
		PrRk[i] = (double)histogram[i] / size;
	}
	//Tính lược đồ độ xám tích lũy của f, lưu vào T
	int T[256];
	cumhist(histogram, T);
	
	int Sk[256];
	//giảm tỉ lệ tích lũy và chuẩn hóa T về đoạn [0;nG-1] nG=255 lưu vào biến mới Sk 
	for (int i = 0; i < 256; i++)
	{
		Sk[i] = cvRound((double)T[i] * alpha);
	}

	//Tạo ảnh mới và copy lại ảnh cũ, mục đích
	Mat new_image = destinationImage.clone();

	for (int y = 0; y < destinationImage.rows; y++)
		for (int x = 0; x < destinationImage.cols; x++)
			new_image.at<uchar>(y, x) = saturate_cast<uchar>(Sk[destinationImage.at<uchar>(y, x)]);
	destinationImage = new_image;//xuất ảnh ra output
	//free(histogram);
	return 1;//không tạo được ảnh
}

int ColorTransformer::CalcHistogram(const Mat& sourceImage, MatNode* histogram)
{
	if (sourceImage.empty())//kiểm tra ảnh có null không
	{
		return 0; //anh khong ton tai
	}
	int imaxWith = 255;//chiều rộng biểu đồ Ouput

	int **H;//khởi tạo mảng histogram, chứa 1 hoặc 3 kênh màu
			//kiểm tra ảnh xám hay ảnh màu
	bool isGray = CheckImageGrayScale(sourceImage);
	if (isGray) // nếu là ảnh xám
	{
		int imaxHeight = 0;//chiều cao biểu đồ Histogram kênh Xám Output tạm thời
		H = new int*[1];
		H[0] = new int[imaxWith];
		//khởi tạo giá trị ban đầu là 0
		for (size_t i = 0; i < imaxWith; i++)
		{
			H[0][i] = 0;
		}

		uchar index;//index của mảng H

		for (int y = 0; y < sourceImage.rows; y++)
		{
			for (int x = 0; x < sourceImage.cols; x++)
			{
				index = sourceImage.at<uchar>(x, y);//lấy giá trị màu cũng như là index của H
				H[0][index]++;//tăng histogram lên 1
				if (H[0][index] > imaxHeight)//kiểm tra imaxHeight có phải là giá trị lớn nhất nữa không
				{
					imaxHeight = H[0][index];//gán giá trị max tạm thời
				}
			}
		}

		Mat Histout;
		RGBColor color;//màu vẽ cột là mầu đen
		color.B = 0;
		color.G = 0;
		color.R = 0;
		DrawMatHistogram(H[0], Size(imaxWith, imaxHeight), color, Histout);//vẽ histogram ảnh xám
		histogram->node = Histout;
		histogram->pnext = NULL;
	}
	else // nếu là ảnh màu
	{
		int imaxHeightB = 0;//chiều cao biểu đồ Histogram kênh B Output tạm thời
		int imaxHeightG = 0;//chiều cao biểu đồ Histogram kênh G Output tạm thời
		int imaxHeightR = 0;//chiều cao biểu đồ Histogram kênh R Output tạm thời
		H = new int*[3];//3 kênh màu RGB
		H[0] = new int[imaxWith];//kênh màu B
		H[1] = new int[imaxWith];//kênh màu G
		H[2] = new int[imaxWith];//kênh màu R
								 //khởi tạo giá trị ban đầu là 0
		for (size_t i = 0; i < imaxWith; i++)
		{
			H[0][i] = 0;
			H[1][i] = 0;
			H[2][i] = 0;
		}

		uchar indexB, indexG, indexR;//index của mảng H[0] H[1] H[2]

		for (int y = 0; y < sourceImage.rows; y++)
		{
			for (int x = 0; x < sourceImage.cols; x++)
			{
				Vec3b color = sourceImage.at<Vec3b>(Point(x, y));
				//lấy màu của từng kênh
				indexB = color.val[0];
				indexG = color.val[1];
				indexR = color.val[2];
				//tính histogram
				H[0][indexB]++;
				H[1][indexG]++;
				H[2][indexR]++;
				if (H[0][indexB] > imaxHeightB)
				{
					imaxHeightB = H[0][indexB];
				}
				if (H[1][indexG] > imaxHeightG)
				{
					imaxHeightG = H[1][indexG];
				}
				if (H[2][indexR] > imaxHeightR)
				{
					imaxHeightR = H[2][indexR];
				}
			}
		}

		//Tạo biểu đồ Histogram o dang ảnh

		RGBColor colorB; // màu xanh dương
		colorB.R = 5;
		colorB.G = 79;
		colorB.B = 198;

		DrawMatHistogram(H[0], Size(imaxWith, imaxHeightB), colorB, (histogram->node));//tạo sơ đồ ảnh kenh B
		RGBColor colorG;//màu xanh lá
		colorG.B = 34;
		colorG.G = 188;
		colorG.R = 11;
		DrawMatHistogram(H[1], Size(imaxWith, imaxHeightG), colorG, histogram->pnext->node);//tạo sơ đồ ảnh kenh G
		RGBColor colorR;//màu đỏ
		colorR.B = 4;
		colorR.G = 4;
		colorR.R = 232;
		DrawMatHistogram(H[2], Size(imaxWith, imaxHeightR), colorR, histogram->pnext->pnext->node);//tạo sơ đồ ảnh kenh R
		
	}


	return 1;//tinh toan histogram thanh cong
}

int ColorTransformer::CalcHistogram(const Mat& sourceImage, Mat& histogram)
{
	if (sourceImage.empty())//kiểm tra ảnh có null không
	{
		return 0; //anh khong ton tai
	}
	int imaxWith = 255;//chiều rộng biểu đồ Ouput
	
	int **H;//khởi tạo mảng histogram, chứa 1 hoặc 3 kênh màu
	//kiểm tra ảnh xám hay ảnh màu
	bool isGray = CheckImageGrayScale(sourceImage);
	if (isGray) // nếu là ảnh xám
	{
		int imaxHeight = 0;//chiều cao biểu đồ Histogram kênh Xám Output tạm thời
		H = new int*[1];
		H[0] = new int[imaxWith];
		//khởi tạo giá trị ban đầu là 0
		for (size_t i = 0; i < imaxWith; i++)
		{
			H[0][i] = 0;
		}
		
		uchar index;//index của mảng H
		
		for (int y = 0; y < sourceImage.rows; y++)
		{
			for (int x = 0; x < sourceImage.cols; x++)
			{
				index = sourceImage.at<uchar>(x, y);
				H[0][index]++;
				if (H[0][index] > imaxHeight)
				{
					imaxHeight = H[0][index];
				}
			}
		}
		RGBColor color;
		color.B = 0;
		color.G = 0;
		color.R = 0;
		DrawMatHistogram(H[0], Size(imaxWith, imaxHeight), color, histogram);
	}
	else // nếu là ảnh màu
	{
		int imaxHeightB = 0;//chiều cao biểu đồ Histogram kênh B Output tạm thời
		int imaxHeightG = 0;//chiều cao biểu đồ Histogram kênh G Output tạm thời
		int imaxHeightR = 0;//chiều cao biểu đồ Histogram kênh R Output tạm thời
		H = new int*[3];//3 kênh màu RGB
		H[0] = new int[imaxWith];//kênh màu B
		H[1] = new int[imaxWith];//kênh màu G
		H[2] = new int[imaxWith];//kênh màu R
		//khởi tạo giá trị ban đầu là 0
		for (size_t i = 0; i < imaxWith; i++)
		{
			H[0][i] = 0;
			H[1][i] = 0;
			H[2][i] = 0;
		}

		uchar indexB,indexG,indexR;//index của mảng H[0] H[1] H[2]

		for (int y = 0; y < sourceImage.rows; y++)
		{
			for (int x = 0; x < sourceImage.cols; x++)
			{
				Vec3b color = sourceImage.at<Vec3b>(Point(x, y));
				//lấy màu của từng kênh
				indexB = color.val[0];
				indexG = color.val[1];
				indexR = color.val[2];
				//tính histogram
				H[0][indexB]++;
				H[1][indexG]++;
				H[2][indexR]++;
				if (H[0][indexB] > imaxHeightB)
				{
					imaxHeightB = H[0][indexB];
				}
				if (H[1][indexG] > imaxHeightG)
				{
					imaxHeightG = H[1][indexG];
				}
				if (H[2][indexR] > imaxHeightR)
				{
					imaxHeightR = H[2][indexR];
				}
			}
		}
		
		//Tạo biểu đồ Histogram o dang ảnh
		//MatNode matnode;
		Mat HistB;
		Mat HistG;
		Mat HistR;
		//matnode.node = &HistB;
		//matnode.pnext->node = &HistG;

		RGBColor colorB; // màu xanh dương
		colorB.R = 5;
		colorB.G = 79;
		colorB.B = 198;
		
		DrawMatHistogram(H[0], Size(imaxWith, imaxHeightB), colorB, HistB);//tạo sơ đồ ảnh kenh B
		RGBColor colorG;//màu xanh lá
		colorG.B = 34;
		colorG.G = 188;
		colorG.R = 11;
		DrawMatHistogram(H[1], Size(imaxWith, imaxHeightG), colorG, HistG);//tạo sơ đồ ảnh kenh G
		RGBColor colorR;//màu đỏ
		colorR.B = 4;
		colorR.G = 4;
		colorR.R = 232;
		DrawMatHistogram(H[2], Size(imaxWith, imaxHeightR), colorR, HistR);//tạo sơ đồ ảnh kenh R
		histogram = HistB;
	}


	return 1;//tinh toan histogram thanh cong
}

int ColorTransformer::DrawHistogram(const Mat& sourceImage, Mat& histImage)
{
	//namedWindow("Histogram gray",CV_WINDOW_AUTOSIZE);
	CalcHistogram(sourceImage, histImage);
	//imshow("Histogram gray", histImage);//hiển thị ảnh lên cửa sổ “Show Image”
	return 0;
}

int ColorTransformer::DrawHistogram(const Mat& sourceImage, MatNode* histImage)
{
	CalcHistogram(sourceImage, histImage);
	return 0;
}

ColorTransformer::ColorTransformer()
{

}
ColorTransformer::~ColorTransformer()
{

}
