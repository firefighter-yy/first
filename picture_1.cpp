#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int main()
{
    cv::Mat cap = imread("../images/picture_1.png");;

        if (cap.empty())
    {
        cerr << "打开失败，请检查文件路径" << endl;
        return -1;
    }

    Mat hsv;

    int structSize = 3;
    GaussianBlur(cap, cap, Size(2 * structSize + 1, 2 * structSize + 1), 0, 0, BORDER_DEFAULT);     //高斯模糊去噪声，方便后续提取绿色小球轮廓

    cvtColor(cap, hsv, COLOR_BGR2HSV);

    Mat green_mask;
    inRange(hsv, Scalar(35, 50, 50), Scalar(85, 255, 255), green_mask);         //绿色阀值分割

    int structElement = 3;
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(2 * structElement + 1, 2 * structElement+1));        //设定椭圆形核，接近小球形状

    //形态学操作：先开运算去噪声，后闭运算填充小球内部小空洞
    Mat temp_mask, final_mask;
    morphologyEx(green_mask, temp_mask, MORPH_OPEN, kernel);
    morphologyEx(temp_mask, final_mask, MORPH_CLOSE, kernel);

    vector<vector<Point>>contours;
    findContours(final_mask,contours,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);//因为每个小球独立，只用检测最外层轮廓就能找到小球

    Mat result=cap.clone();
    int count=0;

    //遍历每一个轮廓，查找绿色小球
    for(size_t i=0;i<contours.size();i++){
        double area=contourArea(contours[i]);

        //因为只有绿色小球，不用设定上限
        if(area>50){
            Rect rect=boundingRect(contours[i]);
            rectangle(result,rect,Scalar(0,0,255),2);
            count++;
        }
    }

    cout<<"总数为："<<count<<endl;
    
    namedWindow("结果图像");
    imshow("结果图像",result);

    waitKey(0);

    return 0;
}