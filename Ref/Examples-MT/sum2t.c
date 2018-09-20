#include        <pthread.h>
#include        <stdlib.h>
#include        <stdio.h>
#include        <sys/types.h>
#include        <unistd.h>
#include        <sys/wait.h>

int     sum, total;
void    *runner (void *param);

main(int argc, char *argv[]) {
        int     i, lower;
        pthread_t       tid;
        pthread_attr_t  attr;
        pthread_attr_init(&attr);
        pthread_create(&tid, &attr, runner, argv[1]);
        total = 0;
        lower = atoi(argv[1]);
        for (i=lower+1; i<=lower+10; i++) {
                total += i;
                printf("The value of i in main thread %ld is %d\n", (unsigned long)pthread_self(), i);
                usleep(10);
        };
        pthread_join(tid, NULL);  // wait for the child thread to complete
        printf("The sum from %d to %d in main thread %ld is %d\n", lower+1, lower+10, (unsigned long)pthread_self(), total);
        total += sum;
        printf("total in main thread %ld is %d\n", (unsigned long)pthread_self(), total);
}

void    *runner (void *param) {
        int     i, upper, j;
        upper = atoi(param);
        sum = 0;
        if (upper <= 0) pthread_exit(&upper);

        for (i=1; i<=upper; i++) {
                printf("The value of i in child thread %ld is %d\n", (unsigned long)pthread_self(), i);
                sum += i;
                usleep(10);
        };
        printf("The sum from 1 to %d in child thread %ld is %d\n", upper, (unsigned long)pthread_self(), sum);
        pthread_exit(0);
}