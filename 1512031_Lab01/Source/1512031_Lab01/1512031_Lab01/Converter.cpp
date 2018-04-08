#include "stdafx.h"
#include"Converter.h"

int Converter::RGB2GrayScale(const Mat& sourceImage, Mat& destinationImage)
{
	//kiểm tra ảnh đầu vao có null hay không
	if (sourceImage.empty())
		return 1;//nếu ảnh null trả về 1

	Mat tmp(sourceImage.size(), CV_8UC1);//tạo một ảnh xám để chứa kết quá tính toán về ảnh xám
	destinationImage = tmp;//gán kết quả ảnh xám cho biến con trả trả về

	for (int y = 0; y < sourceImage.rows; y++)//duyệt các hàng
	{
		for (int x = 0; x < sourceImage.cols; x++)//duyệt các cột 
		{
			Vec3b color = sourceImage.at<Vec3b>(Point(x, y));//lấy giá trị RGB tại điểm (x,y)
			int blue = color.val[0];//lấy giá trị màu Blue
			int green = color.val[1];//lấy giá trị màu Green
			int red = color.val[2];//lấy giá trị màu Red
			//Áp dụng công thức chuyển đổi ảnh xám I(x, y) = (2 * Red(x, y) + 5 * Green(x, y) + 1 * Blue(x, y)) / 8
			destinationImage.at<uchar>(y, x) = (blue + green*5 + red*2)/8;
		}
	}
	return 0;//thành công
}

int Converter::GrayScale2RGB(const Mat& sourceImage, Mat& destinationImage)
{
	if (sourceImage.empty())
		return 1;

	Mat tmp(sourceImage.size(), CV_8UC1);
	destinationImage = Mat(sourceImage.size(),CV_8UC3);

	for (int y = 0; y < sourceImage.rows; y++)
	{
		for (int x = 0; x < sourceImage.cols; x++)
		{
			//lấy vector màu RGB tại điểm (y,x) của ảnh nguồn
			Vec3b color = sourceImage.at<Vec3b>(Point(x, y));
			//lấy giá trị các kênh màu R G B thực ra R = G = B
			float blue = color.val[0];
			float green = color.val[1];
			float red = color.val[2];
			//khôi phục từng kênh màu
			//ta có ẩn số (2/8)*red+(5/8)*green+(1/8)*blue = I(x,y)
			blue = blue / 8;
			green = (green * 8) / 5;
			red = red * 4;
			//gán lại giá trị cho các kenh màu vừa tính được
			color.val[0] = saturate_cast<uchar>(blue);
			color.val[1] = saturate_cast<uchar>(green);
			color.val[2] = saturate_cast<uchar>(red);
			destinationImage.at<Vec3b>(Point(x, y)) = color;
		}
	}
	return 0;
}

