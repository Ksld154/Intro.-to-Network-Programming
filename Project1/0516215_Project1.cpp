#include<iostream>
using namespace std;

struct customer{
    int arrive;
    int rest;
    int N;
};

int main(int argc, char const *argv[]){
    int g, customer_num;
    struct customer cus[10000];

    cin >> g >> customer_num;
    for(int i = 0; i < customer_num; i++){
        cin >> cus[i].arrive >> cus[i].rest >> cus[i].N;
    }
    




    cout << g+1 << endl;
    cout << argc << endl;
    return 0;
}

