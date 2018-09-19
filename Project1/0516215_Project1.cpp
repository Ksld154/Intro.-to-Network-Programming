#include<iostream>
#include<cstdio>
#include<algorithm>
using namespace std;

struct customer{
    int arrive;
    int continuous;
    int rest;
    int N;
    int id;
    bool inqueue;
};

bool cmp(const customer A, const customer B){
    return A.arrive < B.arrive;
}

int main(int argc, char *argv[]){
    FILE *fp = fopen(argv[1], "r");
    int g, customer_num;
    fscanf(fp, "%d%d", &g, &customer_num);
    const int G = g;
    struct customer cus[customer_num];

    /* Read input from files */
    for(int i = 0; i < customer_num; i++){ 
        fscanf(fp, "%d%d%d%d", &cus[i].arrive, &cus[i].continuous, &cus[i].rest, &cus[i].N);
        cus[i].id = i+1;
        cus[i].inqueue = 0;
    }
    
    sort(cus, cus+customer_num, cmp);  //sort by customer's arrival time 
    
    //bool use = 0;
    int finish_cnt = 0;
    int t = cus[0].arrive;
    
    while(finish_cnt < customer_num){
        if(t < cus[finish_cnt].arrive){          //check whether the machine is idle
            t = cus[finish_cnt].arrive;
            g = G;
        }

        cout << t << " " << cus[finish_cnt].id << " " << "start playing" << endl;
        int time_fin = t + cus[finish_cnt].continuous;

        for(int j = finish_cnt+1; j < customer_num; j++){    //scan for customer who is waiting to use the machine
            if(cus[j].arrive < time_fin && !cus[j].inqueue){
                cout << cus[j].arrive << " " << cus[j].id << " " << "wait in line" << endl;
                cus[j].inqueue = 1;
            }
        }

        if(cus[finish_cnt].N <= cus[finish_cnt].continuous){ //get prize
            t = t + cus[finish_cnt].N;
            cout << t << " " << cus[finish_cnt].id << " " << "finish playing YES" << endl;
            finish_cnt++;
            g = G;
            cus[finish_cnt].inqueue = 0;
        }
        else if(g <= cus[finish_cnt].continuous){ // get prize    
            t = t + g;
            cout << t << " " << cus[finish_cnt].id << " " << "finish playing YES" << endl;
            finish_cnt++;
            g = G;
            cus[finish_cnt].inqueue = 0;
        }
        else{  //did not get prize
            g -= cus[finish_cnt].continuous;
            cus[finish_cnt].N -= cus[finish_cnt].continuous;
            cus[finish_cnt].arrive = time_fin + cus[finish_cnt].rest;   //update the customer's new arrival time
            t = t + cus[finish_cnt].continuous;                         //update global time
            cus[finish_cnt].inqueue = 0;
            cout << t << " " << cus[finish_cnt].id << " " << "finish playing NO" << endl; 
        }  
         
        sort(cus+finish_cnt, cus+customer_num, cmp);  //sort by customer's arrival time
        
        /*
        for(int j = finish_cnt; j < customer_num; j++){
            cout << cus[j].id  << endl;
        }
        */
    }
    return 0;
}

