#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define CONFIG_FILE "line.cnf"
int number = 0;
sem_t *sems;
int **fds;

int *getSize(FILE *file) {
  int *NM;
  NM = (int *)malloc(2 * sizeof(int));
  if (fscanf(file, "%d %d", &NM[0], &NM[1]) != EOF) {
    if (NM[0] > 0 && NM[1] > 0) {
      return NM;
    } else {
      printf("columns and rows must be > 0!\n");
      exit(-1);
    }
  } else {
    printf("Config file error!\n");
    exit(-1);
  }
  return NM;
}

int **getValues(int *NM, FILE *file) {
  int **values;
  int i, j;
  values = (int **)malloc(NM[0] * sizeof(int *));
  for (i = 0; i < NM[0]; i++) {
    values[i] = (int *)malloc(NM[1] * sizeof(int));
    for (j = 0; j < NM[1]; j++) {
      fscanf(file, "%d", &values[i][j]);
    }
  }
  return values;
}
void *bench(void *arg_p) {
  int *times = (int *)arg_p;
  int blank = 0;
  int id = number++;
  fprintf(stderr, "Bench %d was initialized \n", id);
  sem_wait(&sems[id]);
  read(fds[id][0], &blank, sizeof(int));
  while (blank != -1) {
    fprintf(stderr, "Bench %d processing blank %d...\n", id, blank);
    sleep(times[blank]);
    fprintf(stderr, "Bench %d completed processing blank %d\n", id, blank);
    if (id < number - 1) {
      write(fds[id + 1][1], &blank, sizeof(int));
      sem_post(&sems[id + 1]);
    }
    sem_wait(&sems[id]);
    read(fds[id][0], &blank, sizeof(int));
  }
  if (id < number - 1) {
    write(fds[id + 1][1], &blank, sizeof(int));
    sem_post(&sems[id + 1]);
  }
  fprintf(stderr, "Bench %d finished job\n", id);
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  FILE *file = 0;
  pthread_attr_t pattr;
  pthread_t *tids;
  int *NM;      // size of matrix
  int **values; // time values from matrix
  int blank = 0;
  if (!(file = fopen(CONFIG_FILE, "r"))) {
    perror("Can't open file!");
    exit(-1);
  }
  NM = getSize(file);
  values = getValues(NM, file); // getting matrix of time values
  fclose(file);
  sems = (sem_t *)malloc(NM[0] * sizeof(sem_t));
  fds = (int **)malloc(NM[0] * sizeof(int *));
  for (int i = 0; i < NM[0]; i++) {
    fds[i] = (int *)malloc(2 * sizeof(int));
    pipe(fds[i]);
  }
  tids = (pthread_t *)malloc(NM[0] * sizeof(pthread_t));
  pthread_attr_init(&pattr);
  pthread_attr_setscope(&pattr, PTHREAD_SCOPE_SYSTEM);
  pthread_attr_setdetachstate(&pattr, PTHREAD_CREATE_JOINABLE);
  for (int i = 0; i < NM[0]; i++) {
    sem_init(&sems[i], 1, 0);
    if (pthread_create(&tids[i], &pattr, bench, (void *)values[i])) {
      perror("Pthread create failure");
    }
    sleep(1);
  }
  char sym;
  printf("%s\n", "Enter blanks sequence:");
  while (blank != -1 && scanf("%d%с", &blank, &sym) != EOF) {
    if (blank < 0 || blank >= NM[1]) {
      printf("Blank number(%d) out of range!\n", blank);
      blank = -1;
    } else {
      write(fds[0][1], &blank, sizeof(int));
      sem_post(&sems[0]);
    }
    if (sym == '\n') {
      blank = -1;
    }
  }
  blank = -1;
  write(fds[0][1], &blank, sizeof(int));
  sem_post(&sems[0]);
  pthread_join(tids[NM[0] - 1], NULL);
  for (int i = 0; i < NM[0]; i++) {
    free(values[i]);
  }
  free(tids);
  free(NM);
  free(values);
  return 0;
}