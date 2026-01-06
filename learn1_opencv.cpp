#include <iostream>
#include <cstring>
#include <opencv2/opencv.hpp>
#include "GxIAPI.h"

using namespace std;
using namespace cv;

int main() {
    GX_STATUS status = GX_STATUS_SUCCESS;
    GX_DEV_HANDLE device = nullptr;
    
    // 1. 初始化SDK
    status = GXInitLib();
    if(status != GX_STATUS_SUCCESS){
        cout << "SDK初始化失败 (错误码:" << status << ")" << endl;
        return 1;
    }
    cout << "SDK初始化成功" << endl;

    // 2. 枚举设备
    uint32_t number = 0;
    status = GXUpdateDeviceList(&number, 10000);
    if(status != GX_STATUS_SUCCESS){
        cout << "枚举设备失败" << endl;
        GXCloseLib();
        return 1;
    }
    cout << "发现相机数量: " << number << endl;
    
    if(number == 0){
        cout << "未检测到相机" << endl;
        GXCloseLib();
        return 1;        
    }
    cout << "检测到相机" << endl;

    // 3. 连接相机
    status = GXOpenDeviceByIndex(1, &device);
    if(status != GX_STATUS_SUCCESS){
        cout << "打开相机失败 (错误码:" << status << ")" << endl;
        GXCloseLib();
        return 1;
    }
    cout << "相机连接成功" << endl;

    // 4. 获取像素格式
    int64_t pixel_format = 0;
    status = GXGetEnum(device, GX_ENUM_PIXEL_FORMAT, &pixel_format);
    cout << "像素格式: 0x" << hex << pixel_format << dec << endl;
    
    // 5. 设置采集模式
    GXSetEnum(device, GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS);

    // 6. 开始采集
    status = GXStreamOn(device);
    if(status != GX_STATUS_SUCCESS){
        cout << "开始采集失败" << endl;
        GXCloseDevice(device);
        GXCloseLib();
        return 1;
    }
    cout << "开始采集..." << endl;

    // 7. 获取图像大小
    int64_t payload_size = 0;
    status = GXGetInt(device, GX_INT_PAYLOAD_SIZE, &payload_size);
    if(status != GX_STATUS_SUCCESS || payload_size <= 0){
        cout << "获取图像大小失败" << endl;
        GXStreamOff(device);
        GXCloseDevice(device);
        GXCloseLib();
        return 1;
    }

    // 8. 预分配图像缓冲区
    unsigned char* image_buffer = new unsigned char[(size_t)payload_size];
    
    // 9. 初始化帧数据结构
    GX_FRAME_DATA frame_data{};
    frame_data.pImgBuf = image_buffer;
    frame_data.nImgSize = (size_t)payload_size;
    
    // 10. 创建OpenCV窗口
    namedWindow("Daheng Camera - Live", WINDOW_AUTOSIZE);
    namedWindow("Daheng Camera - Info", WINDOW_AUTOSIZE);
    
    cout << "\n按 ESC 键退出程序" << endl;
    cout << "按 's' 键保存当前帧" << endl;

    int frame_count = 0;
    int save_count = 0;
    
    while(true) {
        // 采集一帧图像
        status = GXGetImage(device, &frame_data, 100);
        
        if(status == GX_STATUS_SUCCESS) {
            frame_count++;
            
            // 创建OpenCV图像
            Mat image;
            
            // 根据像素格式创建图像
            // 常见格式处理：
            switch(pixel_format) {
                case 0x01080001: // GX_PIXEL_FORMAT_MONO8
                    image = Mat(frame_data.nHeight, frame_data.nWidth, 
                               CV_8UC1, frame_data.pImgBuf);
                    cvtColor(image, image, COLOR_GRAY2BGR);
                    break;
                    
                case 0x0108000A: // GX_PIXEL_FORMAT_BAYER_GB8
                case 0x0108000B: // GX_PIXEL_FORMAT_BAYER_RG8
                case 0x0108000C: // GX_PIXEL_FORMAT_BAYER_GR8
                case 0x0108000D: // GX_PIXEL_FORMAT_BAYER_BG8
                    image = Mat(frame_data.nHeight, frame_data.nWidth, 
                               CV_8UC1, frame_data.pImgBuf);
                    cvtColor(image, image, COLOR_BayerBG2BGR); // 可能需要调整
                    break;
                    
                default:
                    // 默认按8位灰度处理
                    image = Mat(frame_data.nHeight, frame_data.nWidth, 
                               CV_8UC1, frame_data.pImgBuf);
                    cvtColor(image, image, COLOR_GRAY2BGR);
                    cout << "警告: 使用默认灰度处理，像素格式: 0x" 
                         << hex << pixel_format << dec << endl;
            }
            
            // 显示图像
            imshow("Daheng Camera - Live", image);
            
            // 创建信息图像
            Mat info_image = Mat::zeros(200, 400, CV_8UC3);
            putText(info_image, format("Frame: %d", frame_count), 
                    Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.8, 
                    Scalar(0, 255, 0), 2);
            putText(info_image, format("Size: %dx%d", 
                    frame_data.nWidth, frame_data.nHeight), 
                    Point(10, 70), FONT_HERSHEY_SIMPLEX, 0.8, 
                    Scalar(0, 255, 0), 2);
            putText(info_image, format("Frame ID: %d", 
                    frame_data.nFrameID), 
                    Point(10, 110), FONT_HERSHEY_SIMPLEX, 0.8, 
                    Scalar(0, 255, 0), 2);
            putText(info_image, "Press ESC to exit", 
                    Point(10, 150), FONT_HERSHEY_SIMPLEX, 0.7, 
                    Scalar(0, 165, 255), 2);
            putText(info_image, "Press 's' to save frame", 
                    Point(10, 180), FONT_HERSHEY_SIMPLEX, 0.7, 
                    Scalar(0, 165, 255), 2);
            imshow("Daheng Camera - Info", info_image);
            
            // 键盘输入处理
            int key = waitKey(1);
            if (key == 27) { // ESC键
                cout << "用户中断采集" << endl;
                break;
            } else if (key == 's' || key == 'S') {
                // 保存当前帧
                string filename = format("frame_%04d.png", ++save_count);
                imwrite(filename, image);
                cout << "已保存: " << filename << endl;
                
                // 显示保存提示
                Mat saved_msg = info_image.clone();
                putText(saved_msg, format("Saved: %s", filename.c_str()), 
                        Point(10, 150), FONT_HERSHEY_SIMPLEX, 0.7, 
                        Scalar(0, 255, 255), 2);
                imshow("Daheng Camera - Info", saved_msg);
                waitKey(500); // 显示500ms
            }
        } else if (status == GX_STATUS_TIMEOUT) {
            // 超时，继续尝试
            continue;
        } else {
            cout << "采集失败 (错误码:" << status << ")" << endl;
            break;
        }
    }
    
    // 11. 清理资源
    destroyAllWindows();
    delete[] image_buffer;
    
    // 12. 停止采集
    GXStreamOff(device);
    
    // 13. 关闭设备
    GXCloseDevice(device);
    
    // 14. 释放SDK
    GXCloseLib();
    
    cout << "========================================" << endl;
    cout << "程序结束，共采集 " << frame_count << " 帧" << endl;
    if (save_count > 0) {
        cout << "已保存 " << save_count << " 帧图像" << endl;
    }
    cout << "========================================" << endl;
    
    return 0;
}