int Converter::RGB2HSV(const Mat& sourceImage, Mat& destinationImage)
{
	if (sourceImage.empty())//kiểm tra ảnh đầu vao có null hay không
	{
		return 1;//nếu ảnh null trả về 1
	}
	float fR, fG, fB;//lưu tạm hệ màu RGB
	float fH, fS, fV;//lưu hệ tạm màu HSV
	const float FLOAT_TO_BYTE = 255.0f;//hằng số 255 ở dạng số thực để sau khi nhân chia giảm bớt sai số tránh mất mát dữ liệu
	const float BYTE_TO_FLOAT = 1.0f / FLOAT_TO_BYTE;//tỉ số 1/255.
	
	IplImage iplimageRGB = sourceImage;//chuyển ảnh Mat sang ảnh IplImage
	IplImage *imageRGB = &iplimageRGB; //trỏ địa chỉ con trỏ IplImage đến data của tấm ảnh
									   //vì chúng ta không dùng trực tiếp biến tham trị iplimageRGB được
	//Tạo một tấm ảnh HSV rỗng
	IplImage *imageHSV = cvCreateImage(cvGetSize(imageRGB), 8, 3);
	//kiếm tra tấm ảnh HSV imageHSV có tạo được đúng kích thước, chiều sâu 8 và 3 kênh không
	if (!imageHSV || imageRGB->depth != 8 || imageRGB->nChannels != 3) 
	{
		printf("Lỗi, không tạo một tấm ảnh HSV trống được.\n");
		exit(1);//nếu không đúng thì thoát chương trình.
	}

	int h = imageRGB->height;		// số Pixel theo chiều cao.
	int w = imageRGB->width;		// số Pixel theo chiều rộng.
	int rowSizeRGB = imageRGB->widthStep;	// kích thước một hàng tính bằng byte của ảnh đầu vào RGB.
	char *imRGB = imageRGB->imageData;	// vị trí pixel bắt đầu làm việc tại vùng data của ảnh RGB đầu vào.
	int rowSizeHSV = imageHSV->widthStep;	// kích thước một hàng tính bằng byte của ảnh đầu ra HSV.
	char *imHSV = imageHSV->imageData;	// vị trí pixel bắt đầu ghi dữ liệu của ảnh HSV output.
	for (int y = 0; y<h; y++) {
		for (int x = 0; x<w; x++) {
			// Lấy giá trị màu RGB của pixel(x,y)
			uchar *pRGB = (uchar*)(imRGB + y*rowSizeRGB + x * 3);
			int bB = *(uchar*)(pRGB + 0);	// Blue
			int bG = *(uchar*)(pRGB + 1);	// Green
			int bR = *(uchar*)(pRGB + 2);	// Red

			// Chuyển các giá trị RGB màu 8bit kiểu interger sang kiểu float
			fR = bR * BYTE_TO_FLOAT;
			fG = bG * BYTE_TO_FLOAT;
			fB = bB * BYTE_TO_FLOAT;

			// Chuyển giá trị màu RGB sang HSV sử dụng kiểu float trong miền [0.0,1.0]
			float fDelta;//chứa giá trị biến thiên của |fMax-fMin|
			float fMin // chứa giá trị màu nhỏ nhất của một thành phần màu RGB của điểm poxel đang xét
				, fMax;//chứa giá trị màu lớn nhất của một thành phần màu RGB của điểm poxel đang xét
			int iMax;//lưu tạm giá trị màu cho là lớn nhất
			// so sánh các thành phần màu RGB của pixel đang xet để tìm ra giá trị màu lớn nhất nhỏ nhất
			if (bB < bG) 
			{
				if (bB < bR) 
				{
					fMin = fB;
					if (bR > bG) 
					{
						iMax = bR;
						fMax = fR;
					}
					else 
					{
						iMax = bG;
						fMax = fG;
					}
				}
				else 
				{
					fMin = fR;
					fMax = fG;
					iMax = bG;
				}
			}
			else 
			{
				if (bG < bR) 
				{
					fMin = fG;
					if (bB > bR) 
					{
						fMax = fB;
						iMax = bB;
					}
					else 
					{
						fMax = fR;
						iMax = bR;
					}
				}
				else 
				{
					fMin = fR;
					fMax = fB;
					iMax = bB;
				}
			}
			fDelta = fMax - fMin;//tính độ biến thiên
			fV = fMax;				//tìm được giá trị độ sáng V
			if (iMax != 0) 
			{			
				//kiểm tra đảm bảo nó không rơi vào điểm đen.
				fS = fDelta / fMax;		// điểm bão hòa.
				float ANGLE_TO_UNIT = 1.0f / (6.0f * fDelta);	// dùng để điều chình giá trị H (HSV) nằm trong vùng từ [0.0, 1.0] thôi thay vì từ [0.0, 1.0]
				if (iMax == bR) //nếu giá trị iMax tạm thời bằng giá trị màu Red (RGB)
				{		
					// thì giá trị H sẽ nằm giữa màu vàng và màu đỏ tươi.
					fH = (fG - fB) * ANGLE_TO_UNIT;
				}
				else if (iMax == bG) //nếu giá trị iMax tạm thời bằng giá trị màu Green (RGB)
				{		
					//thì giá trị H sẽ nằm giữa màu xanh lục và vàng.
					fH = (2.0f / 6.0f) + (fB - fR) * ANGLE_TO_UNIT;
				}
				else //nếu giá trị iMax tạm thời bằng giá trị màu Blue(RGB)
				{				
					//thì giá trị H sẽ nằm giữa màu đỏ tươi và màu lam.
					fH = (4.0f / 6.0f) + (fR - fG) * ANGLE_TO_UNIT;
				}
				// Giải quyết ngoại lệ, trường hợp giá trị H (HSV) không nằm tròng vùng mong đợi thì quay vòng giá trị H bằng cách +/- 1.0 tránh bị lỗi 
				if (fH < 0.0f) // nếu H (HSV) nhỏ hơn 0.0 tức là bị âm
					fH += 1.0f;//thì fH cộng thêm 1.0
				if (fH >= 1.0f)//nếu H(HSV) lớn hơn 1.0
					fH -= 1.0f; //thì fH trừ đi 1.0
			}
			else //ngược lại nếu imax khác 0 thì 
			{
				fS = 0;//chọn màu đen cho S (HSV)
				fH = 0;	//
			}

			// Chuyển hệ màu HSV kiểu float sang kiểu số nguyên 8 bit
			int bH = (int)(0.5f + fH * 255.0f);
			int bS = (int)(0.5f + fS * 255.0f);
			int bV = (int)(0.5f + fV * 255.0f);

			//Kiểm tra hệ màu HSV sau khi chuyển có bị rơi ra ngoài vùng [0;255] hay không
			if (bH > 255)//nếu H (HSV) lớn hơn 255 là gán lại 255
				bH = 255;
			if (bH < 0)//...tương tự
				bH = 0;
			if (bS > 255)
				bS = 255;
			if (bS < 0)
				bS = 0;
			if (bV > 255)
				bV = 255;
			if (bV < 0)
				bV = 0;

			// gán giá trị màu HSV vừa tìm được bên trên vào điểm tương ứng của tấm ảnh rỗng HSV vừa tạo ở trên
			uchar *pHSV = (uchar*)(imHSV + y*rowSizeHSV + x * 3);
			*(pHSV + 0) = bH;		// H component
			*(pHSV + 1) = bS;		// S component
			*(pHSV + 2) = bV;		// V component
		}
	}
	destinationImage = cvarrToMat(imageHSV);
	return 0;
}

