#include"opencv2/highgui/highgui.hpp"
#include"opencv2/imgproc/imgproc.hpp"
#include<iostream>
using namespace cv;
using namespace std;
float remap(float x, float y, float sigma);
void getY(float *pX, float *pY, int len, float sigma, float lamba);

int main()
{
	Mat input8, inputImg, rowFImg, colFImg, outputImg;
	input8 = imread("F://picture/A.jpg");
	input8.convertTo(inputImg, CV_32FC3, 1 / 255.0);//uchar convert to double

	inputImg.copyTo(rowFImg);
	inputImg.copyTo(colFImg);
	inputImg.copyTo(outputImg);

	vector<Mat>input_ch;
	vector<Mat>rowF_ch;
	vector<Mat>colF_ch;
	vector<Mat>output_ch;
	Mat  input1, rowFI1, colFI1, output1, input1_T, rowFI1_T, colFI1_T;

	split(inputImg, input_ch);
	split(rowFImg, rowF_ch);
	split(colFImg, colF_ch);
	split(outputImg, output_ch);
	float lamba = 1.05, sigma = 14.0;
	int len = 0, row = 0, col = 0, ch = 0;
	row = inputImg.rows; col = inputImg.cols;
	ch = inputImg.channels();
	len = row*col;
	Mat X(1, len, CV_32FC1); //X序列
	Mat Y(1, len, CV_32FC1); //Y序列

							 //分别计算前向和后向序列
	for (int c = 0; c < ch; ++c)
	{
		input1 = input_ch.at(c);
		rowFI1 = Mat::zeros(row, col, CV_32FC1);
		colFI1 = Mat::zeros(row, col, CV_32FC1);
		output1 = Mat::zeros(row, col, CV_32FC1);
		//cout << rowFI1 << endl;
		float* pX = X.ptr<float>(0);//X序列的对应指针
		float* pY = Y.ptr<float>(0);//X序列的对应指针

									//***************************    1、行优先   ************************
									//1.1、先行后列，将输入彩色图像展开成序列，存在X中
		for (int i = 0; i < row; ++i)
		{
			float* data = input1.ptr<float>(i);
			for (int j = 0; j < col; ++j)
			{
				pX[i*col + j] = data[j];
			}
		}

		//获得处理后的Y序列
		getY(pX, pY, len, sigma, lamba);
		//将Y序列存储成先行后列的图像
		for (int i = 0; i < row; ++i)
		{
			float* data = rowFI1.ptr<float>(i);
			for (int j = 0; j < col; ++j)
			{
				data[j] = pY[i*col + j];
			}

		}

		//将先行后列的结果作为先列后行的输入，为了方便先列后行，先将先行后列的结果转置一下
		rowFI1_T = rowFI1.t();
		//1.2、先列后行，将中间结果展开成序列，存在X中
		for (int i = 0; i < col; ++i)
		{
			float* data = rowFI1_T.ptr<float>(i);
			for (int j = 0; j < row; ++j)
			{
				pX[i*row + j] = data[j];
			}
		}
		//获得处理后的Y序列
		getY(pX, pY, len, sigma, lamba);

		//将Y序列存储成先列后行的图像
		for (int i = 0; i < col; ++i)
		{
			float* data = rowFI1_T.ptr<float>(i);
			for (int j = 0; j < row; ++j)
			{
				data[j] = pY[i*row + j];
			}
		}
		//最后将矩阵转置回来
		rowFI1 = rowFI1_T.t();
		rowF_ch.at(c) = rowFI1;

		//********************************   2、列优先   **********************************
		//先对矩阵进行转置
		input1_T = input1.t();
		//1.2、先列后行，将输入彩色图像展开成序列，存在X中
		for (int i = 0; i < col; ++i)
		{
			float* data = input1_T.ptr<float>(i);
			for (int j = 0; j < row; ++j)
			{
				pX[i*row + j] = data[j];
			}
		}
		//获得处理后的Y序列
		getY(pX, pY, len, sigma, lamba);
		//将Y序列存储成先列后行的图像
		//转置
		colFI1_T = colFI1.t();
		for (int i = 0; i < col; ++i)
		{
			float* data = colFI1_T.ptr<float>(i);
			for (int j = 0; j < row; ++j)
			{
				data[j] = pY[i*row + j];
			}
		}
		//转置
		colFI1 = colFI1_T.t();
		//按先行后列将上一步的中间结果转成X序列
		for (int i = 0; i < row; ++i)
		{
			float* data = colFI1.ptr<float>(i);
			for (int j = 0; j < col; ++j)
			{
				pX[i*col + j] = data[j];
			}
		}
		// 获得处理后的Y序列
		getY(pX, pY, len, sigma, lamba);
		//将Y序列按先行后列存储为二维矩阵
		for (int i = 0; i < row; ++i)
		{
			float* data = colFI1.ptr<float>(i);
			for (int j = 0; j < col; ++j)
			{
				data[j] = pY[i*col + j];
			}
		}
		colF_ch.at(c) = colFI1;
		output_ch.at(c) = (rowF_ch.at(c) + colF_ch.at(c)) / 2;
	}

	merge(rowF_ch, rowFImg);
	imshow("rowF图", rowFImg);
	merge(colF_ch, colFImg);
	imshow("colF图", colFImg);
	merge(output_ch, outputImg);
	//outputImg.convertTo(input8, CV_8UC3, 255.0/1);//uchar convert to double
	imshow("美白图", outputImg);
	cvWaitKey(0);
	return 0;
}

//int main()
//{
//	//Mat inputImg = imread("F://picture/A.jpg");
//	//imshow("input", inputImg);
//	double z = 1.1;
//	cout << z << endl;
//	z = remap(0.89, 0.78,14);
//	cout << z << endl;
//	//Mat C = (Mat_<double>(3, 3) << 0, 1, 2, 3, 4, 5, 6, 7, 8);
//	//cout << "C=" << endl<< C << endl;
//	//cout << "C'=" << endl << C.t() << endl;
//	system("pause");
//	return 0;
//}
float remap(float x, float y, float sigma)
{
	float z;
	z = exp(-(x - y)*(x - y) / 2 * sigma*sigma);
	return z;
}

void getY(float *pX, float *pY, int len, float sigma, float lamba)
{
	Mat Xpre(1, len, CV_32FC1); //X前向序列
	Mat Xpost(1, len, CV_32FC1); //X后向序列
								 //Mat Y(1, len, CV_32FC1);
	float* pXpre = Xpre.ptr<float>(0);//X前向序列的对应指针
	float* pXpost = Xpost.ptr<float>(0);//X后向序列的对应指针
										//float* pY = Y.ptr<float>(0);//Y序列的对应指针
										// 获得处理后的Y序列
	pXpre[0] = pX[0];
	pXpost[len - 1] = pX[len - 1];
	for (int i = 1; i < len; ++i)//对应论文公式1
		pXpre[i] = (1 - remap(pX[i], pXpre[i - 1], sigma)*lamba)*pX[i] + remap(pX[i], pXpre[i - 1], sigma)*lamba*pXpre[i - 1];
	for (int i = len - 2; i >= 0; --i)//对应论文公式3
		pXpost[i] = (1 - remap(pX[i], pXpost[i + 1], sigma)*lamba)*pX[i] + remap(pX[i], pXpost[i + 1], sigma)*lamba*pXpost[i + 1];
	for (int i = 0; i < len; ++i)//对应论文公式5
		pY[i] = (pXpre[i] - (1 - lamba)*pX[i] + pXpost[i]) / (1 + lamba);
}