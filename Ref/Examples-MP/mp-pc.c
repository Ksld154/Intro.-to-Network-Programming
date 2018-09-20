#include    <stdlib.h>
#include    <stdio.h>
#include    <sys/types.h>
#include    <unistd.h>
#include    <time.h>
#include	<string.h>
#include	<semaphore.h>
#include	<sys/mman.h>
#include 	<sys/stat.h>        /* For mode constants */
#include 	<fcntl.h>

void	producer (int	producer_id);
void	consumer (int 	consumer_id);

#define	BUFFER_SIZE	5
#define	PRODUCER_NUM	3
#define	CONSUMER_NUM	2
#define	SHM_NAME		"/smyuan_share_memory"

typedef	struct{
	int	prod_id; 		/*  producer id   */
	int	value;			/*  produced item value   */
} item_t;

typedef	struct {
	sem_t	mutex;
	int		top;
	item_t	buffer[BUFFER_SIZE];
} stack_t;

stack_t	*shm_addr;

main(int argc, char *argv[]) {
    int     i, shmfd;
	char	*shm_name = SHM_NAME;
    		
	shmfd = shm_open (shm_name, O_RDWR | O_CREAT, S_IRWXU);

	ftruncate (shmfd, 1024);  //  to eliminate Bus error
	
	shm_addr = (stack_t *) mmap (NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

	sem_init (&shm_addr->mutex, 1, 1);   //  initializes the mutex
	
	shm_addr->top = -1;	/*  empty stack  */
		
	for (i=1; i<=PRODUCER_NUM; i++) {
		if (fork() == 0)
			producer(i);
	};
	
	for (i=1; i<=CONSUMER_NUM; i++) {
		if (fork() == 0)
			consumer(i);
	};
        
    while (1) {
		sleep(1000);
    };
}

void    producer (int	producer_id) {
		item_t	data;

		data.prod_id = producer_id;
		while (1) {  
			data.value = rand()%1000 + 1;	/* Produce an item */
			sem_wait(&shm_addr->mutex);  // get lock
			while (shm_addr->top+1 == BUFFER_SIZE){  
				//  do nothing, no free buffer  
				sem_post(&shm_addr->mutex);  // release lock
				printf("Buffer is full: Producer %d is waiting for free buffer\n", producer_id);
				sleep(10);  /*  to reduce busy waiting  */
				sem_wait(&shm_addr->mutex);  //  get lock again
			};
	        	shm_addr->buffer[++shm_addr->top] = data;
			printf("Producer %d insert an item %d to the buffer %d\n", data.prod_id, data.value, shm_addr->top);
			sem_post(&shm_addr->mutex);  //  release lock
			sleep(rand()%10 + 1);
		};
		exit(0);
}

void	consumer (int	consumer_id) {
		item_t	data;

		while (1) {  
			sem_wait(&shm_addr->mutex);   // get lock
			while (shm_addr->top == -1){  
				//  do nothing, buffer is empty
				sem_post(&shm_addr->mutex);   // release lock
				printf("Buffer is empty: Consumer %d is waiting for an item\n", consumer_id);
				sleep(10);   /*  to reduce busy waiting  */
				sem_wait(&shm_addr->mutex);   // get lock again
			};			
			data = shm_addr->buffer[shm_addr->top--];	/* Consume an item */
			printf("Consumer %d consumer an item %d:%d from the buffer %d\n", consumer_id, data.prod_id, data.value, shm_addr->top+1);
			sem_post(&shm_addr->mutex);   //release lock
			sleep(rand()%10 + 1);
		};
		exit(0);
}
