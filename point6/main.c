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

    int first[2];
    int second[2];

    if (pipe(first) < 0) {
        printf("ERROR: open pipe\n");
        exit(-1);
    }
    if (pipe(second) < 0) {
        printf("ERROR: open pipe\n");
        exit(-1);
    }
    int process = fork();
    if (process < 0) {
        printf("ERROR: fork child\n");
        exit(-1);
    } else if (process == 0) {
        if (close(first[1]) < 0) {
            printf("ERROR: close writing side of pipe\n");
            exit(-1);
        }
        size = read(first[0], buffer, BUFFER_SIZE);
        buffer[size] = '\0';
        if (size < 0) {
            printf("ERROR: read string from pipe\n");
            exit(-1);
        }
        convert_consonants_to_ascii(buffer);
        if (close(first[0]) < 0) {
            printf("ERROR: close reading side of pipe\n");
            exit(-1);
        }
        size = write(second[1], buffer, BUFFER_SIZE);
        if (size != BUFFER_SIZE) {
            printf("ERROR: write all string to pipe\n");
            exit(-1);
        }
        if (close(second[1]) < 0) {
            printf("ERROR: close writing side of pipe\n");
            exit(-1);
        }
        exit(0);
    } else {
        if (close(first[0]) < 0) {
            printf("ERROR: close reading side of pipe\n");
            exit(-1);
        }
        int input_file;
        if ((input_file = open(input, O_RDONLY, 0666)) < 0) {
            printf("ERROR: open file\n");
            exit(-1);
        }
        size = read(input_file, buffer, BUFFER_SIZE);
        buffer[size] = '\0';
        if (close(input_file) < 0) {
            printf("ERROR: close file\n");
        }
        size = write(first[1], buffer, BUFFER_SIZE);
        if (size != BUFFER_SIZE) {
            printf("ERROR: write all string to pipe\n");
            exit(-1);
        }
        if (close(first[1]) < 0) {
            printf("ERROR: close writing side of pipe\n");
            exit(-1);
        }
    }
    while (wait(NULL) > 0) {
    }
    if (close(second[1]) < 0) {
        printf("ERROR: close writing side of pipe\n");
        exit(-1);
    }
    size = read(second[0], buffer, BUFFER_SIZE);
    if (size < 0) {
        printf("ERROR: read string from pipe\n");
        exit(-1);
    }
    if (close(second[0]) < 0) {
        printf("ERROR: close reading side of pipe\n");
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
    return 0;
}