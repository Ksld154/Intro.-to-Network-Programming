#include<cstdio>
#include<cstdlib>
#include<time.h>

int main(int argc, char const *argv[])
{
    srand(time(0));
    
    for(int i = 0; i < 10; i++)
    {
        /* code */
        int ans = rand() % 2;
        printf("%d ", ans);
    }
    printf("\n");
    return 0;
}
