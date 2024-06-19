#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    FILE *file;
    srand(time(0));
    int count_files = rand()%9+1;
    char **files = (char **) malloc(count_files * sizeof(char *));
    for (int i = 0  ; i < count_files; i++){
        int count_letters = rand()%19+1;
        char *string = (char *)malloc((count_letters + 4)* sizeof(char));
        for (int j = 0  ; j < count_letters ; j++){
            int letter = rand()%25+65;
            string[j] = letter;
        }
        string[count_letters] = '.';
        string[count_letters + 1] = 't';
        string[count_letters + 2] = 'x';
        string[count_letters + 3] = 't';
        string[count_letters+4] = '\0';
        file = fopen(string, "w");
        if (file == NULL) {
            perror("Error opening file");
            exit(1);
        }
        fclose(file);
        files[i] = string;
    }
    char *command = (char *)malloc(10000 * sizeof(char));

    // sprintf(command, "ar -cvr libtest.a \"%s\"", files[0]);
    // for (int i = 1; i < count_files; i++){
    //     sprintf(command, "%s \"%s\"", command, files[i]);
    // }
    sprintf(command, "ar -cr libtest.a *.txt");
    system(command);
    for (int i = 0  ; i < count_files ; i++){
        free(files[i]);
    }
    usleep(1000);
    free(files);
    free(command);
    return 0;
}