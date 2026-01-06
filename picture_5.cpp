#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

int main()
{  
    Mat cap = imread("../images/picture_5.png");

    if (cap.empty())
    {
        cerr << "打开失败，请检查文件路径" << endl;
        return -1;
    }

    Mat hsv;
    GaussianBlur(cap, cap, Size(5, 5), 0);
    cvtColor(cap, hsv, COLOR_BGR2HSV);

    Mat green_mask;
    inRange(hsv, Scalar(35, 50, 50), Scalar(85, 255, 255), green_mask);
    
    //排除白色干扰
    Mat white_mask;
    inRange(hsv, Scalar(0, 0, 180), Scalar(180, 60, 255), white_mask);
    green_mask = green_mask - white_mask;

    //形态学处理
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(11, 11));
    morphologyEx(green_mask, green_mask, MORPH_CLOSE, kernel);
    morphologyEx(green_mask, green_mask, MORPH_OPEN, kernel);
    
    Mat kernel_small = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
    erode(green_mask, green_mask, kernel_small, Point(-1,-1), 2);

    //分水岭算法
    Mat result = cap.clone();
    int count = 0;
    
    Mat dist;
    distanceTransform(green_mask, dist, DIST_L2, 5);
    
    Mat sure_fg;
    double max_val;
    minMaxLoc(dist, 0, &max_val);
    
    double fg_threshold = 0.25 * max_val;
    threshold(dist, sure_fg, fg_threshold, 255, THRESH_BINARY);
    sure_fg.convertTo(sure_fg, CV_8U);
    
    Mat markers;
    connectedComponents(sure_fg, markers);
    markers = markers + 1;
    
    Mat sure_bg;
    dilate(green_mask, sure_bg, Mat(), Point(-1,-1), 3);
    Mat unknown;
    subtract(sure_bg, sure_fg, unknown);
    markers.setTo(0, unknown);
    
    watershed(cap, markers);
    
    //统计结果
    for (int label = 2; label <= 100; label++) {
        Mat region = (markers == label);
        if (countNonZero(region) > 50) {
            vector<vector<Point>> contours;
            findContours(region, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
            
            if (!contours.empty()) {
                double area = contourArea(contours[0]);
                double perimeter = arcLength(contours[0], true);
                double circularity = (4 * CV_PI * area) / (perimeter * perimeter);
                
                if (area > 100 && area < 5000 && circularity > 0.4) {
                    Point2f center;
                    float radius;
                    minEnclosingCircle(contours[0], center, radius);
                    circle(result, center, radius, Scalar(0, 0, 255), 2);
                    count++;
                }
            }
        }
    }

    cout << "检测到 " << count << " 个绿色小球" << endl;
    
    //保存成图片
    string output_path = "result_picture_5.jpg";
    bool saved = imwrite(output_path, result);
    
    if (saved) {
        cout << "结果已保存到: " << output_path << endl;
        cout << "请打开文件查看结果" << endl;
    } else {
        cerr << "保存失败！" << endl;
    }
    
    imwrite("green_mask.jpg", green_mask);
    imwrite("distance_transform.jpg", dist);
    
    cout << "处理完成！" << endl;

    return 0;
} 