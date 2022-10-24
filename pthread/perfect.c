#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

struct Arg{
    int min;
    int max;
};

static int
is_perfect(u_int64_t num)
{
    u_int64_t i, sum;
    if (num < 2)
    {
        return 0;
    }
    for (i = 2, sum = 1; i * i <= num; i++)
    {
        if (num % i == 0)
        {
            sum += (i * i == num) ? i : i + num / i;
        }
    }
    return (sum == num);
}

static void *work(void *arg){
    struct Arg *p = (struct Arg*)arg;
    for(int i = p->min; i < p->max; i++){
        if(is_perfect(i)){
            printf("%d\n", i);
        }
    }
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]){
    int opt, min = 1, max = 10000, threads = 1, v = 0, i, status = EXIT_SUCCESS;

    while((opt = getopt(argc, argv, "s:e:t:v")) != -1){
        switch (opt)
        {
        case 's':
            min = atoi(optarg);
            break;
        case 'e':
            max = atoi(optarg); 
            break;
        case 't':
            threads = atoi(optarg);
            break;
        case 'v':
            v = 1;
            break;        
        default:
            break;
        }
    }

    
    struct Arg *arr = (struct Arg*)malloc(threads * sizeof(struct Arg));
    pthread_t *tids = (pthread_t*)malloc(threads * sizeof(pthread_t));

    if(arr == NULL || tids == NULL){
        perror("malloc failed\n");
        return EXIT_FAILURE;
    }

    for(i = 0; i < threads - 1; i++){ 
        arr[i].min = min + (max-min + 1)/threads*i;
        arr[i].max = min + (max-min + 1)/threads*(i+1) - 1;
        if(v){
            fprintf(stderr, "perfect: t%d searching: [%d %d]\n", i, arr[i].min, arr[i].max);
        }
        pthread_create(&tids[i], NULL, work, &arr[i]);
    }

    arr[i].min = min + (max-min + 1)/threads*i;
    arr[i].max = max;
    if(v){
        fprintf(stderr, "perfect: t%d searching: [%d %d]\n", i, arr[i].min, arr[i].max);
    }
    if(pthread_create(&tids[i], NULL, work, &arr[i]) != 0 ){
        perror("pthread_create failed\n");
        status = EXIT_FAILURE;
    }

    for(int i = 0; i < threads; i++){ 
        if(tids[i]){
            if(pthread_join(tids[i], NULL) != 0){
                perror("pthread_join failed\n");
                status = EXIT_FAILURE;
            }
            if(v){
                fprintf(stderr, "perfect: t%d finishing\n", i);
            }
        }
    }

    if(fflush(stdout) || ferror(stdout)){
        perror("write failed\n");
        status = EXIT_FAILURE;
    }

    free(tids);
    free(arr);
    
    return status;
}