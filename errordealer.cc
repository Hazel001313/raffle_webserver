#include <iostream>
#include <string>

using namespace std;
void func(int a,int b){
    if(b==0) throw("b==0\n");
    cout<<"a/b="<<a/b<<endl;
}

void func2(){
    try{
        func(5,0);
    }
    catch(int x){
        cout<<"x="<<x<<endl;
    }
}

int main(){
    try{
        func2();
    }catch(char const* err){
        cout<<"error:"<<err<<endl;
    }


}