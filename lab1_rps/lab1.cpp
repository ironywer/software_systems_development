#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int fdp[2];

  if (argc < 3) {
    fprintf(stderr, "Usage: %s <archive> <prototype>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int p_result = pipe(fdp);
  if (p_result < 0){
    perror("pipe");
    exit(EXIT_FAILURE);
  }
  
  int result_fork = fork();
  if (result_fork < 0){
    perror("fork");
    exit(EXIT_FAILURE);
  }
  else if (result_fork == 0) {

    if (dup2(fdp[1], STDOUT_FILENO) < 0){
      perror("dup2");
      exit(EXIT_FAILURE);
    }
    
    close(fdp[0]);
    close(fdp[1]);

    execl("/usr/bin/ar", "ar", "t", argv[1], NULL);

    perror("execl");
    exit(EXIT_FAILURE);
  } else {
    int child_pid = wait(0);
    if (child_pid == -1) {
      fprintf(stderr, "Error: wait() failed\n");
      exit(EXIT_FAILURE);
    }

    if (dup2(fdp[0], STDIN_FILENO) < 0){
      perror("dup2");
      exit(EXIT_FAILURE);
    }

    close(fdp[0]);
    close(fdp[1]);

    execl("/bin/grep", "grep", argv[2], NULL);

    perror("execl");
    exit(EXIT_FAILURE);
  }

  return 0;
}
