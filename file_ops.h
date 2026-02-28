#pragma once

// Макрос для экспорта функций
#ifdef _MSC_VER
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT
#endif

// Обеспечиваем C-совместимость имен для линковщика
#ifdef __cplusplus
extern "C" {
#endif

// Читает файл в бинарном режиме. Возвращает буфер (требует free).
EXPORT char* readFile(const char* filename, long& size);

// Записывает бинарный буфер в файл.
EXPORT bool writeFile(const char* filename, const char* buffer, long size);

// Печатает содержимое файла на экран.
EXPORT void printFile(const char* filename);

#ifdef __cplusplus
}
#endif