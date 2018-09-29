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
int     thread_stage1_cnt = 0;      //# of threads that have finished stage1
int     thread_stage2_cnt = 0;      //                                stage2-1
int     thread_stage3_cnt = 0;      //                                stage2-2
int     thread_cnt = 0;             //# of threads that have finished all 3 stages
int     got_prize = 0;              //# of people that have get the prizes

// Record machine data
int     G;
int     g_cnt = 0;

struct customer{
    int arrive;
    int continuous;
    int rest;
    int N;
    int id;
    int round_cnt;
};

struct grab_machine{
    int     in_use = 0;
    int     playing_id = -1;
    pthread_mutex_t     machine_mutex;
}m1, m2;

struct sync{
    pthread_cond_t  other_thread;
    pthread_mutex_t mutex; 
}sync_thread1, sync_thread2, sync_thread3;

struct switch1{
    pthread_cond_t  child_thread;
    pthread_mutex_t mutex;
}switch_thread;

pthread_mutex_t g_mutex;
pthread_mutex_t compete_machine;
priority_queue<int, vector<int>, greater<int> > pq;

int main(int argc, char const *argv[]){
    FILE *fp = fopen(argv[1], "r");
    int g, customer_num;
    fscanf(fp, "%d%d", &g, &customer_num);
    //scanf("%d%d", &g, &customer_num);
    G = g;

    pthread_t       tid[customer_num];
    pthread_attr_t  attr[customer_num];  
    struct customer cus[customer_num];

    /* Read input from files */
    for(int i = 0; i < customer_num; i++){ 
        fscanf(fp, "%d%d%d%d", &cus[i].arrive, &cus[i].continuous, &cus[i].rest, &cus[i].N);
        //scanf("%d%d%d%d", &cus[i].arrive, &cus[i].continuous, &cus[i].rest, &cus[i].N);
        cus[i].id = i+1;
        cus[i].round_cnt = 0;
        pq.push(cus[i].arrive);
    }

    pthread_cond_init  (&switch_thread.child_thread, NULL);
    pthread_mutex_init (&switch_thread.mutex, NULL);
    pthread_cond_init  (&sync_thread1.other_thread, NULL);
    pthread_mutex_init (&sync_thread1.mutex, NULL);
    pthread_cond_init  (&sync_thread2.other_thread, NULL);
    pthread_mutex_init (&sync_thread2.mutex, NULL);
    pthread_cond_init  (&sync_thread3.other_thread, NULL);
    pthread_mutex_init (&sync_thread3.mutex, NULL);
    pthread_mutex_init (&m1.machine_mutex, NULL);
    pthread_mutex_init (&m2.machine_mutex, NULL);
    pthread_mutex_init (&g_mutex, NULL);
    pthread_mutex_init (&compete_machine, NULL);
    m2.playing_id = -2;

    /*Create User threads*/
    for(int i = 0; i < customer_num; i++){		
        pthread_create(&tid[i], NULL, user, &cus[i]); 
    }
    
    
    while(got_prize < customer_num){
        /* SYNC1: Make sure each thread has done "Finish stage (i.e. stage1)" */
        if(thread_stage1_cnt == customer_num-got_prize){
            thread_stage1_cnt = 0;
            while(1){
                if(pthread_mutex_trylock(&sync_thread1.mutex) == 0){
                    pthread_mutex_unlock(&sync_thread1.mutex);
                    pthread_cond_broadcast(&sync_thread1.other_thread);
                    break;
                }
            }        
        }
        /* SYNC2: Make sure each thread has done "Start stage part 1 (i.e. stage2-1)" */
        if(thread_stage2_cnt == customer_num-got_prize){
            thread_stage2_cnt = 0;
            while(1){
                if(pthread_mutex_trylock(&sync_thread2.mutex) == 0){
                    pthread_mutex_unlock(&sync_thread2.mutex);
                    pthread_cond_broadcast(&sync_thread2.other_thread);
                    break;
                }
            }        
        }
        /* SYNC3: Make sure each thread has done "Start stage part 2 (i.e. stage2-2)" */
        if(thread_stage3_cnt == customer_num-got_prize){
            thread_stage3_cnt = 0;
            while(1){
                if(pthread_mutex_trylock(&sync_thread3.mutex) == 0){
                    pthread_mutex_unlock(&sync_thread3.mutex);
                    pthread_cond_broadcast(&sync_thread3.other_thread);
                    break;
                }
            }        
        }
        /* CLOCK: Make sure each thread has done all 3 stages of a round, then move to next round(by broadcast)*/
        if(thread_cnt == customer_num-got_prize){
            if(!m1.in_use && !m2.in_use) g_cnt = 0;

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
        //machine1 is used by current user(thread)
        if(m1.in_use && m1.playing_id == cus_info.id){  
            cus_info.round_cnt++;   //record the number of rounds that the user had played
            int lock_success1 = -1;  
            lock_success1 = pthread_mutex_lock(&g_mutex);            

            if(lock_success1 == 0){             //access machine1 successfully, then Start playing
                g_cnt++;
                
                //Finish playing(by G), GET prize
                if(g_cnt == G){
                    printf("%2d  %d  Finish playing YES #1 \n", global_clock, cus_info.id);
                    got_prize++;
                    g_cnt = 0;      
                    m1.in_use = 0;
                    m1.playing_id = -1;
                    pthread_mutex_unlock(&g_mutex);
                    pthread_mutex_unlock(&m1.machine_mutex);
                    pthread_exit(0);
                }
                pthread_mutex_unlock(&g_mutex);
            }

            //Finish playing(by N), GET prize
            if(cus_info.N == cus_info.round_cnt){      
                printf("%2d  %d  Finish playing YES #1\n", global_clock, cus_info.id);
                got_prize++;
                g_cnt = 0;
                m1.in_use = 0;
                m1.playing_id = -1;
                //pthread_mutex_unlock(&sync_thread1.mutex);
                pthread_mutex_unlock(&m1.machine_mutex);
                pthread_exit(0);
            }

            //Finish playing, did NOT get prize
            else if(cus_info.round_cnt % cus_info.continuous == 0){ 
                printf("%2d  %d  Finish playing NO  #1\n", global_clock, cus_info.id);
                cus_info.arrive = global_clock + cus_info.rest;   //update the customer's new arrival time
                pq.push(cus_info.arrive);
                m1.in_use = 0;
                m1.playing_id = -1;
                pthread_mutex_unlock(&m1.machine_mutex);
            }
        }
        //machine2 is used by current user(thread)
        else if(m2.in_use && m2.playing_id == cus_info.id){
            cus_info.round_cnt++;

            int lock_success2 = -1;
            lock_success2 = pthread_mutex_lock(&g_mutex);            
            if(lock_success2 == 0){             //access machine2 successfully, then Start playing
                g_cnt++;
                //Finish playing(by G), GET prize
                if(g_cnt == G ){
                    printf("%2d  %d  Finish playing YES #2 \n", global_clock, cus_info.id);
                    got_prize++;
                    g_cnt = 0;
                    m2.in_use = 0;
                    m2.playing_id = -2;
                    pthread_mutex_unlock(&g_mutex);
                    pthread_mutex_unlock(&m2.machine_mutex);
                    pthread_exit(0);
                }
                pthread_mutex_unlock(&g_mutex);
            }

            //Finish playing(by G), GET prize
            if(cus_info.N == cus_info.round_cnt){      
                printf("%2d  %d  Finish playing YES #2\n", global_clock, cus_info.id);
                got_prize++;
                g_cnt = 0;
                m2.in_use = 0;
                m2.playing_id = -2;
                pthread_mutex_unlock(&m2.machine_mutex);
                pthread_exit(0);
            }
            //Finish playing, did NOT get prize
            else if(cus_info.round_cnt % cus_info.continuous == 0){ 
                printf("%2d  %d  Finish playing NO  #2\n", global_clock, cus_info.id);
                cus_info.arrive = global_clock + cus_info.rest;   //update the customer's new arrival time
                pq.push(cus_info.arrive);
                m2.in_use = 0;
                m2.playing_id = -2;
                pthread_mutex_unlock(&m2.machine_mutex);
            }
        }

        /*make sure each running thread has finished stage1*/
        pthread_mutex_lock(&sync_thread1.mutex);
        thread_stage1_cnt++;   //record the number of users that had finish stage1
        pthread_cond_wait(&sync_thread1.other_thread, &sync_thread1.mutex);    //stucked at here, waiting for broadcast(&&sync_thread1.other_thread)
        pthread_mutex_unlock(&sync_thread1.mutex);
        
        
        //STAGE2  : START playing
        //STAGE2-1: BOTH machines are idle, so users need to compete to use
        if(!m1.in_use && !m2.in_use){       
            if(pq.empty() && cus_info.arrive <= global_clock && cus_info.arrive == pq.top()){
                            
                int lock_access_machine = -1;
                lock_access_machine = pthread_mutex_trylock(&compete_machine);  //trylock: try to get one of the machine to use, if trylock fail, then go to stage2-2
                if(lock_access_machine == 0){                                   //trylock success: randomly choose one machine to use
                    srand(time(0));
                    machine_selector = rand() % 2 + 1;  // ramdomly choose one machine to use
                    printf("machine id:%d\n", machine_selector);
                    
                    if(machine_selector == 1){ 
                        int lock_success = -1;
                        lock_success = pthread_mutex_trylock(&m1.machine_mutex);

                        if(lock_success == 0){         //access machine1 successfully, then Start playing
                            pq.pop();
                            m1.playing_id = cus_info.id;
                            m1.in_use = 1;
                            printf("%2d  %d  Start playing #1\n", global_clock, cus_info.id);
                        }         
                    }else if(machine_selector == 2){
                        int lock_success2 = -1;
                        lock_success2 = pthread_mutex_trylock(&m2.machine_mutex);

                        if(lock_success2 == 0){       //access machine2 successfully, then Start playing
                            pq.pop();
                            m2.playing_id = cus_info.id;
                            m2.in_use = 1;
                            printf("%2d  %d  Start playing #2\n", global_clock, cus_info.id);
                        }
                    }
                    pthread_mutex_unlock(&compete_machine);
                }
            }
        }    
        
        /*make sure each running thread has finished stage2-1*/
        pthread_mutex_lock(&sync_thread2.mutex);
        thread_stage2_cnt++;   //record the number of users that had finish stage2(compete machine)
        pthread_cond_wait(&sync_thread2.other_thread, &sync_thread2.mutex);
        pthread_mutex_unlock(&sync_thread2.mutex);
        
        //STAGE2  : START playing
        //STAGE2-2: ONE OR NO machine is idle, choose the idle one to use
        if(!m1.in_use && !pq.empty() && cus_info.arrive <= global_clock && cus_info.arrive == pq.top() && m2.playing_id != cus_info.id){    
            int lock_success = -1;
            lock_success = pthread_mutex_trylock(&m1.machine_mutex);

            if(lock_success == 0){             //access machine1 successfully, then Start playing
                pq.pop();
                m1.playing_id = cus_info.id;
                m1.in_use = 1;
                printf("%2d  %d  Start playing #1\n", global_clock, cus_info.id);
            }
        }else if(!m2.in_use && !pq.empty() && cus_info.arrive <= global_clock && cus_info.arrive == pq.top() && m1.playing_id != cus_info.id){
            int lock_success2 = -1;
            lock_success2 = pthread_mutex_trylock(&m2.machine_mutex);

            if(lock_success2 == 0){             //access machine2 successfully, then Start playing
                pq.pop();
                m2.playing_id = cus_info.id;
                m2.in_use = 1;
                printf("%2d  %d  Start playing #2\n", global_clock, cus_info.id);
            }
        }
        
        /*make sure each running thread has finished stage2-2*/
        pthread_mutex_lock(&sync_thread3.mutex);
        thread_stage3_cnt++;   //record the number of users that had finish stage2-2(use the left idle machine)
        pthread_cond_wait(&sync_thread3.other_thread, &sync_thread3.mutex);
        pthread_mutex_unlock(&sync_thread3.mutex);


        //STAGE3: WAIT for machine because BOTH machines are occupied
        if(m1.in_use && (m1.playing_id != cus_info.id) && (cus_info.arrive == global_clock)){ 
            if(m2.in_use && (m2.playing_id != cus_info.id) && (cus_info.arrive == global_clock))
                printf("%2d  %d  Wait in line\n", cus_info.arrive, cus_info.id);
        }
        /*
        if(global_clock >= 100){
            printf("error\n");
            pthread_mutex_unlock(&switch_thread.mutex);
            pthread_exit(0);
        }
        */
        //printf("Customer_id:%d   Thread_cnt:%d   Clock=%d\n", cus_info.id, thread_cnt, global_clock);        
        
        /*make sure each running thread has finished all 3 stages*/
        pthread_mutex_lock(&switch_thread.mutex);
        thread_cnt++;               //record the number of threads that had finish all calcution of the round
        pthread_cond_wait(&switch_thread.child_thread, &switch_thread.mutex);
        pthread_mutex_unlock(&switch_thread.mutex);
    }
}