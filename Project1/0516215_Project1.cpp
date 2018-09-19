#include<iostream>
#include<algorithm>
#include<queue>
using namespace std;

struct customer{
    int arrive;
    int continuous;
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
    const int G = g;

    for(int i = 0; i < customer_num; i++){
        cin >> cus[i].arrive >> cus[i].continuous >> cus[i].rest >> cus[i].N;
    }
    sort(cus, cus+customer_num, cmp);
    
    int finish_cnt = 0;
    int t = 0;
    t = cus[0].arrive;
    
    while(finish_cnt <= customer_num){
        
        for(int i = 0; i < customer_num; i++){
            int time_fin = t;
            if(cus[i].N <= cus[i].continuous){
                cout << t+cus[i].N << " " << i << " " << "finish playing YES" << endl;
                g = G;
            }
            else if(g <= cus[i].continuous){
                cout << t+g << " " << i << " " << "finish playing YES" << endl;
                g = G;
            }
            else{
                g -= cus[i].continuous;
                cus[i].N -= cus[i].continuous;
                //time_fin += cus[i].continuous;
                cus[i].arrive += cus[i].continuous;
            }
        }
        sort(cus, cus+customer_num, cmp);
    }
    



    cout << g+1 << endl;
    cout << argc << endl;
    return 0;
}

