#include        <pthread.h>
#include        <stdlib.h>
#include        <stdio.h>
#include        <sys/types.h>
#include        <unistd.h>
#include        <time.h>
#include		<string.h>

int     sum, total;
int 	clk = 0;
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
	int	top;
	item	buffer[BUFFER_SIZE];
} shared_stack;

main(int argc, char *argv[]) {
        int     		i;
        pthread_t       tid;
        pthread_attr_t  attr;
		
        (void)	pthread_attr_init(&attr);		
		(void) 	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		(void)	pthread_mutex_init(&shared_stack.mutex, NULL);
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
				(void) pthread_mutex_unlock(&shared_stack.mutex);
				printf("Buffer is full: Producer %d is waiting for free buffer\n", producer_id);
				sleep(10);  /*  to reduce busy waiting  */
				(void) pthread_mutex_lock(&shared_stack.mutex);
			};
			
	        shared_stack.buffer[++shared_stack.top] = data;
			printf("Producer %d insert an item %d to the buffer %d\n", data.prod_id, data.value, shared_stack.top);
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
				(void) pthread_mutex_unlock(&shared_stack.mutex);
				printf("Buffer is empty: Consumer %d is waiting for an item\n", consumer_id);
				sleep(10);   /*  to reduce busy waiting  */
				(void) pthread_mutex_lock(&shared_stack.mutex);
			};
						
			data = shared_stack.buffer[shared_stack.top--];	/* Consume an item */
			printf("Consumer %d consumer an item %d:%d from the buffer %d\n", consumer_id, data.prod_id, data.value, shared_stack.top+1);
			(void) pthread_mutex_unlock(&shared_stack.mutex);
			sleep(rand()%10 + 1);
		}
}
