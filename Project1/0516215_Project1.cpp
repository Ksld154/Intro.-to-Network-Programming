#include<iostream>
#include<cstdio>
#include<algorithm>
#include<queue>
using namespace std;

struct customer{
    int arrive;
    int continuous;
    int rest;
    int N;
    int id;
    int round_cnt;
    bool inqueue;
};

struct CompareArrival{
    bool operator()(const customer l, const customer r){
        return l.arrive > r.arrive;
    }
};

int main(int argc, char *argv[]){
    //FILE *fp = fopen(argv[1], "r");
    int g, customer_num;
    //fscanf(fp, "%d%d", &g, &customer_num);
    cin >> g >> customer_num;

    struct customer cus[customer_num];
    priority_queue<customer, vector<customer>, CompareArrival> pq;

    /* Read input from files */
    for(int i = 0; i < customer_num; i++){ 
        //fscanf(fp, "%d%d%d%d", &cus[i].arrive, &cus[i].continuous, &cus[i].rest, &cus[i].N);
        cin >> cus[i].arrive >> cus[i].continuous >> cus[i].rest >> cus[i].N;
        cus[i].id = i;
        cus[i].round_cnt = 0;
        cus[i].inqueue = 1;
        pq.push(cus[i]);
    }
    
    //sort(cus, cus+customer_num, cmp);  //sort by customer's arrival time 
    
    bool in_use = 0;
    int finish_cnt = 0;
    int t = cus[0].arrive;
    int playing_id = 0;
    int g_cnt = 0;
    
    while(finish_cnt < customer_num){
        /*
        if(in_use){
            g_cnt++;
            cus[playing_id].round_cnt++;            
        }
        else
            g_cnt = 0;
        */

        /*Using machine*/
        if(in_use){
            g_cnt++;
            cus[playing_id].round_cnt++;
            //cout << cus[playing_id].round_cnt << endl;
            if(cus[playing_id].N == cus[playing_id].round_cnt || g == g_cnt){      //Finish playing, GET prize
                cout << t << " " << playing_id+1 << " " << "finish playing YES" << endl;
                finish_cnt++;
                in_use = 0;
            }else if(cus[playing_id].round_cnt % cus[playing_id].continuous == 0){  //Finish playing, did NOT get prize
                cout << t << " " << playing_id+1 << " " << "finish playing NO" << endl; 
                cus[playing_id].arrive = t + cus[finish_cnt].rest;   //update the customer's new arrival time
                pq.push(cus[playing_id]);
                in_use = 0; 
            }  


        }else{
            g_cnt = 0;
        }

        /*Start Playing*/
        if(!pq.empty() && !in_use && t >= pq.top().arrive){  //check whether someboy is waiting and the machine isn't occupied by others
            playing_id = pq.top().id;
            pq.pop();
            in_use = 1;
            cout << t << " " << playing_id+1 << " " << "start playing" << endl;
        }

        /*search waiting queue*/
        for(int i = 0; i < customer_num; i++){
            if(cus[i].arrive == t && in_use && i != playing_id){
                cout << cus[i].arrive << " " << i+1 << " " << "wait in line" << endl;
            }
        }
        t++;
    }
    return 0;
}

