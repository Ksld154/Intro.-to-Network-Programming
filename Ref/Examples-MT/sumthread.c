#include        <pthread.h>
#include        <stdlib.h>
#include        <stdio.h>
#include        <sys/types.h>
#include        <unistd.h>
#include        <sys/wait.h>

int     sum[10], total;
void    *runner (void *param);
struct  thread_infor {
        int     index;
        char    *argv_string;
};

main(int argc, char *argv[]) {  
        int     i;
        pthread_t       tid[10];
        pthread_attr_t  attr[10];
        struct  thread_infor    t_infor[10];
        //printf("%d\n", argc);
        for (i=0; i<argc-1; i++) {
                pthread_attr_init(&attr[i]);
                //printf("%d\n", i);
                t_infor[i].index = i;
                t_infor[i].argv_string = argv[i+1];
                pthread_create(&tid[i], &attr[i], runner, &t_infor[i] );
        };
        total = 0;
        //printf("%d\n", total);
        for (i=0; i<argc-1; i++) {
                pthread_join(tid[i], NULL);
                printf("sum in child thread %ld is %d\n", (unsigned long)tid[i], sum[i]);
                total += sum[i];
        };
        printf("total in main thread %ld is %d\n", (unsigned long)pthread_self(), total);
}

void    *runner (void *param) {
        int     i, upper, j;
        struct thread_infor     *tinfor;
        tinfor = (struct thread_infor *) param;
        j = tinfor->index;
//      printf("The value of j in thread %d is %d\n", (unsigned int)pthread_self(), j);
        upper = atoi(tinfor->argv_string);
//      printf("The value of upper in thread %d is %d\n", (unsigned int)pthread_self(), upper);
        sum[j] = 0;
        if (upper < 0) pthread_exit(&upper);

        for (i=upper-9; i<=upper; i++) {
                printf("The value of i in child thread %ld is %d\n", (unsigned long)pthread_self(), i);
//              fflush(stdout);
                sum[j] += i;
                usleep(10);
        };
        printf("The sum from %d to %d in thread %ld is %d\n", upper-9, upper, (unsigned long)pthread_self(), sum[j]);
        pthread_exit(0);
}