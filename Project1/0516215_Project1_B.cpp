#include<cstdio>
#include<cstdlib>
#include<time.h>
#include<pthread.h>
#include<unistd.h>
#include<queue>
#include<iostream>
using namespace std;

void    *user(void *arg);


int     global_clock = 0;   // current rounds
int     machine_selector = 0;

//Record user(thread) data
int     check_fin = 0;      //# of threads that have finished stage1 of a round 
int     thread_cnt = 0;     //# of threads that have finished all 3 stages of a round
int     finish_cnt = 0;     //# of people that have get the prizes

// Record machine data
int     G;
int     g_cnt = 0;
struct grab_machine{
    int     in_use = 0;
    int     playing_id = -1;
    pthread_mutex_t     machine_mutex; //add m2
}m1, m2;


struct customer{
    int arrive;
    int continuous;
    int rest;
    int N;
    int id;
    int round_cnt;
};

struct sync{
    pthread_cond_t  other_thread;
    pthread_mutex_t mutex; 
}sync_thread;

struct switch1{
    pthread_cond_t  child_thread;
    pthread_mutex_t mutex;
}switch_thread;

pthread_mutex_t g_mutex;
//pthread_mutex_t machine_mutex; //add m2

priority_queue<int, vector<int>, greater<int> > pq;

