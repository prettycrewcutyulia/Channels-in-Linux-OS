#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>


#define BUFFER_SIZE 5000


int is_consonant(char c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) && !(c == 'a'
    || c == 'e' || c == 'i' || c == 'o' || c == 'u' || c == 'A' || c == 'E' || c == 'I' || c == 'O'
    || c == 'U');
}

void convert_consonants_to_ascii(char *str) {
    int i;
    char output[BUFFER_SIZE] = "";
    for (i = 0; str[i] != '\0'; i++) {
        if (is_consonant(str[i])) {
            char symbol[10];
            int digit = (int) str[i];
            sprintf(symbol, "%d", digit);
            strcat(output, symbol);
        } else {
            char symbol[10];
            sprintf(symbol, "%c", str[i]);
            strcat(output, symbol);
        }
    }
    memset(str, 0, BUFFER_SIZE);
    strcpy(str, output);
}


bool check(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Необходимо запустить программу с двумя аргументами\n");
        return false;
    }
    return true;
}
const char *pipe_first = "first_pipe.fifo";
const char *pipe_second = "second_pipe.fifo";

int main(int argc, char *argv[]) {
    char buffer[BUFFER_SIZE];
    int size;
    char *input, *output;
    if (!check(argc, argv)) {
        return 0;
    } else {
        input = argv[1];
        output = argv[2];
    }
    mknod(pipe_first, S_IFIFO | 0666, 0);
    mknod(pipe_second, S_IFIFO | 0666, 0);

    int first;
    int second = 0;

    int process = fork();
    if (process < 0) {
        printf("ERROR: fork child\n");
        exit(-1);
    } else if (process == 0) {
        first = open(pipe_first, O_RDONLY);
        if (first < 0) {
            printf("ERROR: open FIFO for reading\n");
            exit(-1);
        }

        size = read(first, buffer, BUFFER_SIZE);

        if (size < 0) {
            printf("ERROR: read string from FIFO\n");
            exit(-1);
        }
        convert_consonants_to_ascii(buffer);

        if (close(first) < 0) {
            printf("ERROR: close FIFO\n");
            exit(-1);
        }

        if ((second = open(pipe_second, O_WRONLY)) < 0) {
            printf("ERROR: open FIFO for writting\n");
            exit(-1);
        }
        size = write(second, buffer, BUFFER_SIZE);
        if (size != BUFFER_SIZE) {
            printf("ERROR: write all string to FIFO\n");
            exit(-1);
        }

        if (close(second) < 0) {
            printf("ERROR: close FIFO\n");
            exit(-1);
        }
        exit(0);
    } else {
        int input_file = open(input, O_RDONLY, 0666);
        if (input_file < 0) {
            printf("ERROR: open file\n");
            exit(-1);
        }
        size = read(input_file, buffer, BUFFER_SIZE);
        buffer[size] = '\0';
        if (close(input_file) < 0) {
            printf("ERROR: close file\n");
        }
        first = open(pipe_first, O_WRONLY);
        if (first < 0) {
            printf("ERROR: open FIFO for writing\n");
            exit(-1);
        }
        size = write(first, buffer, BUFFER_SIZE);

        if (size != BUFFER_SIZE) {
            printf("ERROR: write all string to FIFO\n");
            exit(-1);
        }

        if (close(first) < 0) {
            printf("ERROR: close writing side of FIFO\n");
            exit(-1);
        }
    }
    second = open(pipe_second, O_RDONLY);
    if (second < 0) {
        printf("ERROR: open FIFO for reading\n");
        exit(-1);
    }
    size = read(second, buffer, BUFFER_SIZE);
    if (size < 0) {
        printf("ERROR: read string from FIFO\n");
        exit(-1);
    }
    if (close(second) < 0) {
        printf("ERROR: close FIFO\n");
        exit(-1);
    }
    int output_file = open(output, O_WRONLY | O_CREAT, 0666);
    if (output_file < 0) {
        printf("ERROR: open file\n");
        exit(-1);
    }
    size = write(output_file, buffer, strlen(buffer));
    if (size != strlen(buffer)) {
        printf("ERROR: write all string\n");
        exit(-1);
    }
    if (close(output_file) < 0) {
        printf("ERROR: close file\n");
    }
    unlink(pipe_first);
    unlink(pipe_second);
    return 0;
}