#include<pthread.h>
#include<iostream>
#include<cstdlib>
#include<sys/types.h>
#include<unistd.h>
#include<time.h>
#include<string.h>
#include<fstream>
#include<list>

using namespace std;

typedef struct{
	int id;
    int arrive_time;
    int play_round;
    int rest_time;
    int total_round;
	int next_play_time = 0;
	int round_counter = 0;
} customer_data;

struct {
	pthread_mutex_t	mutex;
	int	now_player;
} claw_machine;

struct {
	pthread_mutex_t	mutex;
	pthread_cond_t child_thread;
} thread_switcher;

int G, G_counter, cus_no_prize;
int now_player = -1;
int time_counter = 0;
int thread_counter = 0;

void *customer(customer_data *cus_address);

int main(int argc, char *argv[]){
	// ifstream fin(argv[1]);
    int total_customer;

	claw_machine.now_player = -1;

	// fin >> G >> total_customer;
	cin >> G >> total_customer;
	cus_no_prize = total_customer;
	
	pthread_t tid;
	pthread_attr_t attr[total_customer];
	

	(void)	pthread_mutex_init(&claw_machine.mutex, NULL);
	(void)	pthread_mutex_init(&thread_switcher.mutex, NULL);
	(void)	pthread_cond_init(&thread_switcher.child_thread, NULL);




    customer_data cus[total_customer];
	for(int i = 0; i < total_customer; i++){
		cus[i].id = i;
        // fin >> cus[i].arrive_time >> cus[i].play_round >> cus[i].rest_time >> cus[i].total_round;
		cin >> cus[i].arrive_time >> cus[i].play_round >> cus[i].rest_time >> cus[i].total_round;
        cus[i].next_play_time = cus[i].arrive_time;
    }

	for(int i = 0; i < total_customer; i++){
		(void)	pthread_attr_init(&attr[i]);		
		(void) 	pthread_attr_setdetachstate(&attr[i], PTHREAD_CREATE_DETACHED);
		pthread_create(&tid, &attr[i], (void *(*)(void *))customer, &cus[i]);
	}
    // fin.close();

	G_counter = 0;
	
	while(cus_no_prize){
		if(thread_counter == cus_no_prize){
			if(now_player != -1){
				G_counter++;
			}else G_counter = 0;
			time_counter++;
			thread_counter = 0;
			(void) pthread_cond_broadcast(&thread_switcher.child_thread);
		}
	}
	cout << time_counter << endl;
}

void *customer(customer_data *cus_address){
	while(1){
		(void) pthread_mutex_lock(&thread_switcher.mutex);
		customer_data cus = *cus_address;
/*
		// if(now_player == cus.id){
		// 	/* Finish Part 
		// 	if(cus.round_counter == cus.total_round || G_counter == G){ // Get prize
        //         cus_no_prize--;
        //         cout << time_counter << " " << now_player+1 << " " << "finish playing YES" << endl;
        //         G_counter = 0;
        //         now_player = -1;
		// 		(void) pthread_mutex_unlock(&claw_machine.mutex);
		// 		pthread_exit(0);
		// 	}else if(cus.round_counter%cus.play_round == 0 && cus.round_counter > 0){
		// 		cout << time_counter << " " << now_player+1 << " " << "finish playing NO" << endl;
        //         cus.next_play_time = time_counter + cus.rest_time;
		// 		(void) pthread_mutex_unlock(&claw_machine.mutex);
		// 	}

		// 	now_player = -1;
		// }

		// /* Wait Part 
		// if(cus.next_play_time == time_counter){
		// 	if(now_player != -1){ // Someone is playing now
		// 		cout << time_counter << " " << cus.id+1 << " " << "wait in line" << endl;
		// 	}else{ // No one is playing, but the first guy in the queue is going to play
				
		// 		(void) pthread_mutex_lock(&claw_machine.mutex);

		// 		if(i != 1){ // People who wait behind the first guy still have to wait
		// 			cout << time_counter << " " << (*it)+1 << " " << "wait in line" << endl;
		// 		}
		// 	}
		// }
*/
		if(time_counter == 30){
			cus_no_prize--;
			cout << cus.id << endl;
			(void) pthread_mutex_unlock(&thread_switcher.mutex);
			pthread_exit(0);
		}		
		printf("Customer_id:%d   Thread_cnt:%d   Clock=%d\n",cus.id, thread_counter, time_counter);
		thread_counter++;
		//cout << cus.id << " " << time_counter << endl;
		//printf("Thread_id:%lu   Customer_id:%d   Thread_cnt:%d   Clock=%d\n",pthread_self(), cus.id, thread_counter, time_counter);
		(void) pthread_cond_wait(&thread_switcher.child_thread, &thread_switcher.mutex);
		(void) pthread_mutex_unlock(&thread_switcher.mutex);
	}
}