int main(int argc, char const *argv[]){
    int g, customer_num;
    scanf("%d%d", &g, &customer_num);
    G = g;

    pthread_t       tid[customer_num];
    pthread_attr_t  attr[customer_num];  
    struct customer cus[customer_num];

    /* Read input from files */
    for(int i = 0; i < customer_num; i++){ 
        scanf("%d%d%d%d", &cus[i].arrive, &cus[i].continuous, &cus[i].rest, &cus[i].N);
        cus[i].id = i+1;
        cus[i].round_cnt = 0;
        pq.push(cus[i].arrive);
    }

    pthread_cond_init  (&switch_thread.child_thread, NULL);
    pthread_mutex_init (&switch_thread.mutex, NULL);
    pthread_cond_init  (&sync_thread.other_thread, NULL);
    pthread_mutex_init (&sync_thread.mutex, NULL);
    pthread_mutex_init (&m1.machine_mutex, NULL);
    pthread_mutex_init (&m2.machine_mutex, NULL);
    pthread_mutex_init (&g_mutex, NULL);
    m2.playing_id = -2;

    /*Create User threads*/
    for(int i = 0; i < customer_num; i++){		
        pthread_create(&tid[i], NULL, user, &cus[i]); 
    }
    
    
    while(finish_cnt < customer_num){
        /* SYNC: Make sure each thread has done "Finish step (i.e. stage1)" */
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

        /* clock: Make sure each thread has done all computation of a round, then move to next round(by broadcast)*/
        if(thread_cnt == customer_num-finish_cnt){
            if(!m1.in_use && !m2.in_use) g_cnt = 0;
            //printf("g_cnt:%d\n", g_cnt);


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


        //STAGE1: USING machine
        if(m1.in_use && m1.playing_id == cus_info.id){  
            cus_info.round_cnt++;   //record the number of rounds that the user had played
            int lock_success2 = -1;
            lock_success2 = pthread_mutex_lock(&g_mutex);            


            if(lock_success2 == 0){             //access machine2 successfully, then Start playing
                g_cnt++;
                printf("clk:%d  g_cnt:%d\n", global_clock, g_cnt);

                if(g_cnt == G){
                    printf("t=%2d   id:%d    Finish playing YES #1 G\n", global_clock, cus_info.id);
                    finish_cnt++;
                    g_cnt = 0;
                    m1.in_use = 0;
                    m1.playing_id = -1;
                    pthread_mutex_unlock(&g_mutex);
                    pthread_mutex_unlock(&m1.machine_mutex);
                    pthread_exit(0);
                }
                //printf("clk:%d  g_cnt:%d\n", global_clock, g_cnt);
                pthread_mutex_unlock(&g_mutex);
            }
            
            
            //Finish playing, GET prize
            if(cus_info.N == cus_info.round_cnt){      
                printf("t=%2d   id:%d    Finish playing YES #1\n", global_clock, cus_info.id);
                finish_cnt++;
                g_cnt = 0;
                m1.in_use = 0;
                m1.playing_id = -1;
                //pthread_mutex_unlock(&sync_thread.mutex);
                pthread_mutex_unlock(&m1.machine_mutex);
                pthread_exit(0);
            }
            //Finish playing, did NOT get prize
            else if(cus_info.round_cnt % cus_info.continuous == 0){ 
                printf("t=%2d   id:%d    Finish playing NO #1\n", global_clock, cus_info.id);
                cus_info.arrive = global_clock + cus_info.rest;   //update the customer's new arrival time
                pq.push(cus_info.arrive);
                m1.in_use = 0;
                m1.playing_id = -1;
                pthread_mutex_unlock(&m1.machine_mutex);
            }

        }else if(m2.in_use && m2.playing_id == cus_info.id){
            cus_info.round_cnt++;

            int lock_success2 = -1;
            lock_success2 = pthread_mutex_lock(&g_mutex);            
            if(lock_success2 == 0){             //access machine2 successfully, then Start playing
                g_cnt++;
                printf("clk:%d  g_cnt:%d\n", global_clock, g_cnt);
                
                if(g_cnt == G ){
                    
                    printf("t=%2d   id:%d    Finish playing YES #2 G\n", global_clock, cus_info.id);
                    finish_cnt++;
                    g_cnt = 0;
                    m2.in_use = 0;
                    m2.playing_id = -2;
                    pthread_mutex_unlock(&g_mutex);
                    pthread_mutex_unlock(&m2.machine_mutex);
                    pthread_exit(0);
                }
                //printf("clk:%d  g_cnt:%d\n", global_clock, g_cnt);
                pthread_mutex_unlock(&g_mutex);
            }

            //Finish playing, GET prize
            if(cus_info.N == cus_info.round_cnt){      
                printf("t=%2d   id:%d    Finish playing YES #2\n", global_clock, cus_info.id);
                finish_cnt++;
                g_cnt = 0;
                m2.in_use = 0;
                m2.playing_id = -2;
                pthread_mutex_unlock(&m2.machine_mutex);
                pthread_exit(0);
            }
            //Finish playing, did NOT get prize
            else if(cus_info.round_cnt % cus_info.continuous == 0){ 
                printf("t=%2d   id:%d    Finish playing NO #2\n", global_clock, cus_info.id);
                cus_info.arrive = global_clock + cus_info.rest;   //update the customer's new arrival time
                pq.push(cus_info.arrive);
                m2.in_use = 0;
                m2.playing_id = -2;
                pthread_mutex_unlock(&m2.machine_mutex);
            }


        }

        //pthread_mutex_unlock(&g_mutex);



        pthread_mutex_lock(&sync_thread.mutex);
        check_fin++;   //record the number of users that had finish stage1
        pthread_cond_wait(&sync_thread.other_thread, &sync_thread.mutex);
        pthread_mutex_unlock(&sync_thread.mutex);
        
        
        //STAGE2: START playing
        
        if(!m1.in_use && !m2.in_use){
            srand(time(0));
            machine_selector = rand() % 2;
            // ramdomly choose machine to use
        }else if(!m1.in_use && m2.in_use){
            machine_selector = 0;
        }else if(m1.in_use && !m2.in_use){
            machine_selector = 1;
        }else if(m1.in_use && m2.in_use){
            machine_selector = -1;
        }


        //usleep(10000);
        if(machine_selector == 0 && !pq.empty() && cus_info.arrive <= global_clock && cus_info.arrive == pq.top() && m2.playing_id != cus_info.id){    

            int lock_success = -1;
            lock_success = pthread_mutex_trylock(&m1.machine_mutex);

            if(lock_success == 0){             //access machine1 successfully, then Start playing
                pq.pop();
                m1.playing_id = cus_info.id;
                m1.in_use = 1;
                printf("t=%2d   id:%d    Start playing #1\n", global_clock, cus_info.id);
            }
            /*else{                            //didn't access the machine1, so WAIT
                if(cus_info.arrive == global_clock)
                    printf("t=%2d   id:%d    Waiting \n", cus_info.arrive, cus_info.id);
            }*/
        }else if(machine_selector == 1 && !pq.empty() && cus_info.arrive <= global_clock && cus_info.arrive == pq.top() && m1.playing_id != cus_info.id){
            int lock_success2 = -1;
            lock_success2 = pthread_mutex_trylock(&m2.machine_mutex);

            if(lock_success2 == 0){             //access machine2 successfully, then Start playing
                pq.pop();
                m2.playing_id = cus_info.id;
                m2.in_use = 1;
                printf("t=%2d   id:%d    Start playing #2\n", global_clock, cus_info.id);
            }
        }
        
        //STAGE3: WAIT for machine
        if(m1.in_use && (m1.playing_id != cus_info.id) && (cus_info.arrive == global_clock)){ 
            if(m2.in_use && (m2.playing_id != cus_info.id) && (cus_info.arrive == global_clock))
                printf("t=%2d   id:%d    Waiting \n", cus_info.arrive, cus_info.id);
        }

        if(global_clock >= 100){
            printf("error\n");
            pthread_mutex_unlock(&switch_thread.mutex);
            pthread_exit(0);
        }

        //printf("Customer_id:%d   Thread_cnt:%d   Clock=%d\n", cus_info.id, thread_cnt, global_clock);        
        pthread_mutex_lock(&switch_thread.mutex);
        thread_cnt++;               //record the number of threads that had finish all calcution of the round
        pthread_cond_wait(&switch_thread.child_thread, &switch_thread.mutex);
        pthread_mutex_unlock(&switch_thread.mutex);
    }
}