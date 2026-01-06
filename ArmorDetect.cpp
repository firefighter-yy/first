#include<iostream>
#include<opencv2/opencv.hpp>
#include<vector>

using namespace std;
using namespace cv;

Mat detectRedArea(Mat& frame, int value_thres) {
    //灰度图像
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

    Mat gray2;
    cvtColor(frame,gray2,COLOR_BGR2GRAY);

    threshold(gray2,gray2,value_thres*0.75,255,THRESH_BINARY);

    vector<Mat>splited_red;
    split(frame,splited_red);
    subtract(splited_red[2],splited_red[0],color);

    threshold(color,color,105,255,THRESH_BINARY);

    Mat processed;
    Mat kernel1=getStructuringElement(MORPH_RECT,Size(2,2));
    morphologyEx(result,processed,MORPH_OPEN,kernel1);
    Mat kernel2 = getStructuringElement(MORPH_RECT, Size(1, 3));
    dilate(processed,processed, kernel2, Point(-1, -1), 6);

    Mat hsv1;
    cvtColor(frame,hsv1,COLOR_BGR2HSV);
    Scalar lower_red1(0,150,150);
    Scalar upper_red1(8,255,255);
    Scalar lower_red2(170,150,150);
    Scalar upper_red2(180,255,255);
    Mat mask1,mask2;
    inRange(hsv1,lower_red1,upper_red1,mask1);
    inRange(hsv1,lower_red2,upper_red2,mask2);

    Mat red_mask;
    red_mask=mask1|mask2;

    Mat temp3=color& gray2;
    Mat result1=temp3&red_mask;

    Mat kernel = getStructuringElement(MORPH_RECT, Size(1, 4));
    dilate(result1, result1, kernel, Point(-1, -1), 4);

    Mat processed1;
    Mat kernel3 = getStructuringElement(MORPH_RECT, Size(6, 3));
    morphologyEx(result1, processed1, MORPH_CLOSE, kernel3);
    Mat kernel4 = getStructuringElement(MORPH_RECT, Size(7, 2));
    dilate(processed1, processed1, kernel4);

    processed=processed1|processed;
    
    return processed;
}

float getGrayBetweenBars(const Mat& frame, const RotatedRect& left_bar, const RotatedRect& right_bar) {
    int center_x = (left_bar.center.x + right_bar.center.x) / 2;
    int center_y = (left_bar.center.y + right_bar.center.y) / 2;
    
    int width = (right_bar.center.x - left_bar.center.x) * 0.27;
    int height = (left_bar.size.height + right_bar.size.height) * 0.27;
    
    //防御措施（避免图像出框）
    int x = max(0, center_x - width/2);
    int y = max(0, center_y - height/2);
    width = min(width, frame.cols - x);
    height = min(height, frame.rows - y);
    
    //排除小噪音
    if (width <= 7 || height <= 7) return 0;
    
    //得roi区域
    Mat roi = frame(Rect(x, y, width, height));
    Mat gray;
    cvtColor(roi, gray, COLOR_BGR2GRAY);
    
    //得灰度平均值
    return (float)mean(gray)[0];
}

