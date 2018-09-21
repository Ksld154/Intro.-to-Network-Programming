#include<cstdio>
#include<cstdlib>
#include<pthread.h>

#define	BUFFER_SIZE		1

int     global_clock = 0;
int     G;

void    *producer(int producer_id);
void    *consumer(int consumer_id);

struct customer{
    int arrive;
    int continuous;
    int rest;
    int N;
    int id;
    int round_cnt;
};

struct {
	pthread_mutex_t	mutex;
	int	top;
	struct customer	buffer[BUFFER_SIZE];
}shared_stack;


struct CompareArrival{
    bool operator()(const customer l, const customer r){
        return l.arrive > r.arrive;
    }
};

/*
typedef	struct{
	int	prod_id; 		// producer id   
	int	value;			//  produced item value   
} item;
*/



int main(int argc, char const *argv[]){
    int g, customer_num;
    cin >> g >> customer_num;
    G = g;
    //FILE *fp = fopen(argv[1], "r");
    //fscanf(fp, "%d%d", &g, &customer_num);
 
    pthread_t       tid[customer_num];
    pthread_attr_t  attr[customer_num];  
    struct customer cus[customer_num];
    priority_queue<customer, vector<customer>, CompareArrival> pq;

    /* Read input from files */
    for(int i = 0; i < customer_num; i++){ 
        //fscanf(fp, "%d%d%d%d", &cus[i].arrive, &cus[i].continuous, &cus[i].rest, &cus[i].N);
        cin >> cus[i].arrive >> cus[i].continuous >> cus[i].rest >> cus[i].N;
        cus[i].id = i;
        cus[i].round_cnt = 0;
        pq.push(cus[i]);
    }
    

    shared_stack.top = -1;	/*  empty stack  */

    //bool in_use = 0;
    int g_cnt = 0;
    int finish_cnt = 0;
    int t = cus[0].arrive;
    int playing_id = 0;


    /*Create Producer threads*/
    for(int i = 0; i < customer_num; i++){
        pthread_attr_init(&attr[i]);		
        pthread_attr_setdetachstate(&attr[i], PTHREAD_CREATE_DETACHED);
        pthread_mutex_init(&shared_stack.mutex, NULL);
        pthread_create(&tid[i], &attr[i], (void *(*)(void *))producer, &cus[i]); 
    }
    /*
    while(finish_cnt < customer_num){
        if(in_use){
            g_cnt++;
            cus[playing_id].round_cnt++;

            if(cus[playing_id].N == cus[playing_id].round_cnt || g == g_cnt){      //Finish playing, GET prize
                cout << t << " " << playing_id+1 << " " << "finish playing YES" << endl;
                finish_cnt++;
                g_cnt = 0;
                in_use = 0;
                playing_id = -1;
            }else if(cus[playing_id].round_cnt % cus[playing_id].continuous == 0){  //Finish playing, did NOT get prize
                cout << t << " " << playing_id+1 << " " << "finish playing NO" << endl; 
                cus[playing_id].arrive = t + cus[playing_id].rest;   //update the customer's new arrival time
                pq.push(cus[playing_id]);
                in_use = 0;
                playing_id = -1; 
            }  

        }else{
            g_cnt = 0;
        }

        /*search waiting queue*-/
        for(int i = 0; i < customer_num; i++){
            if(cus[i].arrive == t){
                if(in_use && i != playing_id)         //the machine is BUSY, so others must to wait
                    cout << cus[i].arrive << " " << i+1 << " " << "wait in line" << endl;
                else if(!in_use && i != pq.top().id)  //the machine is IDLE, but there are multiple users that can use it immediatly,
                    cout << cus[i].arrive << " " << i+1 << " " << "wait in line" << endl; // only the man who is the first one in the queue can use it, others must to wait 
            }
        }

        /*Start Playing*-/
        if(!in_use && !pq.empty() && pq.top().arrive <= t){  //check whether somebody is waiting and the machine is idle
            playing_id = pq.top().id;
            pq.pop();
            in_use = 1;
            cout << t << " " << playing_id+1 << " " << "start playing" << endl;
        }
        
        t++;
    }
    */




    /*
    for (int i = 0; i < customer_num; i++) {
        pthread_join(tid[i], NULL);
        printf("sum in child thread %ld is %d\n", (unsigned long)tid[i], sum[i]);
        //total += sum[i];
    };
    */
   /*
    while (1) {
        sleep(1000);
    };
    */
    return 0;
}


void *producer(void *param){
    struct customer	*cus_info;
    cus_info = (struct customer *) param;

    while (1) { // need to modify the condition

        //data.value = rand()%1000 + 1;	/* Produce an item */
        (void) pthread_mutex_lock(&shared_stack.mutex);
        
        while (shared_stack.top+1 == BUFFER_SIZE){  //  do nothing, no free buffer  
            (void) pthread_mutex_unlock(&shared_stack.mutex);
            printf("Buffer is full: Producer %d is waiting for free buffer\n", producer_id);
            //cout << cus_info->arrive << " " << cus_info->id << " " << "wait in line" << endl;

            sleep(10);  /*  to reduce busy waiting  */
            (void) pthread_mutex_lock(&shared_stack.mutex);
        };
        
        shared_stack.buffer[++shared_stack.top] = data;
        printf("Producer %d insert an item %d to the buffer %d\n", data.prod_id, data.value, shared_stack.top);
        //cout << cus_info->arrive << " " << cus_info->id << " " << "start playing" << endl;

        (void) pthread_mutex_unlock(&shared_stack.mutex);
        sleep(rand()%10 + 1);
    }
}