#include<cstdio>
#include<cstdlib>
#include<pthread.h>
#include<queue>
#include<iostream>
using namespace std;

#define	BUFFER_SIZE		1

void    *user(void *arg);
//void    *consumer(int consumer_id);

struct customer{
    int arrive;
    int continuous;
    int rest;
    int N;
    int id;
    int round_cnt;
};

struct stack{
	pthread_mutex_t	mutex;
	int	top;
	struct customer	buffer[BUFFER_SIZE];
}shared_stack;


struct CompareArrival{
    bool operator()(const customer l, const customer r){
        return l.arrive > r.arrive;
    }
};

pthread_mutex_t	mutex_clk;


bool    in_use = 0;
int     global_clock = 0;
int     g_cnt;
int     G;
int     playing_id = -1;
int     finish_cnt = 0;
priority_queue<customer, vector<customer>, CompareArrival> pq;


int main(int argc, char const *argv[]){
    int g, customer_num;
    cin >> g >> customer_num;
    G = g;
    //FILE *fp = fopen(argv[1], "r");
    //fscanf(fp, "%d%d", &g, &customer_num);
 
    pthread_t       tid[customer_num];
    pthread_attr_t  attr[customer_num];  
    struct customer cus[customer_num];

    /* Read input from files */
    for(int i = 0; i < customer_num; i++){ 
        //fscanf(fp, "%d%d%d%d", &cus[i].arrive, &cus[i].continuous, &cus[i].rest, &cus[i].N);
        cin >> cus[i].arrive >> cus[i].continuous >> cus[i].rest >> cus[i].N;
        cus[i].id = i;
        cus[i].round_cnt = 0;
        pq.push(cus[i]);
    }
    
    shared_stack.top = -1;	/*  empty stack  */


    /*Create Producer threads*/
    for(int i = 0; i < customer_num; i++){
        pthread_attr_init(&attr[i]);		
        //pthread_attr_setdetachstate(&attr[i], PTHREAD_CREATE_DETACHED);
        pthread_mutex_init(&shared_stack.mutex, NULL);
        pthread_mutex_init(&mutex_clk, NULL);
        pthread_create(&tid[i], NULL, user, &cus[i]); 
    }

    for(int i = 0; i < customer_num; i++){
        pthread_join(tid[i], NULL); 
    }
    

    return 0;
}


/* How to set global_clock??? */

void *user(void *arg){
    struct customer	*cus_ptr = (struct customer *) arg;
    struct customer cus = *cus_ptr;
    
    unsigned long thread_id = pthread_self();

    printf("Thread:%ld   id:%d\n", thread_id, cus.id+1);
    printf("t=%d   id:%d  in_use:%d thread in\n", global_clock, cus.id+1, in_use);

    while(1){

        //Start Playing 
        if(!in_use && !pq.empty() && pq.top().id == cus.id && cus.arrive <= global_clock){    
            //global_clock++;
            pq.pop();
            //pthread_mutex_lock(&shared_stack.mutex);
            playing_id = cus.id;
            ++shared_stack.top;
            in_use = 1;
            //pthread_mutex_unlock(&shared_stack.mutex);
            
            printf("t=%d   id:%d  Start playing\n", global_clock, cus.id+1);
        }
        
        /*
        //machine is IDLE
        if(!in_use){
            pthread_mutex_lock(&shared_stack.mutex);
            g_cnt = 0;
            global_clock++;
            //printf("t=%d\n", global_clock);
            pthread_mutex_unlock(&shared_stack.mutex);
        }
        */


        //Using machine
        while(in_use && playing_id == cus.id){
            

            //pthread_mutex_lock(&shared_stack.mutex);
            //printf("t=%d\n", global_clock);
            global_clock++;
            g_cnt++;
            cus.round_cnt++;
            //pthread_mutex_unlock(&shared_stack.mutex);

            if(cus.N == cus.round_cnt || G == g_cnt){      //Finish playing, GET prize
                //cout << global_clock++ << " " << cus.id+1 << " " << "finish playing YES" << endl;
                printf("t=%d   id:%d  Finish playing YES\n", global_clock, cus.id+1);
                finish_cnt++;
                g_cnt = 0;
                in_use = 0;
                shared_stack.top--;
                //pthread_mutex_unlock(&shared_stack.mutex);
                pthread_exit(0);
            }else if(cus.round_cnt % cus.continuous == 0){  //Finish playing, did NOT get prize
                //cout << global_clock++ << " " << cus.id+1 << " " << "finish playing NO" << endl; 
                printf("t=%d   id:%d  Finish playing NO\n", global_clock, cus.id+1);
                cus.arrive = global_clock + cus.rest;   //update the customer's new arrival time
                pq.push(cus);
                printf("pq_top = %d\n", pq.top().id);
                in_use = 0; 
                shared_stack.top--;
                //pthread_mutex_unlock(&shared_stack.mutex);
            }
        }

        //Wait for machine
        if(in_use && (playing_id != cus.id) && (cus.arrive <= global_clock)){
            //pthread_mutex_lock(&shared_stack.mutex);
            //cout << cus.arrive << " " << global_clock << " " << "wait in line" << endl; // only the man who is the first one in the queue can use it, others must to wait 
            printf("t=%d   id:%d  Waiting \n", global_clock, cus.id+1);

            
            //pthread_mutex_unlock(&shared_stack.mutex);
        }

    }

    /*
        pthread_mutex_lock(&shared_stack.mutex);
        
        while (shared_stack.top+1 == BUFFER_SIZE){  //  do nothing, no free buffer  
            pthread_mutex_unlock(&shared_stack.mutex);
            printf("Buffer is full: Producer %d is waiting for free buffer\n", producer_id);
            //cout << cus_info->arrive << " " << cus_info->id << " " << "wait in line" << endl;

            sleep(10);  /*  to reduce busy waiting  
            pthread_mutex_lock(&shared_stack.mutex);
        };
        
        shared_stack.buffer[++shared_stack.top] = data;
        printf("Producer %d insert an item %d to the buffer %d\n", data.prod_id, data.value, shared_stack.top);
        //cout << cus_info->arrive << " " << cus_info->id << " " << "start playing" << endl;

        pthread_mutex_unlock(&shared_stack.mutex);
        
        
        sleep(rand()%10 + 1);
    */    
}