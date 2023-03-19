// Этот код на языке C++ описывает программу, которая считывает текстовый файл,
// затем заменяет все согласные буквы в тексте на ASCII-коды и записывает
// полученный текст в другой файл, используя процессы и каналы.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

#define BUFFER_SIZE 5000

// Функция, которая определяет, является ли символ согласной буквой.
int is_consonant(char c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) && !(c == 'a'
    || c == 'e' || c == 'i' || c == 'o' || c == 'u' || c == 'A' || c == 'E' || c == 'I' || c == 'O'
    || c == 'U');
}
// Функция, которая заменяет все согласные буквы в строке на ASCII-коды.
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
    // Очистка исходной строки и замена её на обработанную строку.
    memset(str, 0, BUFFER_SIZE);
    strcpy(str, output);
}

// Функция, которая проверяет правильность использования программы.
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
    // Проверка правильности использования программы.
    if (!check(argc, argv)) { // проверяем аргументы командной строки
        return 0; // если аргументы некорректны, завершаем программу
    } else {
        input = argv[1]; // присваиваем имя входного файла
        output = argv[2]; // присваиваем имя выходного файла
    }

    int first[2]; // создаем первый канал
    int second[2]; // создаем второй канал

    if (pipe(first) < 0) { // создаем первый канал и проверяем успешность операции
        printf("ERROR: open pipe\n"); // выводим сообщение об ошибке
        exit(-1); // завершаем программу с ошибкой
    }

    if (pipe(second) < 0) { // создаем второй канал и проверяем успешность операции
        printf("ERROR: open pipe\n"); // выводим сообщение об ошибке
        exit(-1); // завершаем программу с ошибкой
    }

    int process1 = fork(); // создаем первый дочерний процесс

    if (process1 < 0) { // проверяем успешность операции создания дочернего процесса
        printf("ERROR: fork child\n"); // выводим сообщение об ошибке
        exit(-1); // завершаем программу с ошибкой
    } else if (process1 == 0) { // если это дочерний процесс

        int process2 = fork(); // создаем второй дочерний процесс

        if (process2 < 0) { // проверяем успешность операции создания дочернего процесса
            printf("ERROR: fork child\n"); // выводим сообщение об ошибке
            exit(-1); // завершаем программу с ошибкой
        } else if (process2 == 0) { // если это второй дочерний процесс

            if (close(second[1]) < 0) { // закрываем записывающую сторону второго канала
                printf("ERROR: close writing side of pipe\n"); // выводим сообщение об ошибке
                exit(-1); // завершаем программу с ошибкой
            }

            size = read(second[0], buffer, BUFFER_SIZE); // читаем данные из второго канала
            if (size < 0) { // проверяем успешность операции чтения
                printf("ERROR: read string from pipe\n"); // выводим сообщение об ошибке
                exit(-1); // завершаем программу с ошибкой
            }
            if (close(second[0]) < 0) { // закрываем читающую сторону второго канала
                printf("ERROR: close reading side of pipe\n"); // выводим сообщение об ошибке
                exit(-1); // завершаем программу с ошибкой
            }
            int output_file = open(output, O_WRONLY | O_CREAT, 0666); // открываем файл для записи результата
            if (output_file < 0) {
                printf("ERROR: open file\n"); // выводим сообщение об ошибке, если файл не удалось открыть
                exit(-1);
            }
            size = write(output_file, buffer, strlen(buffer)); // записываем результат в файл
            if (size != strlen(buffer)) {
                printf("ERROR: write all string\n"); // выводим сообщение об ошибке, если не удалось записать все данные
                exit(-1);
            }
            if (close(output_file) < 0) {
                printf("ERROR: close file\n"); // выводим сообщение об ошибке, если не удалось закрыть файл
            }
        } else { // процесс-родитель
            if (close(first[1]) < 0) {
                printf("ERROR: close writing side of pipe\n"); // выводим сообщение об ошибке, если не удалось закрыть записывающую сторону первого канала
                exit(-1);
            }
            size = read(first[0], buffer, BUFFER_SIZE); // читаем данные из первого канала
            if (size < 0) {
                printf("ERROR: read string from pipe\n"); // выводим сообщение об ошибке, если не удалось прочитать данные
                exit(-1);
            }
            convert_consonants_to_ascii(buffer); // преобразуем согласные символы в ASCII-коды
            if (close(first[0]) < 0) {
                printf("ERROR: close reading side of pipe\n"); // выводим сообщение об ошибке, если не удалось закрыть читающую сторону первого канала
                exit(-1);
            }
            size = write(second[1], buffer, BUFFER_SIZE); // записываем результат второго процесса во второй канал
            if (size != BUFFER_SIZE) {
                printf("ERROR: write all string to pipe\n (size = %d)", size); // выводим сообщение об ошибке, если не удалось записать все данные
                exit(-1);
            }
            if (close(second[1]) < 0) {
                printf("ERROR: close writing side of pipe\n"); // выводим сообщение об ошибке, если не удалось закрыть записывающую сторону второго канала
                exit(-1);
            }
        }
    } else { // процесс-родитель
        if (close(first[0]) < 0) {
            printf("ERROR: close reading side of pipe\n"); // выводим сообщение об ошибке, если не удалось закрыть читающую сторону первого канала
            exit(-1);
        }
        int input_file = open(input, O_RDONLY, 0666); // открываем файл для чтения
        if (input_file < 0) {
            printf("ERROR: open file\n"); // выводим сообщение об ошибке, если файл не удалось открыть
            exit(-1);
        }
        size = read(input_file, buffer, BUFFER_SIZE - 1); // читаем данные из файла
        buffer[size] = '\0'; // добавляем нулевой символ в конец строки
        if (close(input_file) < 0) {
            printf("ERROR: close file\n"); // выводим сообщение об ошибке, если не удалось закрыть файл
        }
        size = write(first[1], buffer, BUFFER_SIZE); // записываем данные в первый канал
        if (size != BUFFER_SIZE) { // проверяем успешность записи
            printf("ERROR: write all string to pipe\n (size = %d)", size); // выводим сообщение об ошибке, если не все данные удалось записать
            exit(-1);
        }
        if (close(first[1]) < 0) { // закрываем записывающую сторону первого канала
            printf("ERROR: close writing side of pipe\n"); // выводим сообщение об ошибке, если не удалось закрыть записывающую сторону первого канала
            exit(-1);
        }
    }
    return 0;
}