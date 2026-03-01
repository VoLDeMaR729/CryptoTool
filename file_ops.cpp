#include "file_ops.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Чтение файла в память
char* readFile(const char* filename, long& size) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Ошибка: Не удалось открыть файл %s.\n", filename);
        return NULL;
    }

    // Определяем размер файла
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size == 0) {
        fclose(file);
        size = 0;
        return (char*)malloc(0); // Возвращаем пустой указатель
    }

    // Выделяем память
    char* buffer = (char*)malloc(size);
    if (buffer == NULL) {
        fprintf(stderr, "Ошибка памяти.\n");
        fclose(file);
        return NULL;
    }

    // Читаем данные
    if (fread(buffer, 1, size, file) != (size_t)size) {
        fprintf(stderr, "Ошибка чтения.\n");
        free(buffer);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return buffer;
}

// Запись буфера в файл
bool writeFile(const char* filename, const char* buffer, long size) {
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        fprintf(stderr, "Ошибка открытия файла для записи.\n");
        return false;
    }

    if (size > 0 && buffer != NULL) {
        if (fwrite(buffer, 1, size, file) != (size_t)size) {
            fprintf(stderr, "Ошибка записи данных.\n");
            fclose(file);
            return false;
        }
    }

    fclose(file);
    return true;
}

// Вывод файла в консоль
void printFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Ошибка открытия файла.\n");
        return;
    }

    printf("\n--- Начало файла: %s ---\n", filename);
    int c;
    while ((c = fgetc(file)) != EOF) {
        putchar(c);
    }
    printf("\n--- Конец файла: %s ---\n", filename);
    
    fflush(stdout); // Сброс буфера вывода
    fclose(file);
}