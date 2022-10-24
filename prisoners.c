#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

static unsigned int s;
static pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t drawer_mutex[101];

typedef struct
{
    int *drawer_arr;
    int p_no;
    int win;
}prisoner_t;

static void mutex_lock(pthread_mutex_t *mutex)
{
    int e = pthread_mutex_lock(mutex);
    if (e) {
        fprintf(stderr, "pthread_mutex_lock(): %s\n", strerror(e));
        exit(EXIT_FAILURE);
    }
}

static void mutex_unlock(pthread_mutex_t *mutex)
{
    int e = pthread_mutex_unlock(mutex);
    if (e) {
        fprintf(stderr, "pthread_mutex_unlock(): %s\n", strerror(e));
        exit(EXIT_FAILURE);
    }
}

int* generate_random_array (int *rand_arr, unsigned int seed){
    int max = 100, n, i, initial_arr[101];
    for(i = 1; i <= 100; i++){
        initial_arr[i] = i;
    }
    i=1;
    srand(seed);
    while(max >= 1){
        n = rand() % max + 1;
        rand_arr[i] = initial_arr[n];
        initial_arr[n] = initial_arr[max];
        i++;
        max--;
    }
    return rand_arr;
}

static void *random_global(void *arg){
    prisoner_t *prisoner = (prisoner_t*) arg;
    int found = 0, n, max = 100;
    int index_arr[101];
    for(int i = 1; i <= 100; i++){
        index_arr[i] = i;
    }
    srand(s++);
    mutex_lock(&global_mutex);
    while(found == 0 && max >= 51){
        n = rand() % max + 1;
        if(prisoner->drawer_arr[index_arr[n]] == prisoner->p_no){
            found = 1;
        }
        index_arr[n] = index_arr[max];
        max--;
    }
    mutex_unlock(&global_mutex);
    if(found){
        prisoner->win = 1;
    }
    return EXIT_SUCCESS;
}

static void *random_drawer(void *arg){
    prisoner_t *prisoner = (prisoner_t*) arg;
    int found = 0, n, max = 100, index_arr[101];
    for(int i = 1; i <= 100; i++){
        index_arr[i] = i;
    }
    srand(s++);
    while(found == 0 && max >= 51){
        n = rand() % max + 1;
        mutex_lock(&drawer_mutex[index_arr[n]]); 
        if(prisoner->drawer_arr[index_arr[n]] == prisoner->p_no){
            found = 1;
        }
        mutex_unlock(&drawer_mutex[index_arr[n]]);
        index_arr[n] = index_arr[max];
        max--;
    }
    if(found){
        prisoner->win = 1;
    }
    return EXIT_SUCCESS;
}

static void *strategy_global(void *arg){
    prisoner_t *prisoner = (prisoner_t*) arg;
    int tries, found, search_index;
    found = 0;
    tries = 1;
    search_index = prisoner->p_no;
    mutex_lock(&global_mutex);
    while(found == 0 && tries <= 50){
        if(prisoner->drawer_arr[search_index] == prisoner->p_no){
            found = 1;
        }
        else{
            search_index = prisoner->drawer_arr[search_index];
            tries++;
        }
    }
    mutex_unlock(&global_mutex);
    if(found){
        prisoner->win = 1;
    }
    return EXIT_SUCCESS;
}

static void *strategy_drawer(void *arg){
    prisoner_t *prisoner = (prisoner_t*) arg;
    int tries, found, search_index;
    found = 0;
    tries = 1;
    search_index = prisoner->p_no;
    while(found == 0 && tries <= 50){
        mutex_lock(&drawer_mutex[search_index]); 
        if(prisoner->drawer_arr[search_index] == prisoner->p_no){
            found = 1;
        }
        mutex_unlock(&drawer_mutex[search_index]); 
        search_index = prisoner->drawer_arr[search_index];
        tries++;
        
    }
    if(found){
        prisoner->win = 1;
    }
    return EXIT_SUCCESS;
}



void run_threads(int t, void*(*proc)(void *), unsigned int *strategy_wins){
    pthread_t tids[t];
    int drawer_arr[101], win = 1;
    generate_random_array(drawer_arr, s++);

    prisoner_t prisoners[101];
    for(int i = 1; i <=100; i++){
        prisoners[i].win = 0;   
        prisoners[i].p_no = i;     
        prisoners[i].drawer_arr = drawer_arr;
    }

    for(int i = 0; i < t; i++){
        int e = pthread_create(&tids[i], NULL, proc, &prisoners[i+1]);
        if(e){
            fprintf(stderr, "pthread_create(): %s\n", strerror(e));
            exit(EXIT_FAILURE);
        }
    }
    for(int i = 0; i < t; i++){
        int e = pthread_join(tids[i], NULL);
        if(e){
            fprintf(stderr, "pthread_create(): %s\n", strerror(e));
            exit(EXIT_FAILURE);
        }
    }
    for(int i = 1; i <=100 && win == 1; i++){
        if(!prisoners[i].win){
            win = 0;
        }
    }

    if(win)
        *strategy_wins += 1;
}

static double
timeit(int n, void *(*proc)(void *), unsigned int *strategy_wins)
{
    clock_t t1, t2;
    t1 = clock();
    run_threads(n, proc, strategy_wins);
    t2 = clock();
    return ((double)t2 - (double)t1) / CLOCKS_PER_SEC * 1000;
}

double execute_strategy(int games, void *(*proc)(void *), unsigned int *strategy_wins){
    double time = 0; 
    for(int i = 0; i < games; i++){
        time += timeit(100, proc, strategy_wins);
    }
    return time;
}

int main(int argc, char *argv[])
{
    int opt, status = EXIT_SUCCESS;
    unsigned int games = 100, seed = 10000, random_global_wins = 0, random_drawer_wins = 0, strategy_global_wins = 0, strategy_drawer_wins = 0;

    while((opt = getopt(argc, argv, "n:s:")) != -1){
        switch (opt)
        {
        case 'n':
                games = atoi(optarg);
            break;
        case 's':
            seed = atoi(optarg);
            break;
        default:
            break;
        }
    }

    s = seed;

    for(int i = 1; i <= 100; i++){
        int e = pthread_mutex_init(&drawer_mutex[i], NULL);
        if(e){
            fprintf(stderr, "pthread_mutex_init(): %s\n", strerror(e));
            exit(EXIT_FAILURE);
        }
    }

    double strategy_global_time = execute_strategy(games, strategy_global, &strategy_global_wins); 
    double strategy_drawer_time = execute_strategy(games, strategy_drawer, &strategy_drawer_wins); 
    double random_global_time = execute_strategy(games, random_global, &random_global_wins); 
    double random_drawer_time = execute_strategy(games, random_drawer, &random_drawer_wins); 

    printf("method random_global\t\t  %d/%d wins =  %.2f%% \t %lf ms\n", random_global_wins, games,(double)random_global_wins/games*100, random_global_time);
    printf("method random_drawer\t\t  %d/%d wins =  %.2f%% \t %lf ms\n", random_drawer_wins, games,(double)random_drawer_wins/games*100, random_drawer_time);
    printf("method strategy_global\t\t %d/%d wins = %.2f%% \t %lf ms\n", strategy_global_wins, games, (double)strategy_global_wins/games*100, strategy_global_time);
    printf("method strategy_drawer\t\t %d/%d wins = %.2f%% \t %lf ms\n", strategy_drawer_wins, games, (double)strategy_drawer_wins/games*100, strategy_drawer_time);

    if(fflush(stdout) || ferror(stdout)){
        perror("write failed\n");
        status = EXIT_FAILURE;
    }

    return status;
}