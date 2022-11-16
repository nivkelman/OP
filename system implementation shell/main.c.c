#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#define MaxLengh 512

char** Matrix (int , char** ,  char* , char*  , int , int , int);
void sigHand();
int sig = 0;
int status;

int main() {
    char str[MaxLengh]; // input array
    char strH[MaxLengh]; // history array
    char strC[MaxLengh]; // "done" array
    char strHistory[MaxLengh];
    char arrr[MaxLengh];
    char **arr;          // pointers array
    char **arr1;
    char **arr2;
    char **arr3;
    char **arr4;
    char **arr5;
    char word1[MaxLengh];
    char word2[MaxLengh];
    char word3[MaxLengh];
    char word[MaxLengh];
    char ampersand[MaxLengh];
    char noHup[MaxLengh];
    char cwd[MaxLengh];
    int numOfCommands = 0;
    int numOfWordCommands = 0;
    int numOfPipe = 0;
    int n = 0;
    int p;
    int pipe1;
    int pipe2;
    int numAfter = 0;
    int countCommands = 0;
    int scan;
    FILE *f;

    while (1) // infinite loop
    {
        if (getcwd(cwd, sizeof(cwd)) != NULL || n == 1)
        {
            printf("%s>", cwd);
        }
        else
        {
            perror("getcwd() error");
            exit(1);
        }

        int wordsCount = 0;
        int charsCount = 0;
        int loopNum = 1;
        int index = 0;
        int indexMatrix = 0;
        int count = 1;

        int checkExevp = 1;
        int a = 0;
        int b = 0;
        int c = 0;
        int d = 0;
        int i = 0;
        int k = 0;
        int l = 0;
        int q = 0;
        int j;
        n = 0;

        // reset all the arrays for the next iteration
        strcpy(word, "NULL");
        strcpy(word1, "NULL");
        strcpy(word2, "NULL");
        strcpy(word3, "NULL");
        strcpy(ampersand, "NULL");
        strcpy(noHup, "NULL");

        fgets(str, MaxLengh, stdin); // get input from the user

        int size = (int) strlen(str);
        str[size ] = '\0';

        if (str[0] == ' ' || str[size - 2] == ' ')  //check space before or after the String
        {
            printf(" invalid command\n");
            continue;
        }

        for (j = i; str[j] != '\0'; j++) // running on the input and shows how chars and words he has
        {
            if (str[j] != '\n' && str[j] != ' ') {
                wordsCount++;

                while (str[j] != ' ' && str[j] != '\n') {
                    charsCount++;

                    if (str[i] == 'c') //copy str to strC
                    {
                        strC[k] = str[j];
                        k++;

                    } else if (str[i] == 'h')//copy str to strH
                    {
                        strH[l] = str[j];
                        l++;
                    }
                    j++;
                }
            }
        }

        if (str[size - 2] == '&') // check if there is a command with &
        {
            sig = 1;

            for (int y = 0; str[y] != '&'; y++) {
                ampersand[a] = str[y];
                a++;
            }

            ampersand[a] = '\n';
            ampersand[a+1] = '\0';

            arr4 = Matrix(wordsCount, arr, ampersand, word, count, index, indexMatrix);
            arr4[wordsCount - 1] = NULL;
        }


        if (str[0] == 'n' && str[1] == 'o' && str[2] == 'h' && str[3] == 'u' && str[4] == 'p' &&str[5] == ' ') // check if the first word is nohup
        {
            n = 1;

            for (int m = 6; str[m] != '\n'; m++) {
                noHup[d] = str[m];
                d++;
            }
            noHup[d] = '\n';
            noHup[d+1] = '\0';

            arr5 = Matrix(wordsCount, arr, noHup, word, count, index, indexMatrix);
            arr5[wordsCount] = NULL;
        }

        if (strcmp(str, "cd\n") == 0) {
            printf("command not support (Yet) \n");
            continue;
        }

        if (strcmp(str, "done\n") == 0) {
            numOfCommands++;
            printf("Num of command: %d\n", numOfCommands);
            printf("Total number of words in all commands: %d", numOfWordCommands);
            numOfCommands = 0;
            return 1;
        }

        numOfPipe = 0;

        for (p = 0; p < strlen(str); p++)  // check if the input have pipes and save each word in array
        {
            if (str[p] == '|' && numOfPipe == 0) {
                numOfPipe = numOfPipe + 1;
                pipe1 = p;
                for (q = 0; q < pipe1; q++) {
                    word1[q] = str[q];
                }
                word1[q] = '\n';
                word1[q+1] = '\0';
                for (int q = pipe1 + 1; q < strlen(str) && str[q] != '|'; q++) {
                    word2[b] = str[q];
                    b++;
                }
                word2[b] = '\n';
                word2[b+1] = '\0';
                continue;
            }
            if (str[p] == '|' && numOfPipe == 1) {
                numOfPipe = numOfPipe + 1;
                pipe2 = p;
                for (int q = pipe2 + 1; q < strlen(str) + 1; q++) {
                    word3[c] = str[q];
                    c++;
                }
                word3[c] = '\n';
                word3[c+1] = '\0';
            }
        }


        if (strcmp(strH, "history") == 0) // check if the word is "history"
        {
            strcpy(strH, "NULL"); // array reset

            f = fopen("file.txt", "r"); //open the file and read
            if (f == NULL) {
                printf("Error cant open the file\n");
                exit(1);
            }
            while (fgets(strHistory, MaxLengh, f) != NULL) // copy all the history from the file
            {
                printf("%d  ", loopNum); //number of iteration
                loopNum++;
                printf("%s", strHistory);
            }
            fclose(f); // close file
        }

        f = fopen("file.txt", "a"); // open the file and write into

        if (f == NULL) {
            printf("Error cant open the file\n");
            exit(1);
        }

        if (wordsCount > 0) //copy input to the file
        {
            fputs(str, f);
        }

        fclose(f); // close file


//////////////////////////////////////////////////////////////////////////////////////////////////////

        if (numOfPipe == 1) {
            arr1 = Matrix(wordsCount, arr, word1, word, count, index, indexMatrix);
            arr1[wordsCount] = NULL;
            arr2 = Matrix(wordsCount, arr, word2, word, count, index, indexMatrix);
            arr2[wordsCount] = NULL;

            int pipe_fd[2];

            if (pipe(pipe_fd) == -1) {
                perror("cannot open pipe");
                close(pipe_fd[0]);
                close(pipe_fd[1]);
                exit(1);
            }

            pid_t x = fork();

            if (x == -1) {
                perror("fork failed");
                exit(1);
            }

            if (x == 0) // first fork
            {
                close(STDOUT_FILENO); // closing stdout
                dup(pipe_fd[1]); // replacing stdout with pipe write
                close(pipe_fd[0]);// closing pipe read
                close(pipe_fd[1]);

                execvp(arr1[0], arr1);
                perror("execvp failed");

                exit(1);
            }
            for (int w = 0; w < wordsCount; w++) {
                free(arr1[w]);
            }
            free(arr1);

            pid_t x1 = fork();

            if (x1 == -1) {
                perror("fork failed");
                exit(1);
            }

            if (x1 == 0)// creating 2 child
            {
                close(STDIN_FILENO); // closing stdin
                dup(pipe_fd[0]); // replacing stdin with pipe read
                close(pipe_fd[1]);// closing pipe write
                close(pipe_fd[0]);

                execvp(arr2[0], arr2);
                perror("execvp failed");

                exit(1);
            }
            for (int w = 0; w < wordsCount; w++) {
                free(arr2[w]);
            }
            free(arr2);
            close(pipe_fd[0]);
            close(pipe_fd[1]);
            wait(0);
            wait(0);
            continue;
        }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


        if (numOfPipe == 2) {

            arr1 = Matrix(wordsCount, arr, word1, word, count, index, indexMatrix);
            arr1[wordsCount] = NULL;
            arr2 = Matrix(wordsCount, arr, word2, word, count, index, indexMatrix);
            arr2[wordsCount] = NULL;
            arr3 = Matrix(wordsCount, arr, word3, word, count, index, indexMatrix);
            arr3[wordsCount] = NULL;

            int pipe_fd[2];

            if (pipe(pipe_fd) == -1) {
                perror("cannot open pipe");
                close(pipe_fd[0]);
                close(pipe_fd[1]);
                exit(1);
            }

            int pipe_fd1[2];

            if (pipe(pipe_fd1) == -1) {
                perror("cannot open pipe");
                exit(1);
            }

            pid_t x = fork();

            if (x == -1) // first fork
            {
                perror("fork failed");
                exit(1);
            }

            if (x == 0) {
                close(STDOUT_FILENO); // closing stdout
                dup(pipe_fd[1]); // replacing stdout with pipe write
                close(pipe_fd[0]);// closing pipe read
                close(pipe_fd[1]);
                close(pipe_fd1[0]);
                close(pipe_fd1[1]);

                execvp(arr1[0], arr1);
                exit(1);
            }
            for (int w = 0; arr1[w]!=NULL; w++)
            {
                free(arr1[w]);
            }
            free(arr1);

            pid_t x1 = fork(); // second fork

            if (x1 == -1) {
                perror("fork failed");
                exit(1);
            }

            if (x1 == 0)// creating 2 child
            {
                close(STDIN_FILENO); // closing stdin
                dup(pipe_fd[0]); // replacing stdin with pipe read
                close(STDOUT_FILENO); // closing stdin
                dup(pipe_fd1[1]); // replacing stdin with pipe read
                close(pipe_fd[1]);// closing pipe write
                close(pipe_fd[0]);
                close(pipe_fd1[1]);
                close(pipe_fd1[0]);

                execvp(arr2[0], arr2);
                exit(1);
            }

            for (int w = 0; arr2[w]!=NULL; w++) {
                free(arr2[w]);
            }
            free(arr2);

            pid_t x2 = fork(); // third fork

            if (x2 == 0) {
                close(STDIN_FILENO); // closing stdin
                dup(pipe_fd1[0]); // replacing stdin with pipe read
                close(pipe_fd[1]);// closing pipe write
                close(pipe_fd[0]);
                close(pipe_fd1[1]);
                close(pipe_fd1[0]);

                execvp(arr3[0], arr3);
                perror("execvp failed");

                exit(1);
            }
            for (int w = 0; arr3[w] != NULL ; w++)
            {
                free(arr3[w]);
            }
            free(arr3);

            close(pipe_fd[0]);
            close(pipe_fd[1]);
            close(pipe_fd1[1]);// closing pipe write
            close(pipe_fd1[0]);
            wait(0);
            wait(0);
            wait(0);

        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////

        if (sig == 1) // if the command with & in the end
        {
            signal(SIGCHLD, sigHand);

            pid_t x = fork();

            if (x == -1) {
                perror("fork failed");
                exit(1);
            }
            if (x == 0)  // child process
            {
                if (execvp(arr4[0], arr4) == -1) {
                    checkExevp = 0;
                    perror("execvp() fail ");
                }

                exit(1);
            }
            for (int o = 0; o < wordsCount; o++) {
                free(arr4[o]);
            }

            free(arr4);
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////

        if (n == 1) // implementaion nohup
        {

            int hup = open("nohup.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

            pid_t x = fork();

            if (x == 0)  // child process
            {
                signal(SIGHUP, SIG_IGN);

                int value = dup2(hup, STDOUT_FILENO);
                if (value == -1) {
                    fprintf(stderr, "dup2 failed\n");
                    exit(1);
                }

                if (execvp(arr5[0], arr5)) {
                    checkExevp = 0;
                    perror("execvp() fail ");
                }
            }
            if (x == -1) {
                perror("fork failed");
                exit(1);
            }


            for (int z = 0; z < wordsCount; z++) {
                free(arr5[z]);
            }
            free(arr5);

            wait(NULL);
            continue;
        }



            /////////////////////////////////////////////////////////////////////////////////////////////////////////

            arr = Matrix(wordsCount, arr, str, word, count, index, indexMatrix);

            if (strcmp(str, "done\n") != 0 && strcmp(str, "cd\n") != 0 && strcmp(str, "history\n") != 0 &&numOfPipe == 0 && sig == 0 && n == 0) // if the command is pure
            {
                pid_t x = fork();

                if (x == -1) {
                    perror("fork failed");
                    exit(1);
                }
                if (x == 0)  // child process
                {
                    if (execvp(arr[0], arr) == -1) {
                        checkExevp = 0;
                        perror("execvp() fail ");
                    }


                    exit(1);
                }
                // parent process

                wait(NULL);

                for (int z = 0; z < wordsCount ; z++) {
                    free(arr[z]);
                }
                free(arr);
                wait(&status);
                if (WEXITSTATUS(status) == 1) {
                    checkExevp = 0;
                }
                numOfCommands++;
                numOfWordCommands += wordsCount;
            }
        }
    }

    //methods

    char **Matrix(int wordsCount, char **arr, char *str, char *word, int count, int index, int indexMatrix) {
        arr = (char **) malloc(sizeof(char *) * ((wordsCount) + 1)); // create array in size of words count of the String

        int i = 0;
        int j = 0;
        if (arr == NULL) {
            printf("error");
            exit(1);
        }
        for (j = 0; str[j] != '\0' && str[j] != '\n' ; j++) {
            if (str[j] != ' ' && str[j] != '\n' && str[j] != '\0' )
            {
                while (str[j] != ' ' && str[j] != '\n' && str[j] != '\0') // analyzes each word individually
                {
                    word[index] = str[j]; // fill word array
                    index++;
                    count++;
                    j++;
                }

                word[index] = '\0';

                arr[indexMatrix] = (char *) malloc(sizeof(char) * ((count) +1)); // create array in size of words count of the String and the size of each word in one cell

                if (arr[indexMatrix] == NULL) {
                    printf("Error");
                    exit(1);
                }

                strcpy(arr[indexMatrix], word); // copy word array to arr
                indexMatrix++;
                index = 0; // reset the index for next iteration
                count = 0; // reset the count for next iteration
                strcpy(word, "NULL");
            }
        }
            arr[indexMatrix] = NULL; // insert NULL to the end of the array , arr ready to use

            return arr;

    }
    void sigHand()
    {
        if (sig == 1)
        {
            sig = 0;
            waitpid(-1, &status, WNOHANG);
        }
    }
