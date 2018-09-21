#include        <pthread.h>
#include        <stdlib.h>
#include        <stdio.h>
#include        <sys/types.h>
#include        <unistd.h>
#include        <time.h>
#include		<string.h>

int     sum, total;
void    *producer (int	producer_id);
void	*consumer (int 	consumer_id);

#define	BUFFER_SIZE		5
#define	PRODUCER_NUM	3
#define	CONSUMER_NUM	2

typedef	struct{
	int	prod_id; 		/*  producer id   */
	int	value;			/*  produced item value   */
} item;

struct {
	pthread_mutex_t	mutex;
	pthread_cond_t	notbusy;  /* there exists free buffer  */
	pthread_cond_t	notempty; /* there exists free item    */
	int				top;
	item			buffer[BUFFER_SIZE];
} shared_stack;

main(int argc, char *argv[]) {
        int     		i;
        pthread_t       tid;
        pthread_attr_t  attr;
		
        (void)	pthread_attr_init(&attr);		
		(void) 	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		(void)	pthread_mutex_init(&shared_stack.mutex, NULL);
		(void)	pthread_cond_init(&shared_stack.notbusy, NULL);
		(void)	pthread_cond_init(&shared_stack.notempty, NULL);
		shared_stack.top = -1;	/*  empty stack  */
		
		for (i=1; i<=PRODUCER_NUM; i++)
			pthread_create(&tid, &attr, (void *(*)(void *))producer, (void *)i);
		for (i=1; i<=CONSUMER_NUM; i++)
			pthread_create(&tid, &attr, (void *(*)(void *))consumer, (void *)i);
        
        while (1) {
			sleep(1000);
        };
}

void    *producer (int	producer_id) {
		item	data;

		data.prod_id = producer_id;
		while (1) {  
			data.value = rand()%1000 + 1;	/* Produce an item */
			(void) pthread_mutex_lock(&shared_stack.mutex);
			
			while (shared_stack.top+1 == BUFFER_SIZE){  
				//  do nothing, no free buffer  
				printf("Buffer is full: Producer %d is waiting for free buffer\n", producer_id);
				//  wait for the cond_variable notbusy
				(void) pthread_cond_wait(&shared_stack.notbusy, &shared_stack.mutex);
			};
	        
			shared_stack.buffer[++shared_stack.top] = data;
			printf("Producer %d insert an item %d to the buffer %d\n", data.prod_id, data.value, shared_stack.top);
			//   signal all threads blocked for cond_variable notempty   
			(void) pthread_cond_broadcast(&shared_stack.notempty);
			(void) pthread_mutex_unlock(&shared_stack.mutex);
			sleep(rand()%10 + 1);
		}
}

void	*consumer (int	consumer_id) {
		item	data;

		while (1) {  
			(void) pthread_mutex_lock(&shared_stack.mutex);
			while (shared_stack.top == -1){  
				//  do nothing, buffer is empty
				printf("Buffer is empty: Consumer %d is waiting for an item\n", consumer_id);
				// wait for the cond_variable notempty
				(void) pthread_cond_wait(&shared_stack.notempty, &shared_stack.mutex);
			};			
			data = shared_stack.buffer[shared_stack.top--];	/* Consume an item */
			printf("Consumer %d consumer an item %d:%d from the buffer %d\n", consumer_id, data.prod_id, data.value, shared_stack.top+1);
			//   signal all threads blocks for con_variable notbusy
			(void) pthread_cond_broadcast(&shared_stack.notbusy);
			(void) pthread_mutex_unlock(&shared_stack.mutex);
			sleep(rand()%10 + 1);
		}
}
