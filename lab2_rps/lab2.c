#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CONFIG_FILE "line.cnf"

int number = 0;
sem_t *sems;

typedef struct
{
    int *times;
    int *blanks;
    int sizeBlanks;
} ThreadArgs;

int *getSize(FILE *file)
{
    int *NM = (int *)malloc(2 * sizeof(int));
    if (fscanf(file, "%d %d", &NM[0], &NM[1]) != EOF)
    {
        if (NM[0] > 0 && NM[1] > 0)
        {
            return NM;
        }
        else
        {
            printf("columns and rows must be > 0!\n");
            exit(-1);
        }
    }
    else
    {
        printf("Config file error!\n");
        exit(-1);
    }
    return NM;
}

int **getValues(int *NM, FILE *file)
{
    int **values = (int **)malloc(NM[0] * sizeof(int *));
    for (int i = 0; i < NM[0]; i++)
    {
        values[i] = (int *)malloc(NM[1] * sizeof(int));
        for (int j = 0; j < NM[1]; j++)
        {
            fscanf(file, "%d", &values[i][j]);
        }
    }
    return values;
}

void *bench(void *arg_p)
{
    ThreadArgs *args = (ThreadArgs *)arg_p;
    int *times = args->times;
    int *patterns = args->blanks;
    int sizePatterns = args->sizeBlanks;
    int id = number++;
    int num_of_pattern = 0;
    fprintf(stderr, "Bench %d was initialized\n", id);
    sem_wait(&sems[id]);
    while (num_of_pattern != sizePatterns)
    {
        int blank = patterns[num_of_pattern++];
        fprintf(stderr, "Bench %d processing blank %d...\n", id, blank);
        sleep(times[blank]);
        fprintf(stderr, "Bench %d completed processing blank %d\n", id, blank);
        sem_post(&sems[id + 1]);
        if (id != 0 && num_of_pattern != sizePatterns)
        {
            sem_wait(&sems[id]);
        }
    }
    fprintf(stderr, "Bench %d finished job\n", id);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    FILE *file;
    pthread_t *tids;
    int *NM;      // size of matrix
    int **values; // time values from matrix

    if (!(file = fopen(CONFIG_FILE, "r")))
    {
        perror("Can't open file!");
        exit(-1);
    }
    NM = getSize(file);
    values = getValues(NM, file); // getting matrix of time values
    fclose(file);

    sems = (sem_t *)malloc(NM[0] * sizeof(sem_t));
    tids = (pthread_t *)malloc(NM[0] * sizeof(pthread_t));

    int capacity = 10;
    int size = 0;
    int *blanks = (int *)malloc(capacity * sizeof(int));
    printf("%s\n", "Enter blanks sequence:");
    int blank;
    int flag = 0;
    char c = '\0';
    while (scanf("%d%c", &blank, &c) == 2)
    {
        if (blank < 0 || blank >= NM[1])
        {
            printf("Blank number(%d) out of range!\n", blank);
            flag = -1;
            break;
        }
        else if (c != ' ' && c != '\n')
        {
            printf("Invalid character!\n");
            flag = -1;
            break;
        }
        if (size >= capacity)
        {
            capacity *= 2;
            blanks = (int *)realloc(blanks, capacity * sizeof(int));
        }
        blanks[size++] = blank;
        if (c == '\n')
            break;
    }
    if (size == 0){
        flag = -1;
        printf("Invalid character!\n");
        }
    ThreadArgs thread_args;
    for (int i = 0; flag == 0 && i < NM[0]; i++)
    {
        sem_init(&sems[i], 0, 0);
        thread_args.times = values[i];
        thread_args.blanks = blanks;
        thread_args.sizeBlanks = size;
        if (pthread_create(&tids[i], NULL, bench, (void *)&thread_args))
        {
            perror("Pthread create failure");
        }
        sleep(1);
    }

    // for (int i = 0; flag == 0 && i < size; i++)
    // {
    if (flag == 0)
        sem_post(&sems[0]);
    // }

    for (int i = 0; flag == 0 && i < NM[0]; i++)
    {
        pthread_join(tids[i], NULL);
    }

    for (int i = 0; i < NM[0]; i++)
    {
        free(values[i]);
    }
    free(tids);
    free(NM);
    free(values);
    free(sems);
    free(blanks);
    return 0;
}
