#include<cstdio>
#include<cstdlib>
#include<pthread.h>
#include<unistd.h>
#include<queue>
#include<iostream>
using namespace std;

#define	BUFFER_SIZE		1

int     global_clock = 0;
int     thread_cnt = 0;
int     G;
int     in_use = 0;
int     g_cnt = 0;
int     finish_cnt = 0;
int     playing_id = -1;

void    *user(void *arg);

struct customer{
    int arrive;
    int continuous;
    int rest;
    int N;
    int id;
    int round_cnt;
    bool waiting;
};

struct switch1{
    pthread_cond_t  child_thread;
    pthread_mutex_t mutex;
}thread_switcher;

struct machine1{
    pthread_mutex_t mutex_machine;
}machine;

struct CompareArrival{
    bool operator()(const customer l, const customer r){
        return l.arrive > r.arrive;
    }
};

priority_queue<customer, vector<customer>, CompareArrival> pq;

int main(int argc, char const *argv[]){
    int g, customer_num;
    cin >> g >> customer_num;
    //scanf("%d%d", &g, &customer_num);
    G = g;

    pthread_t       tid[customer_num];
    pthread_attr_t  attr[customer_num];  
    struct customer cus[customer_num];

    /* Read input from files */
    for(int i = 0; i < customer_num; i++){ 
        scanf("%d%d%d%d", &cus[i].arrive, &cus[i].continuous, &cus[i].rest, &cus[i].N);
        cus[i].id = i+1;
        cus[i].round_cnt = 0;
        cus[i].waiting = 0;
        pq.push(cus[i]);
    }

    pthread_cond_init(&thread_switcher.child_thread, NULL);
    pthread_mutex_init(&thread_switcher.mutex, NULL);

    /*Create Producer threads*/
    for(int i = 0; i < customer_num; i++){
        pthread_attr_init(&attr[i]);		
        pthread_create(&tid[i], NULL, user, &cus[i]); 
    }
    
    while(finish_cnt < customer_num){
        if(thread_cnt == customer_num-finish_cnt){
            if(in_use) g_cnt++;  
            else g_cnt = 0;
            
            global_clock++;
            thread_cnt = 0;
            
            usleep(20);
            pthread_cond_broadcast(&thread_switcher.child_thread);
        }
    }
    return 0;
}


void *user(void *arg){
    pthread_mutex_lock(&thread_switcher.mutex);
    struct customer	*cus_ptr = (struct customer *)arg;
    struct customer cus_info = *cus_ptr;
    
    while(1){

        /*
        //Using machine
        if(in_use && playing_id == cus_info.id){
            
            pthread_mutex_lock(&machine.mutex_machine);
            cus_info.round_cnt++;
            pthread_mutex_unlock(&machine.mutex_machine);

            if(cus_info.N == cus_info.round_cnt || G == g_cnt){      //Finish playing, GET prize

                printf("t=%d   id:%d    Finish playing YES\n", global_clock, cus_info.id);
                finish_cnt++;
                in_use = 0;
                //shared_stack.top--;

                //pthread_mutex_unlock(&shared_stack.mutex);
                pthread_exit(0);
            }else if(cus_info.round_cnt % cus_info.continuous == 0){  //Finish playing, did NOT get prize

                printf("t=%d   id:%d    Finish playing NO\n", global_clock, cus_info.id);
                cus_info.arrive = global_clock + cus_info.rest;   //update the customer's new arrival time
                pq.push(cus_info);
                printf("pq_top = %d\n", pq.top().id);
                in_use = 0; 
                //shared_stack.top--;
                //pthread_mutex_unlock(&shared_stack.mutex);
            }
        }

        //Wait for machine
        if(in_use && (playing_id != cus_info.id) && (cus_info.arrive <= global_clock) && !cus_info.waiting){
            //pthread_mutex_lock(&shared_stack.mutex);
            
            printf("t=%d   id:%d    Waiting \n", cus_info.arrive, cus_info.id);
            cus_info.waiting = 1;
            //printf("t=%d   id:%d  wait:%d    Waiting \n", cus_info.arrive, cus_info.id, cus_info.waiting);

            //pthread_mutex_unlock(&shared_stack.mutex);
        }

        //Start playing
        if(!in_use && !pq.empty() && pq.top().id == cus_info.id && cus_info.arrive <= global_clock){    
            pq.pop();
            pthread_mutex_lock(&machine.mutex_machine);
            playing_id = cus_info.id;
            in_use = 1;
            pthread_mutex_unlock(&machine.mutex_machine);
            
            printf("t=%d   id:%d    Start playing\n", global_clock, cus_info.id);
        }
        */
        //Early quit (for test)
        if(global_clock == 40){
			finish_cnt++;
			cout << cus_info.id << endl;
			(void) pthread_mutex_unlock(&thread_switcher.mutex);
			pthread_exit(0);
		}	
        
        printf("Customer_id:%d   Thread_cnt:%d   Clock=%d\n", cus_info.id, thread_cnt, global_clock);
        thread_cnt++;
        pthread_cond_wait(&thread_switcher.child_thread, &thread_switcher.mutex);
        pthread_mutex_unlock(&thread_switcher.mutex);
    }
}