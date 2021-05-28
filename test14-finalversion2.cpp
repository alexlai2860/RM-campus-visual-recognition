
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int nThreshold1 = 0;
int nThreshold2 = 240;
int tmin = 190, tmax = 240;
int amax = 520;
Mat src;
Mat src_gray, binary1, binary2, edge, edgepro;
void on_trackbar(int, void*)
{

}
// 生成轨迹条
void creat_trackbar()
{
	
	namedWindow("阈值",1);
	resizeWindow("阈值", 640, 320);
	createTrackbar("二值图 1", "阈值", &nThreshold1, 254, on_trackbar);
	createTrackbar("二值图 2", "阈值", &nThreshold2, 254, on_trackbar);
	createTrackbar("canny阈值1", "阈值", &tmin, 254, on_trackbar);
	createTrackbar("canny阈值2", "阈值", &tmax, 254, on_trackbar);
	createTrackbar("最小面积", "阈值", &amax, 2000, on_trackbar);
}

int main()
{
	//默认为外接摄像头
	VideoCapture capture(1);
	creat_trackbar();
	on_trackbar(0,0);
	while (1)
    {
		Mat frame;
		capture >> frame;
		src = frame;
		Mat result = src.clone();

		//灰度化
		cvtColor(src, src_gray, COLOR_BGR2GRAY);
		//首次二值化，滤除低频无关信号；默认阈值为0
		threshold(src_gray, binary1, nThreshold1, 255, THRESH_TOZERO);
		//再次二值化，压制高光，只保留中间调；默认阈值为240
		threshold(binary1, binary2, nThreshold2, 255, THRESH_TRUNC);
		imshow("binary", binary2);
		//中值滤波降噪
		medianBlur(binary2, edge, 3);
		imshow("edge1", edge);
		//高斯模糊处理
		GaussianBlur(edge, edge, cv::Size(3, 3), 0);
		imshow("edge2", edge);
		//运行canny算子，提取边缘；默认高低阈值分别为240和190
		Canny(edge, edge, tmin, tmax, 3);

		//进行简单的边缘膨胀，防止重复识别同一边缘
		dilate(edge, edgepro, cv::Mat());
		imshow("edgepro", edgepro);

		//发现轮廓
		vector<vector<Point>> contours;
		vector<Point> point;
		vector<Vec4i> hireachy;
		findContours(edgepro, contours, hireachy, RETR_TREE, CHAIN_APPROX_NONE, Point());
		int num = 0;
		int a = 0;
		int b = 0;
		int c = 0;
		int d = 0;
		int e = 0;
		//绘制出所有轮廓
		for (int t = 1; t < contours.size(); t++)
		{
			int x = arcLength(contours[t], true);
			int area = contourArea(contours[t], true);
			int epsilon = 0.01 * arcLength(contours[t], true);
			approxPolyDP(contours[t], point, epsilon, true);  
			// 对周长和面积进行限制，降低误判
			if (x > 80)
			{
				//面积可用trackbar进行更改，默认最小面积为520
				if (area > amax)
				{
					drawContours(result, contours, t, Scalar(128, 0, 128), 3, 8, Mat(), 0, Point());
					num++;
					//用边数判断对应图形
					if (point.size() == 3)
					{
						a++;
					}
					else if (point.size() == 4)
					{
						b++;
					}
					else if (point.size() == 5)
					{
						c++;
					}
					else if (point.size() == 10)
					{
						d++;
					}
					else if (point.size() >= 15)
					{
						e++;
					}
				}
			}
		}
		//找出最大的元素，若最大值不为0，则进一步识别对应的图形
		int n[5] = { a,b,c,d,e };
		int x = *max_element(n, n + 5);
		string A;
		stringstream S1;
		S1 << num;
		S1 >> A;
		if (x == 0)
		{
			string t0 = "none!";
			putText(result, t0, Point(50, 60), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 0, 255), 4, 8);
		}
		//通过各边长累加数量是否为0来判断形状
		while (x != 0)
		{
			if (a != 0)
			{
				string t1 = "triangle:" + A;
				putText(result, t1, Point(50, 60), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 0, 255), 4, 8);
				break;
			}
			else if (b != 0)
			{
				string t2 = "rectangle:" + A;
				putText(result, t2, Point(50, 60), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 0, 255), 4, 8);
				break;
			}
			else if (c != 0)
			{
				string t3 = "pentagon:" + A;
				putText(result, t3, Point(50, 60), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 0, 255), 4, 8);
				break;
			}
			else if (d != 0)
			{
				string t4 = "star:" + A;
				putText(result, t4, Point(50, 60), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 0, 255), 4, 8);
				break;
			}
			else if (e != 0)
			{
				string t5 = "circle:" + A;
				putText(result, t5, Point(50, 60), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 0, 255), 4, 8);
				break;
			}
		}
		imshow("result", result);
		waitKey(50);
	}

	return 0;
}