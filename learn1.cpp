#include <iostream>
#include "GxIAPI.h"

using namespace std;

int main() {  
    GX_STATUS status=GX_STATUS_SUCCESS;
    
    //1.初始化SDK
    GX_STATUS status=GXInitLib();
    if(status!=GX_STATUS_SUCCESS){
        cout<<"失败（错误码:0x"<<hex<<status<<")"<<dec<<endl;
        return 1;
    }
    cout<<"成功"<<endl;

    //2.枚举设备
    uint32_t number=0;
    status=GXUpdateDeviceList(&number,10000);
    if(status!=GX_STATUS_SUCCESS){
        cout<<"失败"<<endl;
        GXCloseLib();
        return 1;
    }
    cout<<"发现相机数量"<<number<<endl;
    if(number==0){
        cout<<endl;
        cout<<"未检测到相机"<<endl;
        cout<<"请检查:"<<endl;
        cout<<"1.相机USB连接"<<endl;
        cout<<"2.相机电源"<<endl;
        cout<<"3.USB线是否为USB3.0"<<endl;

        GXCloseLib();
        return 1;        
    }
    else{
        cout<<endl;
        cout<<"检测到相机"<<endl;
    }

    //3.连接相机
    GX_DEV_HANDLE device=nullptr;
    status=GXOpenDeviceByIndex(1,&device);
    if(status!=GX_STATUS_SUCCESS){
        cout<<"失败(错误码:0x"<<hex<<status<<")"<<dec<<endl;
        GXCloseLib();
        return 1;
    }
    cout<<"成功"<<endl;

    //4.设置参数
    //设置连续采集模式
    GXSetEnum(device,GX_ENUM_ACQUISITION_MODE,GX_ACQ_MODE_CONTINUOUS);

    //5.开始采集
    status=GXStreamOn(device);
    if(status!=GX_STATUS_SUCCESS){
        cout<<"采集失败"<<endl;
        GXCloseDevice(device);
        GXCloseLib();
        return 1;
    }

    //6.获取图像
    //获得图像大小
    int64_t imagesSize=0;
    status=GXGetInt(device,GX_INT_PAYLOAD_SIZE,&imagesSize);
    if(status!=GX_STATUS_SUCCESS||imagesSize<=0){
        cout<<"获得图像大小失败(错误码:"<<hex<<status<<")"<<dec<<endl;
        GXStreamOff(device);
        GXCloseDevice(device);
        GXCloseLib();
        return 1;
    }

    //预分配图像缓冲区
    unsigned char* imageBuff= new unsigned char[(size_t)imagesSize];        //动态分配缓冲区
    if(imageBuff==nullptr){
        cout<<"内存分配失败"<<endl;
        GXCloseDevice(device);
        GXCloseLib();
        return 1;
    }

    //初始化帧数据结构
    GX_FRAME_DATA frameData{};              //初始化图像结构体为0
    frameData.pImgBuf=imageBuff;            //将缓冲区地址赋值给帧数据结构的图像缓冲区指针
    frameData.nImgSize=(size_t)imagesSize;  //设置缓冲区大小

    //连续采集多帧图像
    for(int i=0;i<10;i++){
        status=GXGetImage(device,&frameData,1000);
        if(status==GX_STATUS_SUCCESS){
            cout<<"第"<<i+1<<"帧"<<frameData.nWidth<<"x"<<frameData.nHeight<<"(ID:"<<frameData.nFrameID<<")"<<endl;
        }
        else{
            cout<<"第"<<i+1<<"帧"<<"帧采集失败(错误码:0x"<<hex<<status<<")"<<dec<<endl;
        }
    }

    //释放缓冲区
    delete[] imageBuff;

    //7.停止采集
    GXStreamOff(device);

    //8.关闭设备
    GXCloseDevice(device);

    //9.释放SDK
    GXCloseLib();

    return 0;
}