vector<Rect> detectRedArmor(Mat& frame, Mat& binary, int value_thres) {
    vector<Rect> armors;
    Mat mask = detectRedArea(frame, value_thres);
    binary = mask.clone();

    //图像处理
    Mat kernel1 = getStructuringElement(MORPH_RECT, Size(6, 3));
    morphologyEx(binary, binary, MORPH_CLOSE, kernel1);
    Mat kernel2 = getStructuringElement(MORPH_RECT, Size(3, 1));
    dilate(binary, binary, kernel2, Point(-1, -1), 3);

    vector<vector<Point>> contours;
    findContours(binary, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    
    vector<RotatedRect> light_bars;
    
    for(size_t i = 0; i < contours.size(); i++) {
        double area = contourArea(contours[i]);
        //排除小的和大的噪音
        if(area < 10 || area > 3000) continue;

    RotatedRect min_rect = minAreaRect(contours[i]);              
    float width = min(min_rect.size.width, min_rect.size.height);
    float height = max(min_rect.size.width, min_rect.size.height);
    float ratio = height / width;
    double fill_rate = area / (width * height);

    if((ratio > 1.1 && ratio < 4.8 && fill_rate > 0.1)) {
        if(min_rect.size.width > min_rect.size.height) {                       
            swap(min_rect.size.width, min_rect.size.height);
            min_rect.angle += 90.0f;
        }

        // 确保角度在[-90, 0)范围内
        if(min_rect.angle>0.0f){
            if(min_rect.angle>90.0f){
                min_rect.angle-=180.0f;
            }
        }
        if(min_rect.angle<-90.0f){
            min_rect.angle+=90.0f;
        }
        light_bars.push_back(min_rect);
    }
}

    if(light_bars.size() >= 2) {
        //以x坐标排序
        sort(light_bars.begin(), light_bars.end(),
            [](const RotatedRect& a, const RotatedRect& b) {
                return a.center.x < b.center.x;
            });

        for(size_t i = 0; i < light_bars.size(); i++) {
            for(size_t j = i + 1; j < light_bars.size(); j++) {
                RotatedRect& r1 = light_bars[i];
                RotatedRect& r2 = light_bars[j];

                //判断角度相似
                float raw_angle_diff = abs(r1.angle - r2.angle);
                float angle_diff = raw_angle_diff;
                if(raw_angle_diff > 90.0f) {
                    angle_diff = 180.0f - raw_angle_diff;
                }
                bool angle_match = angle_diff < 28.0f || angle_diff > 150.0f;

                //判断y坐标相似
                bool same_level = abs(r1.center.y - r2.center.y) < 28;
                //判断宽度合理
                float dist = abs(r1.center.x - r2.center.x);
                float avg_height = (r1.size.height + r2.size.height) / 2.0f;
                bool suit_distance = (dist > avg_height * 1.4) && (dist < avg_height * 2.45);
                //判断高度相似
                bool same_height = abs(r1.size.height - r2.size.height) < 30;
                //判断灯条宽度相似
                bool same_width = abs(r1.size.width - r2.size.width) < 15;
                
                float ratio1 = r1.size.height / r1.size.width;
                float ratio2 = r2.size.height / r2.size.width;
                //对不同灯条分类讨论
                bool is_slender = (ratio1 > 3.5f) || (ratio2 > 3.5f);
                float max_ratio_diff = is_slender ? 5.0f : 3.5f;
                float min_ratio_diff = is_slender ? 0.25f : 0.35f;
                bool same_ratio = (abs(ratio1 / ratio2) < max_ratio_diff) && 
                                  (abs(ratio1 / ratio2) > min_ratio_diff);
                bool correct_order = r1.center.x < r2.center.x;     //判断顺序正确

                if(angle_match && same_level && suit_distance && 
                   same_height && same_width && same_ratio && correct_order) {
                    
                    float gray_value = getGrayBetweenBars(frame, r1, r2);
                    
                    float x = min(r1.center.x - r1.size.width/2, r2.center.x - r2.size.width/2);
                    float y = min(r1.center.y - r1.size.height/2, r2.center.y - r2.size.height/2);
                    float right = max(r1.center.x + r1.size.width/2, r2.center.x + r2.size.width/2);
                    float bottom = max(r1.center.y + r1.size.height/2, r2.center.y + r2.size.height/2);

                    float armor_width = right - x;
                    float armor_height = bottom - y;

                    if (gray_value < 71) continue;
                        if(armor_height > 10 && armor_width > 10) {
                            int margin = 5;
                            x = max(0.0f, x - margin);
                            y = max(0.0f, y - margin);
                            armor_width = min(armor_width + 2 * margin, frame.cols - x);
                            armor_height = min(armor_height + 2 * margin, frame.rows - y);
                        
                            armors.push_back(Rect(x, y, armor_width, armor_height));
                        }
                    }
                }
            }
        }
    return armors;
}

int main() {
    string video_path = "/home/firefighter-yy/桌面/detectRedArmor/video/装甲板识别视频.mp4";
    VideoCapture cap(video_path);
    
    if(!cap.isOpened()) {
        cerr << "请检测视频路径: " << video_path << endl;
        return -1;
    }
    
    namedWindow("原图", WINDOW_NORMAL);
    namedWindow("结果", WINDOW_NORMAL);
    namedWindow("处理后图像", WINDOW_NORMAL);
    
    int value_thres = 145;
    
    while(true) {
        Mat frame;
        cap >> frame;
        
        if(frame.empty()) {
            cout << "视频结束" << endl;
            break;
        }

        Mat binary;
        vector<Rect> armors = detectRedArmor(frame, binary, value_thres);
        
        Mat result = frame.clone();
        
        for(size_t i = 0; i < armors.size(); i++) {
            rectangle(result, armors[i], Scalar(0, 255, 0), 2);
        }
        
        putText(result, format("armor: %d", (int)armors.size()),
                Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 2);
        
        imshow("原图", frame);
        imshow("结果", result);
        imshow("处理后图像", binary);

        int key = waitKey(30);
        if(key == 'q' || key == 'Q') {
            cout << "退出" << endl;
            break;
        }
        else if(key == ' ') {
            waitKey(0);
        }
    }

    cap.release();
    destroyAllWindows();
    
    return 0;
}