int Converter::HSV2RGB(const Mat& sourceImage, Mat& destinationImage)
{
	if (sourceImage.empty())//kiểm tra ảnh đầu vao có null hay không
	{
		return 1;//nếu ảnh null trả về 1
	}

	float fH, fS, fV;//lưu tạm hệ màu RGB
	float fR, fG, fB;//lưu tạm hệ màu HSV

	const float FLOAT_TO_BYTE = 255.0f;//hằng số 255 ở dạng số thực để sau khi nhân chia giảm bớt sai số tránh mất mát dữ liệu
	const float BYTE_TO_FLOAT = 1.0f / FLOAT_TO_BYTE;//tỉ số 1/255.

	IplImage iplimageHSV = sourceImage;//chuyển ảnh Mat sang ảnh IplImage
	IplImage *imageHSV = &iplimageHSV; //trỏ địa chỉ con trỏ IplImage đến data của tấm ảnh
									   //vì chúng ta không dùng trực tiếp biến tham trị iplimageRGB được

									   
	IplImage *imageRGB = cvCreateImage(cvGetSize(imageHSV), 8, 3);//Tạo một tấm ảnh RGB rỗng dùng để chứa ảnh đầu ra
																  
	if (!imageRGB || imageHSV->depth != 8 || imageHSV->nChannels != 3)//kiếm tra tấm ảnh HSV imageHSV có tạo được đúng kích thước, chiều sâu 8 và 3 kênh không
	{
		printf("Lỗi, không tạo một tấm ảnh HSV trống được.\n");
		exit(1);//nếu không đúng thì thoát chương trình.
	}

	int h = imageRGB->height;		// số Pixel theo chiều cao.
	int w = imageRGB->width;		// số Pixel theo chiều rộng.
	int rowSizeRGB = imageRGB->widthStep;	// kích thước một hàng tính bằng byte của ảnh đầu vào RGB.
	char *imRGB = imageRGB->imageData;	// vị trí pixel bắt đầu làm việc tại vùng data của ảnh RGB đầu vào.
	int rowSizeHSV = imageHSV->widthStep;	// kích thước một hàng tính bằng byte của ảnh đầu ra HSV.
	char *imHSV = imageHSV->imageData;	// vị trí pixel bắt đầu ghi dữ liệu của ảnh HSV output.
	for (int y = 0; y<h; y++) 
	{
		for (int x = 0; x<w; x++)
		{
			// Lấy giá trị màu RGB của pixel(x,y)
			uchar *pHSV = (uchar*)(imHSV + y*rowSizeHSV + x * 3);
			int bH = *(uchar*)(pHSV + 0);	// H
			int bS = *(uchar*)(pHSV + 1);	// S
			int bV = *(uchar*)(pHSV + 2);	// V

			// Chuyển các giá trị HSV màu 8bit kiểu interger sang kiểu float
			fH = (float)bH * BYTE_TO_FLOAT;
			fS = (float)bS * BYTE_TO_FLOAT;
			fV = (float)bV * BYTE_TO_FLOAT;

			// Chuyển giá trị màu HSV sang RGB sử dụng kiểu float trong miền [0.0; 1.0]
			int iI;
			float fI, fF, p, q, t;

			if (bS == 0)//nếu giá trị S HSV bằng 0
			{
				//giá trị màu RGB gán bằng giá trị mức xám
				fR = fG = fB = fV;
			}
			else
			{
				// nếu Hue (HSV) == 1.0, thì quay vòng lại giá trị 0.0
				if (fH >= 1.0f)
					fH = 0.0f;

				fH *= 6.0;			// 0 - 5
				fI = floor(fH);		// lấy phần nguyên của giá trị H (0,1,2,3,4,5 hoặc 6)
				iI = (int)fH;			//		"		"		"		"
				fF = fH - fI;			// phần giai thừa của H (0 đến 1)

				p = fV * (1.0f - fS);
				q = fV * (1.0f - fS * fF);
				t = fV * (1.0f - fS * (1.0f - fF));

				switch (iI) 
				{
				case 0:
					fR = fV;
					fG = t;
					fB = p;
					break;
				case 1:
					fR = q;
					fG = fV;
					fB = p;
					break;
				case 2:
					fR = p;
					fG = fV;
					fB = t;
					break;
				case 3:
					fR = p;
					fG = q;
					fB = fV;
					break;
				case 4:
					fR = t;
					fG = p;
					fB = fV;
					break;
				default:		// case 5 (hoặc 6):
					fR = fV;
					fG = p;
					fB = q;
					break;
				}
			}

			// Chuyển hệ màu RGB vừa tìm được từ kiểu float về kiếu số nguyên 8 bit
			int bR = (int)(fR * FLOAT_TO_BYTE);
			int bG = (int)(fG * FLOAT_TO_BYTE);
			int bB = (int)(fB * FLOAT_TO_BYTE);

			// Kiểm tra giá trị màu RGB khỏi các ngoại lệ
			if (bR > 255)
				bR = 255;
			if (bR < 0)
				bR = 0;
			if (bG > 255)
				bG = 255;
			if (bG < 0)
				bG = 0;
			if (bB > 255)
				bB = 255;
			if (bB < 0)
				bB = 0;

			// Gán các thành phần RGB vào ảnh đầu ra
			uchar *pRGB = (uchar*)(imRGB + y*rowSizeRGB + x * 3);
			*(pRGB + 0) = bB;		// Blue
			*(pRGB + 1) = bG;		// Gren
			*(pRGB + 2) = bR;		// Red
		}
	}
	destinationImage = cvarrToMat(imageRGB);//gán địa chỉ ảnh output vào contror output
	return 0;
}

int Converter::Convert(Mat& sourceImage, Mat& destinationImage, int type)
{
	int res; 
	Mat tmp(sourceImage.size(), CV_8UC1);
	switch (type)
	{
	case 0://RGB to GrayScale
		res = RGB2GrayScale(sourceImage, destinationImage);
		break;
	case 1://GrayScale to RGB
		res = GrayScale2RGB(sourceImage, destinationImage); 
		break;
	case 2://RGB to HSV
		res = RGB2HSV(sourceImage, destinationImage);
		break;
	case 3://HSV to RGB
		res = HSV2RGB(sourceImage, destinationImage);
		break;

	default:
		return 1;
		break;
	}
	return res;
}
int Converter::Convert(IplImage* sourceImage, IplImage* destinationImage, int type)
{
	return 0;
}

Converter::Converter()
{

}
Converter::~Converter()
{

}