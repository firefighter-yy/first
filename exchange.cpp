#include<iostream>
using namespace std;
inline int min(int x,int y)
{
    return(x<y?x:y);
}
int main()
{
    cout<<"before exchange:";
    int a[4];
    for(int i=0;i<4;i++){
        cin>>a[i];
    }

    int m=a[0];
    for(int i=0;i<4;i++){
        m=min(a[i],m);
    }
    cout<<"最小数为："<<m<<endl;

    int minn=0;
    for(int i=0;i<4;i++){
        if(a[i]<a[minn]){
            minn=i;
        }
    }
    
    int temp=a[minn];
    a[minn]=a[3];
    a[3]=temp;

    cout<<"after exchange";
    for( int i=0;i<4;i++){
    cout<<a[i]<<" ";
    }
    return 0;
}