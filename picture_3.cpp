#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

int main()
{
        Mat cap = imread("../images/picture_3.png");

        if (cap.empty())
    {
        cerr << "打开失败，请检查文件路径" << endl;
        return -1;
    }

    Mat hsv;

    int structSize = 3;
    GaussianBlur(cap, cap, Size(2 * structSize + 1, 2 * structSize + 1), 0, 0, BORDER_DEFAULT);     //高斯模糊,方便后续提取绿色小球

    cvtColor(cap, hsv, COLOR_BGR2HSV);

    Mat green_mask;
    inRange(hsv, Scalar(36, 50, 50), Scalar(77, 255, 255), green_mask);         //设定绿色颜色阀值

    int structElement = 5;
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(2 * structElement + 1, 2 * structElement+1));        //设定核

    //形态学操作
    Mat  temp_mask,final_mask;
    morphologyEx(green_mask,temp_mask,MORPH_OPEN,kernel);

    Mat kernel1=getStructuringElement(1,Size(11,11));
    erode(temp_mask,final_mask,kernel1,Point(-1,-1),2);

    vector<vector<Point>>contours;
    findContours(final_mask,contours,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);

    Mat result=cap.clone();
    int count=0;

    //遍历每一个轮廓，查找绿色小球
    for(size_t i=0;i<contours.size();i++){
        double area=contourArea(contours[i]);

        //计算圆形度,使更精确地找到绿色小球
        double perimeter=arcLength(contours[i],true);
        double circularity=(4*CV_PI*area)/(perimeter*perimeter);

        //使用最小外接园
        if(area>150&&area<3000&&circularity>0.5){
            Point2f center;
            float radius;
            minEnclosingCircle(contours[i],center,radius);

            circle(result,center,radius,Scalar(0,0,255),2);     //绘制圆形框

            count++;
        }
    }

    cout<<"总数为："<<count<<endl;
    
    namedWindow("结果图像",WINDOW_NORMAL);
    imshow("结果图像",result);

    waitKey(0);

    return 0;
}