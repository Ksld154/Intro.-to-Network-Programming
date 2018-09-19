#include<iostream>
#include<algorithm>
#include<queue>
using namespace std;

struct customer{
    int arrive;
    int continuous;
    int rest;
    int N;
    int id;
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
        cus[i].id = i+1;
    }
    sort(cus, cus+customer_num, cmp);
    
    bool use = 0;
    int finish_cnt = 0;
    int t = 0;
    t = cus[0].arrive;
    
    while(finish_cnt < customer_num){
        
        /*
        for(int tt = 0; ; tt++){

        }
        */

        //t = cus[finish_cnt].arrive;   //wrong!!

        if(t < cus[finish_cnt].arrive){
            t = cus[finish_cnt].arrive;
            g = G;
        }

        cout << t << " " << cus[finish_cnt].id << " " << "start playing" << endl;
        int time_fin = t + cus[finish_cnt].continuous;

        for(int j = finish_cnt+1; j < customer_num; j++){
            if(cus[j].arrive < time_fin){
                cout << cus[j].arrive << " " << cus[j].id << " " << "wait in line" << endl;
            }
        }


        
        if(cus[finish_cnt].N <= cus[finish_cnt].continuous){
            t = t + cus[finish_cnt].N;
            cout << t << " " << cus[finish_cnt].id << " " << "finish playing YES" << endl;
            finish_cnt++;
            g = G;
        }
        else if(g <= cus[finish_cnt].continuous){
            t = t + g;
            cout << t << " " << cus[finish_cnt].id << " " << "finish playing YES" << endl;
            finish_cnt++;
            g = G;
        }
        else{
            g -= cus[finish_cnt].continuous;
            cus[finish_cnt].N -= cus[finish_cnt].continuous;
            cus[finish_cnt].arrive = time_fin + cus[finish_cnt].rest;
            t = t + cus[finish_cnt].continuous;
            cout << t << " " << cus[finish_cnt].id << " " << "finish playing NO" << endl; 
        }
                    
        sort(cus+finish_cnt, cus+customer_num, cmp);

        for(int i = finish_cnt; i < customer_num; i++){
            //cout << cus[i].arrive << endl;
        }


        // deal with t!!
        // waiting
        //非連續operation G要回復!!
    }
    
    //cout << g+1 << endl;
    //cout << argc << endl;
    return 0;
}

