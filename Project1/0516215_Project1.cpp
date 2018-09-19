#include<iostream>
#include<algorithm>
#include<queue>
using namespace std;

struct customer{
    int arrive;
    int rest;
    int N;
};

bool cmp(const customer A, const customer B){
    return A.arrive < B.arrive;
}


int main(int argc, char const *argv[]){
    int g, customer_num;
    struct customer cus[1000];

    cin >> g >> customer_num;
    for(int i = 0; i < customer_num; i++){
        cin >> cus[i].arrive >> cus[i].rest >> cus[i].N;
    }
    
    
    while(1){
        
    }
    



    cout << g+1 << endl;
    cout << argc << endl;
    return 0;
}

