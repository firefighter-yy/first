#include<iostream>
using namespace std;
int main()
{
    cout<<"请输入小于100的偶数"<<"\n";
    int n;
    cin>>n;
    cout<<endl;
    cout<<n<<endl;

    for(int i=1;i<=n;i++)
    {
        for(int j=1;j<=n;j++)
        {
            if(i%2==1){

                cout<<(j<=i?"0":"1");
        }
                    else{

                        cout<<(j<=i?"1":"0");
        }
            if(j<n)
                cout<<" ";
        }
        cout<<endl;
    }
    return 0;
}