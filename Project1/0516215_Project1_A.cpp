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
int     check_fin = 0;

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
}switch_thread;


struct sync{
    pthread_cond_t  other_thread;
    pthread_mutex_t mutex; 
}sync_thread;

pthread_mutex_t machine_mutex;


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

    pthread_cond_init(&switch_thread.child_thread, NULL);
    pthread_mutex_init(&switch_thread.mutex, NULL);
    pthread_cond_init(&sync_thread.other_thread, NULL);
    pthread_mutex_init(&sync_thread.mutex, NULL);
    pthread_mutex_init(&machine_mutex, NULL);

    /*Create Producer threads*/
    for(int i = 0; i < customer_num; i++){
        pthread_attr_init(&attr[i]);		
        pthread_create(&tid[i], NULL, user, &cus[i]); 
    }
    
    while(finish_cnt < customer_num){
        if(check_fin == customer_num-finish_cnt){
            check_fin = 0;
            while(1){
                if(pthread_mutex_trylock(&sync_thread.mutex) == 0){
                    pthread_mutex_unlock(&sync_thread.mutex);
                    pthread_cond_broadcast(&sync_thread.other_thread);
                    break;
                }
            }        
        }



        if(thread_cnt == customer_num-finish_cnt){
            if(in_use) g_cnt++;  
            else g_cnt = 0;

            global_clock++;
            thread_cnt = 0;
            while(1){
                if(pthread_mutex_trylock(&switch_thread.mutex) == 0){
                    pthread_mutex_unlock(&switch_thread.mutex);
                    pthread_cond_broadcast(&switch_thread.child_thread);
                    break;
                }
            }
            //usleep(10000);
            //pthread_cond_broadcast(&switch_thread.child_thread);
        }
    }
    return 0;
}


void *user(void *arg){    
    struct customer	*cus_ptr = (struct customer *)arg;
    struct customer cus_info = *cus_ptr;
    
    while(1){
        pthread_mutex_lock(&sync_thread.mutex);
        //Using machine
        if(in_use && playing_id == cus_info.id){
            //pthread_cond_wait(&sync_thread.other_thread, &sync_thread.mutex);
            //pthread_mutex_unlock(&sync_thread.mutex);
            
            cus_info.round_cnt++;

            if(cus_info.N == cus_info.round_cnt || G == g_cnt){      //Finish playing, GET prize

                printf("t=%2d   id:%d    Finish playing YES\n", global_clock, cus_info.id);
                g_cnt = 0;
                finish_cnt++;
                in_use = 0;
                //pthread_cond_broadcast(&sync_thread.other_thread);
                pthread_mutex_unlock(&sync_thread.mutex);
                pthread_mutex_unlock(&machine_mutex);
                pthread_exit(0);
            }else if(cus_info.round_cnt % cus_info.continuous == 0){  //Finish playing, did NOT get prize

                printf("t=%2d   id:%d    Finish playing NO\n", global_clock, cus_info.id);
                cus_info.arrive = global_clock + cus_info.rest;   //update the customer's new arrival time
                pq.push(cus_info);
                //printf("pq_top = %d\n", pq.top().id);
                in_use = 0;
                //pthread_cond_broadcast(&sync_thread.other_thread);
                pthread_mutex_unlock(&machine_mutex);
            }
        }
        //printf("Check_fin:%d\n", check_fin);
        check_fin++;

        pthread_cond_wait(&sync_thread.other_thread, &sync_thread.mutex);
        pthread_mutex_unlock(&sync_thread.mutex);
        
        
        //Start playing(BUG)
        //usleep(10000);
        if(!in_use && !pq.empty() && cus_info.arrive <= global_clock && cus_info.arrive == pq.top().arrive){    
            //pthread_mutex_trylock(&sync_thread.mutex);  //where to put broadcast????

        

            int lock_success = -1;
            lock_success = pthread_mutex_trylock(&machine_mutex);            
            if(lock_success == 0){             //access machine successfully, then start playing
                pq.pop();
                playing_id = cus_info.id;
                in_use = 1;
                cus_info.waiting = 0;
                printf("t=%2d   id:%d    Start playing\n", global_clock, cus_info.id);
            }
            /*else{
                if(cus_info.arrive == global_clock)
                    printf("t=%2d   id:%d    Waiting \n", cus_info.arrive, cus_info.id);
            }*/
        }

        
        //Wait for machine(BUG)
        if(in_use && (playing_id != cus_info.id) && (cus_info.arrive == global_clock)){ 
            printf("t=%2d   id:%d    Waiting \n", cus_info.arrive, cus_info.id);
        }



        /*
        //Early quit (for test)
        if(global_clock == 100){
			finish_cnt++;
			cout << cus_info.id << endl;
			(void) pthread_mutex_unlock(&switch_thread.mutex);
			pthread_exit(0);
		}
    `   */

        pthread_mutex_lock(&switch_thread.mutex);

        //printf("Customer_id:%d   Thread_cnt:%d   Clock=%d\n", cus_info.id, thread_cnt, global_clock);
        thread_cnt++;
        pthread_cond_wait(&switch_thread.child_thread, &switch_thread.mutex);
        pthread_mutex_unlock(&switch_thread.mutex);
    }
}