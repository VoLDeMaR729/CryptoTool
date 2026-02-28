#include "ciphers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TEA_DELTA 0x9e3779b9
#define TEA_ROUNDS 32

// --- Внутренние функции TEA ---

// Подготовка ключа из пароля
static void setupTeaKey(const char* password, uint32_t* k) {
    memset(k, 0, 16); 
    for (int i = 0; i < 16; ++i) {
        if (password[i] == '\0') break;
        k[i / 4] |= (uint32_t)password[i] << (8 * (i % 4));
    }
}

static void teaEncrypt(uint32_t* v, uint32_t* k) {
    uint32_t v0 = v[0], v1 = v[1], sum = 0;
    for (int i = 0; i < TEA_ROUNDS; i++) {
        sum += TEA_DELTA;
        v0 += ((v1 << 4) + k[0]) ^ (v1 + sum) ^ ((v1 >> 5) + k[1]);
        v1 += ((v0 << 4) + k[2]) ^ (v0 + sum) ^ ((v0 >> 5) + k[3]);
    }
    v[0] = v0; v[1] = v1;
}

static void teaDecrypt(uint32_t* v, uint32_t* k) {
    uint32_t v0 = v[0], v1 = v[1], sum = TEA_DELTA * TEA_ROUNDS;
    for (int i = 0; i < TEA_ROUNDS; i++) {
        v1 -= ((v0 << 4) + k[2]) ^ (v0 + sum) ^ ((v0 >> 5) + k[3]);
        v0 -= ((v1 << 4) + k[0]) ^ (v1 + sum) ^ ((v1 >> 5) + k[1]);
        sum -= TEA_DELTA;
    }
    v[0] = v0; v[1] = v1;
}

// --- Утилиты ---

// Проверка валидности числового ключа
bool isValidKey(const char* key, int length) {
    if (length == 0) return false;
    char* check = (char*)calloc(length + 1, sizeof(char)); 
    if (!check) return false;

    for (int i = 0; i < length; ++i) {
        if (!isdigit(key[i])) { free(check); return false; }
        int digit = key[i] - '0';
        if (digit < 1 || digit > length || check[digit]) { free(check); return false; }
        check[digit] = 1;
    }
    free(check);
    return true;
}

extern "C" {

// 1. Реализация Атбаш
void atbash(char* data, long size) {
    for (long i = 0; i < size; ++i) {
        char c = data[i];
        if (c >= 'A' && c <= 'Z') data[i] = 'Z' - (c - 'A');
        else if (c >= 'a' && c <= 'z') data[i] = 'z' - (c - 'a');
    }
}

// 2. Реализация Двойной Перестановки
char* doubleTransposition(const char* data, long size, const char* colKey, const char* rowKey, bool encrypt, long& newSize) {
    int C = (int)strlen(colKey);
    int R = (int)strlen(rowKey);
    if (C == 0 || R == 0) return NULL;
    
    int blockSize = C * R;
    long numBlocks = (size + blockSize - 1) / blockSize;
    newSize = numBlocks * blockSize;
    
    // Подготовка буфера с паддингом 'X'
    char* paddedData = (char*)calloc(newSize, sizeof(char));
    if (!paddedData) return NULL;
    memcpy(paddedData, data, size);
    for (long i = size; i < newSize; ++i) paddedData[i] = 'X';

    // Создание таблиц перестановки
    int colPerm[C], rowPerm[R];
    int colInvPerm[C], rowInvPerm[R];

    for (int i = 0; i < C; ++i) {
        colPerm[i] = colKey[i] - '1';
        colInvPerm[colKey[i] - '1'] = i;
    }
    for (int i = 0; i < R; ++i) {
        rowPerm[i] = rowKey[i] - '1';
        rowInvPerm[rowKey[i] - '1'] = i;
    }

    char* output = (char*)malloc(newSize);
    if (!output) { free(paddedData); return NULL; }

    // Проход по блокам
    for (long b = 0; b < numBlocks; ++b) {
        const char* in = paddedData + b * blockSize;
        char* out = output + b * blockSize;
        
        for (int i = 0; i < blockSize; ++i) {
            int r = i / C, c = i % C;
            long targetIdx;

            if (encrypt) {
                targetIdx = rowPerm[r] * C + colPerm[c];
                out[targetIdx] = in[i];
            } else {
                targetIdx = rowInvPerm[r] * C + colInvPerm[c];
                out[targetIdx] = in[i];
            }
        }
    }
    free(paddedData);

    // Удаление 'X' при дешифровке
    if (!encrypt) {
        while (newSize > 0 && output[newSize - 1] == 'X') {
            newSize--;
        }
    }
    return output;
}

// 3. Реализация TEA
char* teaProcess(const char* data, long size, const char* password, bool encrypt, long& newSize) {
    uint32_t key[4];
    setupTeaKey(password, key);

    int block_size = 8;
    long padding_needed = 0; 
    long paddedSize;

    if (encrypt) {
        padding_needed = (block_size - (size % block_size));
        paddedSize = size + padding_needed;
    } else {
        if (size % block_size != 0) return NULL;
        paddedSize = size;
    }

    char* buffer = (char*)malloc(paddedSize);
    if (!buffer) return NULL;

    // Шифрование
    if (encrypt) {
        memcpy(buffer, data, size);
        memset(buffer + size, (char)padding_needed, padding_needed); // PKCS#7 padding

        for (long i = 0; i < paddedSize; i += block_size) {
            teaEncrypt((uint32_t*)(buffer + i), key);
        }
        newSize = paddedSize;
    } 
    // Дешифрование
    else {
        memcpy(buffer, data, size);
        for (long i = 0; i < paddedSize; i += block_size) {
            teaDecrypt((uint32_t*)(buffer + i), key);
        }

        // Проверка и удаление паддинга
        int padVal = buffer[paddedSize - 1];
        if (padVal > 0 && padVal <= block_size) {
            newSize = paddedSize - padVal;
        } else {
            newSize = paddedSize; // Ошибка паддинга, оставляем как есть
        }
    }
    return buffer;
}
}