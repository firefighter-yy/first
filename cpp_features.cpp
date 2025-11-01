#include<opencv2/opencv.hpp>
#include<iostream>
#include<vector>
#include<map>
#include<string>

using namespace cv;
using namespace std;

//使用颜色结构体，管理不同颜色
struct ColorInfo
{
    string name;
    vector<Scalar>lower_ranges;
    vector<Scalar>upper_ranges;
    Scalar display_color;
    int total_count;
    //初始化
    ColorInfo(string n,vector<Scalar>lowers,vector<Scalar>uppers,Scalar color)
        :name(n),lower_ranges(lowers),upper_ranges(uppers),display_color(color),total_count(0){}
};

int main(){
    VideoCapture cap("/home/firefighter-yy/桌面/first video/魔方识别视频.mp4");
    
    if(!cap.isOpened()){
        cerr<<"请检查视频路径"<<endl;
        return -1;
    }

    namedWindow("原始视频",WINDOW_NORMAL);
    namedWindow("最终视频",WINDOW_NORMAL);

    //初始化颜色
    vector<ColorInfo>colors={
        ColorInfo(
            "红色",
            {Scalar(0,150,150),Scalar(160,150,150)},
            {Scalar(10,255,255),Scalar(100,255,255)},
            Scalar(0,0,255)
        ),
        ColorInfo(
            "绿色",
            {Scalar(35,100,100)},
            {Scalar(85,255,255)},
            Scalar(0,255,0)
        ),
        ColorInfo(
            "蓝色",
            {Scalar(100,100,100)},
            {Scalar(140,255,255)},
            Scalar(255,0,0)
        )
    };

    Mat kernel=getStructuringElement(MORPH_RECT,Size(3,3));         //建立核

    Mat frame,hsv;                                                  //声明原始图像和hsv图像
    int frame_number=0;                                             //初始化帧数
    map<string,int>color_count;                                     //声明不同颜色的实时色块

    while(1){
        cap>>frame;
        if(frame.empty()){
            cout<<"完成"<<endl;
            break;
        }
        frame_number++;

        cvtColor(frame,hsv,COLOR_BGR2HSV);                          //转化为hsv空间
        color_count.clear();                                        //清除记录的色块

        Mat result=frame.clone();                                   //建立副本

        //对各颜色进行阈值分割
        for(auto & color:colors){
        Mat color_mask;
        for(size_t i=0;i<color.lower_ranges.size();i++){
            Mat temp_mask;
            
        inRange(hsv,color.lower_ranges[i],color.upper_ranges[i],temp_mask);

        if(i==0){
            color_mask=temp_mask;
            }
        else{
            color_mask=color_mask|temp_mask;
            }
        }
    
        //进行开，闭运算
        Mat cleaned_mask;
        morphologyEx(color_mask,cleaned_mask,MORPH_OPEN,kernel);
        morphologyEx(cleaned_mask,cleaned_mask,MORPH_CLOSE,kernel);
    
        vector<vector<Point>>contours;                                //存储检测的轮廓                          
        findContours(cleaned_mask,contours,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
        
        int block_count=0;

        //计算面积
        for(const auto & contour:contours){
            double area=contourArea(contour);
        
            //由于魔方块为正方体，通过边长相近的特点使检测结果更准确
            if(area>80&&area<2000){
                Rect boundRect=boundingRect(contour);
                double aspect_ratio=(double)boundRect.width/boundRect.height;

            //在result上画出轮廓
            if(aspect_ratio>0.6&&aspect_ratio<1.4){
                drawContours(result,vector<vector<Point>>{contour},-1,color.display_color,2);
                block_count++;
            }
        }
    }

        //更新统计数据
        color_count[color.name]=block_count;
        color.total_count+=block_count;
    }

    string block_text="当前块:";
    for(auto & pair:color_count){
        block_text+=pair.first +":"+to_string(pair.second)+" ";
    }
    
    //在图像上添加实时色块的数量，同时添加帧数
    putText(result,block_text,
        Point(10,25),FONT_HERSHEY_SIMPLEX,0.5,Scalar(255,255,255),1);
    putText(result,"帧"+to_string(frame_number),
        Point(10,frame.rows - 10),FONT_HERSHEY_SIMPLEX,0.5,Scalar(255,255,255),1);
    
    //输出图像结果
    imshow("原始视频",frame);
    imshow("最终视频",result);

    int key=waitKey(30);
    if(key==27){
        cout<<"输入ESC退出"<<endl;
        break;
        }
    }

    //输出统计结果
    for(auto &color:colors){
        cout<<color.name<<"色块总数"<<color.total_count<<endl;
    }
    
    //释放内存
    cap.release();
    destroyAllWindows();

    return 0;
}