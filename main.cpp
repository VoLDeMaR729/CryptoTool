#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_ops.h"
#include "ciphers.h"

#define MAX_INPUT_LENGTH 256

// Перечисление для безопасной работы с меню
enum class MenuOption {
    EXIT = 0,
    PRINT_FILE = 1,
    ENCRYPT = 2,
    DECRYPT = 3,
    WRITE_TEXT = 4
};

// Безопасное чтение строки с очисткой символа переноса
void readLine(char* buffer, size_t size) {
    if (fgets(buffer, (int)size, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
    }
}

// Вывод приглашения и чтение ввода
void readStringPrompt(const char* prompt, char* buffer, size_t size) {
    printf("%s", prompt);
    fflush(stdout);
    readLine(buffer, size);
}

// Главная процедура обработки шифрования/дешифрования
void handleCipherOperation(bool encrypt) {
    char inputFile[MAX_INPUT_LENGTH];
    char outputFile[MAX_INPUT_LENGTH];
    char choiceBuf[16];
    int cipherChoice = 0;
    
    // Сбор входных данных
    readStringPrompt("Введите имя входного файла: ", inputFile, MAX_INPUT_LENGTH);
    readStringPrompt("Введите имя выходного файла: ", outputFile, MAX_INPUT_LENGTH);

    // Блок Try-Catch для безопасной обработки ошибок
    try {
        long size = 0;
        char* data = readFile(inputFile, size);
        if (data == NULL) {
            throw "Не удалось открыть или прочитать входной файл!";
        }
        
        printf("\nВыберите шифр:\n");
        printf(" 1. Атбаш\n");
        printf(" 2. Двойная табличная перестановка\n");
        printf(" 3. TEA (Tiny Encryption Algorithm)\n");
        printf("Ваш выбор: ");
        fflush(stdout);
        
        readLine(choiceBuf, sizeof(choiceBuf));
        cipherChoice = atoi(choiceBuf);

        char* outputData = NULL;
        long newSize = size;
        bool dataFreed = false;

        // Вызов алгоритмов в зависимости от выбора пользователя
        switch (cipherChoice) {
            case 1: // Атбаш (in-place)
                printf("Выбран Атбаш.\n");
                outputData = data;
                atbash(outputData, size); 
                break;

            case 2: { // Перестановка
                char colKey[MAX_INPUT_LENGTH], rowKey[MAX_INPUT_LENGTH];
                printf("Выбрана Двойная перестановка.\n");
                readStringPrompt("Введите ключ столбцов (цифры 1..N): ", colKey, MAX_INPUT_LENGTH);
                readStringPrompt("Введите ключ строк (цифры 1..M): ", rowKey, MAX_INPUT_LENGTH);

                if (!isValidKey(colKey, (int)strlen(colKey)) || !isValidKey(rowKey, (int)strlen(rowKey))) {
                    free(data);
                    throw "Неверный формат ключа! Ключ должен содержать уникальные цифры.";
                }
                outputData = doubleTransposition(data, size, colKey, rowKey, encrypt, newSize);
                free(data); 
                dataFreed = true;
                break;
            }

            case 3: { // TEA
                char password[MAX_INPUT_LENGTH];
                printf("Выбран TEA.\n");
                readStringPrompt("Введите пароль (до 16 символов): ", password, MAX_INPUT_LENGTH);
                outputData = teaProcess(data, size, password, encrypt, newSize);
                free(data);
                dataFreed = true;
                break;
            }
            default:
                free(data);
                throw "Неверный выбор алгоритма шифрования!";
        }

        if (outputData == NULL) {
            throw "Внутренняя ошибка: алгоритм вернул пустой результат.";
        }

        // Сохранение итогового результата
        if (!writeFile(outputFile, outputData, newSize)) {
            if (outputData != data && !dataFreed) free(outputData);
            throw "Не удалось сохранить результат в выходной файл!";
        }
        
        printf("Операция завершена успешно. Результат сохранен в %s (%ld байт).\n", outputFile, newSize);

        // Ручное освобождение памяти
        if (outputData && outputData != data) free(outputData);
        else if (!outputData && !dataFreed) free(data);

    } catch (const char* errorMessage) {
        fprintf(stderr, "\nОШИБКА: %s\n", errorMessage);
    }
}

int main() {
    char inputBuf[64];
    bool running = true;

    while (running) {
        printf("\n--- Меню Криптографа ---\n");
        printf("1. Печать файла на экране\n");
        printf("2. Шифрование файла\n");
        printf("3. Дешифрование файла\n");
        printf("4. Ввод текста в файл\n");
        printf("0. Выход\n");
        printf("--------------------------\n");
        printf("Ваш выбор: ");
        fflush(stdout);
        
        readLine(inputBuf, sizeof(inputBuf));
        int choiceInt = atoi(inputBuf);
        MenuOption choice = (MenuOption)choiceInt;

        if (choiceInt == 0 && inputBuf[0] != '0') choice = (MenuOption)-1;

        // Использование switch case в связке с enum class
        switch (choice) {
            case MenuOption::PRINT_FILE: {
                char filename[MAX_INPUT_LENGTH];
                readStringPrompt("Введите имя файла для печати: ", filename, MAX_INPUT_LENGTH);
                printFile(filename);
                break;
            }
            case MenuOption::ENCRYPT:
                handleCipherOperation(true);
                break;
            case MenuOption::DECRYPT:
                handleCipherOperation(false);
                break;
            case MenuOption::WRITE_TEXT: {
                char filename[MAX_INPUT_LENGTH], text[2048];
                readStringPrompt("Введите имя файла для записи: ", filename, MAX_INPUT_LENGTH);
                printf("Введите текст: "); fflush(stdout);
                readLine(text, sizeof(text));
                
                if (writeFile(filename, text, strlen(text))) {
                    printf("Текст успешно записан.\n");
                }
                break;
            }
            case MenuOption::EXIT:
                printf("Завершение работы программы.\n");
                running = false;
                break;
            default:
                printf("Неверный выбор. Попробуйте снова.\n");
                break;
        }
    }
    return 0;
}