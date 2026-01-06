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
namedWindow("白色",WINDOW_NORMAL);
namedWindow("处理后图像",WINDOW_NORMAL);
namedWindow(".",WINDOW_NORMAL);

int value_thres=145;

while(true) {
    Mat frame;
    cap >> frame;
            
    if(frame.empty()) {
        cout << "视频结束"<< endl;
            break;
    }
    Mat gray;
    cvtColor(frame,gray,COLOR_BGR2GRAY);

    threshold(gray, gray, 200, 255, THRESH_BINARY); 

    vector<Mat>splited;
    split(frame,splited);
    Mat color;
    Mat blue_wight=splited[0]*1.8;
    Mat red_wight=splited[2]*0.55;
    subtract(blue_wight,red_wight,color);
    threshold(color,color,105,255,THRESH_BINARY);

    Mat hsv;
    cvtColor(frame,hsv,COLOR_BGR2HSV);

    Scalar lower_blue(90, 50, 150); 
    Scalar upper_blue(140, 255, 255);
    Mat blue_mask;
    inRange(hsv, lower_blue, upper_blue, blue_mask);

    Scalar lower_white(0,0,220);
    Scalar upper_white(180,20,255);
    Mat white_mask;
    inRange(hsv,lower_white,upper_white,white_mask);

    bitwise_not(blue_mask,blue_mask);

    Mat temp1 = color & gray;
    Mat temp2= white_mask& blue_mask;
    Mat result = temp1 & temp2;

    /*vector<vector<Point>> contours;
    findContours(result, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    Mat filled = Mat::zeros(result.size(), CV_8UC1);
    for(size_t i = 0; i < contours.size(); i++) {
        if(contourArea(contours[i]) > 20) {
            drawContours(filled, contours, i, Scalar(255), FILLED);
        }
    }
    Mat kernel1 = getStructuringElement(MORPH_RECT, Size(3,3));
    morphologyEx(filled, filled, MORPH_CLOSE, kernel1); // 闭运算连接

    Mat kernel2 = getStructuringElement(MORPH_RECT, Size(2,6));
    dilate(filled, filled, kernel2, Point(-1,-1), 1);

    Mat processed = filled;*/

    Mat processed;
    Mat kernel1 = getStructuringElement(MORPH_RECT, Size(2, 2));
    morphologyEx(result, processed, MORPH_OPEN, kernel1);
    Mat kernel2 = getStructuringElement(MORPH_RECT, Size(1, 3));
    dilate(processed, processed, kernel2, Point(-1, -1), 6);

    imshow("二值化",result);
    imshow("HSV",blue_mask);
    imshow("灰度",gray);
    imshow("处理后图像",processed);
    imshow("白色",white_mask);
    imshow("原图",frame);
    imshow(".",color);

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