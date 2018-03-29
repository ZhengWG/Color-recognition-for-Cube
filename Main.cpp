#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv2/core/core.hpp>  
#include <opencv2/opencv.hpp> 
#include <opencv.hpp>
#include <vector>
#include <iostream>
using namespace cv;
using namespace std;

Mat frame;
Mat binary;
Mat blured;
Mat aa;
Mat opened;
Mat AROI;
vector<Point> centers;
vector<vector<Point>> contours;
vector<vector<Point>> contours1;
vector<Point> cubecenters;
vector<Point> myCube;
int myCubex1, myCubex2, myCubey1, myCubey2;
float delta = 2;
double a;
int amin = 50; //原始值200 300
int amax = 200; //原始值500 1000

#define fileName "sample.jpg"

int blockSize = 57;
int constValue = 5;

string Colors[9] = { "zz" };

void getImage() {

	//VideoCapture cap(0);
	
	//cap >> frame;
	frame = imread(fileName);
	imshow("原图", frame);
}
void getbinary() {
	cvtColor(frame, binary, CV_RGB2GRAY);//转换到灰度空间
	//threshold(binary, binary, 70, 255, CV_THRESH_BINARY);//遍历灰度图中点，将图像信息二值化，处理过后的图片只有二种色值；初始阈值为70
	adaptiveThreshold(binary, binary, 255, ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, blockSize, constValue);
	imshow("灰度空间", binary);
	for (int i = 1; i < 6; i += 2)
		medianBlur(binary, blured, i);
	imshow("去噪", blured);
}
void getEroded() {
	Mat element(7, 7, CV_8U, Scalar(1));//5*5
	erode(blured, opened, element);
	imshow("腐蚀", opened);
}
void getcontours() {
	findContours(opened, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
	Mat temp(frame.size(), frame.type());
	drawContours(temp, contours, -1, Scalar(255, 0, 0));
	imshow("边缘检测", temp);
}
void getResult() {//除去太长或者太短的轮廓
	centers.clear(); //清空容器中的内容
	vector<vector<Point>>::const_iterator it = contours.begin();
	int xmin, xmax, ymin, ymax;//轮廓在x、y方向的最值
	while (it != contours.end()) {
		int size = it->size();
		if (size<amin || size>amax) {
			it = contours.erase(it);//删除元素it
		}
		else {
			int x = 0, y = 0, i = 0;
			xmin = 2000;//原始值2000
			xmax = 0;
			ymin = 2000;//原始值2000
			ymax = 0;
			vector<Point> con = *it;
			for (; i < size; i++) {
				xmin = (xmin < (con[i]).x) ? xmin : (con[i]).x;
				xmax = (xmax >(con[i]).x) ? xmax : (con[i]).x;
				ymin = (ymin < (con[i]).y) ? ymin : (con[i]).y;
				ymax = (ymax >(con[i]).y) ? ymax : (con[i]).y;
				x += (con[i]).x;
				y += (con[i]).y;
			}
			if ((float(xmax - xmin)) / (ymax - ymin) > delta || (float(ymax - ymin)) / (xmax - xmin) > delta) {
				it = contours.erase(it);
			}
			else {
				centers.push_back(Point(x / i, y / i));//末尾插入一个元素
				++it;
			}
		}
	}

	//面积筛选 select 9 contours with largest areas
	int Area[100] = { 0 };
	int centersSize = centers.size();
	for (int i = 0; i < centersSize; i++) {
		Area[i] = (int)-contourArea(contours[i], true);
		cout << Area[i] << endl;
	}


	for (int i = 0; i < 9; i++) {
		int maxArea = Area[i];
		int temp = i;
		for (int j = i; j < centersSize; j++) {
			if (Area[j] > maxArea) {
				maxArea = Area[j];
				temp = j;
			}
		}
		Area[temp] = Area[i];
		Area[i] = maxArea;

		Point tempPoint = centers[i];
		centers[i] = centers[temp];
		centers[temp] = tempPoint;

		cout << Area[i] << "  ";

	}
	cout << endl;



	//排序
	Point temp2;
	for (int i = 0; i < 9; i++) {
		for (int j = 8; j > i; j--) {
			if (centers[j].y < centers[j - 1].y) {
				temp2 = centers[j];
				centers[j] = centers[j - 1];
				centers[j - 1] = temp2;
			}
		}
	}

	for (int row = 0; row < 3; row++) {
		for (int i = 3 * row; i < 3 * row + 3; i++) {
			for (int j = 3 * row + 2; j > i; j--) {
				if (centers[j].x < centers[j - 1].x) {
					temp2 = centers[j];
					centers[j] = centers[j - 1];
					centers[j - 1] = temp2;
				}
			}
		}
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			cout << centers[3 * i + j].x << ", " << centers[3 * i + j].y << ", " << "      "; // Area[3 * i + j] << "         ";
		cout << endl;
	}



	Mat abc(frame.size(), frame.type());
	drawContours(abc, contours, -1, Scalar(255, 0, 0));
	for (int i = 0; i < centers.size(); i++) {
		//circle(abc, centers[i], 3, frame.at<Vec3b>(centers[i].y, centers[i].x));
		circle(abc, centers[i], 3, Scalar(255, 255, 255));
	}
	for (int i = 0; i < (int)contours.size(); i++)
	{

		a += arcLength(contours[i], true) / ((int)contours.size() * 4);
	}
	a /= 5;
	imshow("轮廓处理结果", abc);

}
int color(int iLowH, int iHighH, int iLowS, int iHighS, int iLowV, int iHighV, int cmin)
{
	Mat imgOriginal;
	imgOriginal = AROI;
	Mat imgHSV;
	vector<Mat> hsvSplit;
	cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

	//因为我们读取的是彩色图，直方图均衡化需要在HSV空间做
	split(imgHSV, hsvSplit);
	equalizeHist(hsvSplit[2], hsvSplit[2]);//H:色调；S：饱和度；V：明度
	merge(hsvSplit, imgHSV);
	Mat imgThresholded;

	inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

	//开操作 (去除一些噪点)
	Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
	morphologyEx(imgThresholded, imgThresholded, MORPH_OPEN, element);

	//闭操作 (连接一些连通域)
	morphologyEx(imgThresholded, imgThresholded, MORPH_CLOSE, element);

	//imshow("Thresholded Image", imgThresholded); //show the thresholded image
	//imshow("Original", imgOriginal); //show the original image

	Mat image;
	image = imgThresholded;

	//获取轮廓  
	std::vector<std::vector<Point>> contours;
	findContours(image, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	//打印轮廓信息  
	std::cout << "共有外围轮廓：" << contours.size() << "条" << std::endl;
	std::vector<std::vector<Point>>::const_iterator itContours = contours.begin();
	for (; itContours != contours.end(); ++itContours)
	{
		std::cout << "每个轮廓的长度: " << itContours->size() << std::endl;
	}


	//除去太长或者太短的轮廓  
	//int cmin = 100;  
	int cmax = 2000;
	std::vector<std::vector<Point>>::const_iterator itc = contours.begin();
	while (itc != contours.end())
	{
		if (itc->size() < cmin || itc->size() > cmax)  itc = contours.erase(itc);
		else  ++itc;
	}

	//把结果画在源图像上：
	Mat result(image.size(), CV_8U, Scalar(255));
	drawContours(imgOriginal, contours, -1, Scalar(255, 255, 255), 2);
	imshow("轮廓", imgOriginal);
	if (contours.size()>0)
		return 1;
	return 0;
}
int getCubeCenters() {

	Mat temp = Mat::zeros(frame.size(), frame.type());
	for (int i = 0; i<centers.size() && i<9; i++) {
		if (1) {
			Mat A = imread(fileName);
			Rect rect(centers[i].x - a, centers[i].y - a, 2 * a, 2 * a);
			AROI = A(rect);
			imshow("1", AROI);
			if (color(45, 77, 43, 255, 46, 255, 10)) {
				cout << i << ":绿色" << endl;
				Colors[i] = "green";
			}
			else if (color(0, 180, 0, 43, 46, 255, 10)) {
				cout << i << ":白色" << endl;
				Colors[i] = "white";
			}
			else if (color(26, 45, 43, 255, 46, 255, 10)) {
				cout << i << ":黄色" << endl;
				Colors[i] = "yellow";
			}
			else if (color(100, 124, 43, 255, 46, 255, 10)) {
				cout << i << ":蓝色" << endl;
				Colors[i] = "blue";
			}
			else if (color(2, 25, 0, 230, 46, 255, 10)) {
				cout << i << ":橙色" << endl;
				Colors[i] = "orange";
			}
			else if (color(0, 2, 230, 255, 46, 255, 10) || color(156, 180, 43, 255, 46, 255, 10)) {
				cout << i << ":红色" << endl;
				Colors[i] = "red";
			}

			//waitKey(0);
			rectangle(temp, Point(centers[i].x - a, centers[i].y - a), Point(centers[i].x + a, centers[i].y + a), Scalar(255, 255, 255), 2);
		}
	}
	cvtColor(temp, temp, CV_RGB2GRAY);
	imshow("rectangle", temp);
	Mat element2(2 * a, 2 * a, CV_8U, Scalar(1));//原始70，70
	dilate(temp, temp, element2);//膨胀、腐蚀将小方块拼合成矩形
	imshow("轮廓中心", temp);
	imwrite("finalContour.jpg", temp);
	findContours(temp, contours1, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
	Mat temp1(frame.size(), frame.type());
	drawContours(temp1, contours1, -1, Scalar(255, 0, 0));
	imshow("轮廓边缘最终", temp1);
	vector<vector<Point>> thisContours;
	findContours(temp, thisContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	vector<vector<Point>>::iterator it = thisContours.begin();
	while (it != thisContours.end()) {
		if (it->size() < 100 || it->size() > 3000)//修改阈值
			it = thisContours.erase(it);
		else
			it++;
	}
	if (thisContours.size() == 1) {
		Mat abcd(frame.size(), frame.type());
		drawContours(abcd, thisContours, -1, Scalar(255, 255, 255), 1);
		imshow("con", abcd);
		//确定轮廓的四个方向的极值
		myCubex1 = myCubey1 = 2000;
		myCubex2 = myCubey2 = 0;
		for (int i = 0; i < thisContours[0].size(); i++) {
			myCubex1 = (myCubex1<(thisContours[0][i]).x) ? myCubex1 : (thisContours[0][i]).x;
			myCubex2 = (myCubex2>(thisContours[0][i]).x) ? myCubex2 : (thisContours[0][i]).x;
			myCubey1 = (myCubey1<(thisContours[0][i]).y) ? myCubey1 : (thisContours[0][i]).y;
			myCubey2 = (myCubey2>(thisContours[0][i]).y) ? myCubey2 : (thisContours[0][i]).y;
		}
		myCubex1 += 70;
		myCubex2 -= 70;
		myCubey1 += 70;
		myCubey2 -= 70;
		if ((fabs((float)myCubex2 - myCubex1) > fabs((float)myCubey2 - myCubey1)*1.2) ||
			(fabs((float)myCubex2 - myCubex1)*1.2 < fabs((float)myCubey2 - myCubey1)))
			return 0;
		else
			return 1;
	}
	else {
		return 0;
	}
}

int getColor() {
	Mat imag1, imag2, result;
	imag1 = imread(fileName);
	imag2 = imread("finalContour.jpg");
	Mat hole(imag1.size(), CV_8U, Scalar(0)); //遮罩图层
	//imshow("hole1", hole);
	cv::drawContours(hole, contours1, -1, Scalar(255), CV_FILLED); //在遮罩图层上，用白色像素填充轮廓  
	//imshow("hole2", hole);
	Mat crop(imag1.rows, imag1.cols, CV_8UC3);
	imag1.copyTo(crop, hole);//将原图像拷贝进遮罩图层  
	imshow("My warpPerspective", crop);
	return 0;
}

void Visualize() {
	Mat cubeFacet = Mat::zeros(300, 300, CV_8UC3);
	int B = 0, G = 0, R = 0;
	for (int i = 0; i < 9; i++) {
		if (Colors[i] == "red") {
			B = 0;
			G = 0;
			R = 255;
		}
		else if (Colors[i] == "yellow") {
			B = 0;
			G = 255;
			R = 255;
		}
		else if (Colors[i] == "orange") {
			B = 10;
			G = 97;
			R = 230;
		}
		else if (Colors[i] == "blue") {
			B = 255;
			G = 0;
			R = 0;
		}
		else if (Colors[i] == "green") {
			B = 0;
			G = 255;
			R = 0;
		}
		else if (Colors[i] == "white") {
			B = 255;
			G = 255;
			R = 255;
		}
		rectangle(cubeFacet, Point(i % 3 * 100, i / 3 * 100),
			Point((i % 3 + 1) * 100, (i / 3 + 1) * 100),
			Scalar(B, G, R), -1, 8);
	}
	line(cubeFacet, Point(0, 100), Point(300, 100), Scalar(0, 0, 0), 3, 8, 0);
	line(cubeFacet, Point(0, 200), Point(300, 200), Scalar(0, 0, 0), 3, 8, 0);
	line(cubeFacet, Point(100, 0), Point(100, 300), Scalar(0, 0, 0), 3, 8, 0);
	line(cubeFacet, Point(200, 0), Point(200, 300), Scalar(0, 0, 0), 3, 8, 0);
	imshow("Visualise", cubeFacet);
}




int main() {
	getImage(); waitKey(0);
	getbinary(); waitKey(0);
	getEroded(); waitKey(0);
	getcontours(); waitKey(0);
	getResult(); waitKey(0);
	getCubeCenters(); waitKey(0);
	getColor(); waitKey(0);
	Visualize();
	waitKey(0);
	system("pause");
	return 0;
}
