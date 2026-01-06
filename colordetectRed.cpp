#include<iostream>
#include<opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(){
VideoCapture cap("/home/firefighter-yy/桌面/detectRedArmor/video/装甲板识别视频.mp4");
if(!cap.isOpened()){
    cerr<<"请检测视频路径"<<endl;
        return -1;
}
    
namedWindow("原图",WINDOW_NORMAL);
namedWindow("灰度",WINDOW_NORMAL);
namedWindow("HSV",WINDOW_NORMAL);
namedWindow("二值化",WINDOW_NORMAL);
namedWindow("处理后图像",WINDOW_NORMAL);


int value_thres=145;

while(true) {
    Mat frame;
    cap >> frame;
            
    if(frame.empty()) {
        cout << "视频结束"<< endl;
            break;
    }

    Mat gray2;
    cvtColor(frame,gray2,COLOR_BGR2GRAY);

    threshold(gray2,gray2,value_thres*0.75,255,THRESH_BINARY);

    vector<Mat>splited_red;
    split(frame,splited_red);
    Mat color;
    subtract(splited_red[2],splited_red[0],color);

    threshold(color,color,105,255,THRESH_BINARY);

    Mat hsv;
    cvtColor(frame,hsv,COLOR_BGR2HSV);
    Scalar lower_red1(0,150,150);
    Scalar upper_red1(8,255,255);
    Scalar lower_red2(170,150,150);
    Scalar upper_red2(180,255,255);
    Mat mask1,mask2;
    inRange(hsv,lower_red1,upper_red1,mask1);
    inRange(hsv,lower_red2,upper_red2,mask2);

    Mat red_mask;
    red_mask=mask1|mask2;

    Mat temp3=color& gray2;
    Mat result=temp3&red_mask;

    Mat kernel = getStructuringElement(MORPH_RECT, Size(1, 4));
    dilate(result, result, kernel, Point(-1, -1), 4);

    Mat processed1;
    Mat kernel1 = getStructuringElement(MORPH_RECT, Size(6, 3));
    morphologyEx(result, processed1, MORPH_CLOSE, kernel1);
    Mat kernel2 = getStructuringElement(MORPH_RECT, Size(7, 2));
    dilate(processed1, processed1, kernel2);

    imshow("二值化",result);
    imshow("HSV",red_mask);
    imshow("灰度",gray2);
    imshow("处理后图像",processed1);
    imshow("原图",frame);

    int key=cv::waitKey(0);
    if(key == ' ') {
        break;
    }
        
    if(key == 'q' || key == 'Q') {
        cout << "用户退出" << endl;
        break;
    }

}
return 0;
}