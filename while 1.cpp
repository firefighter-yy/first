#include<iostream>
using namespace std;
int main()
{
    int i=1;
    while(i<10){
        int j=1;
        while(j<=i){
            cout<<j<<"*"<<i<<"="<<j*i<<"   ";
            j++;
        }
        cout<<endl;
        i++;
    }
    return 0;
}