#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <vector>
#include <string>
#include <locale.h> //для вывода русскими буквами

// Сортируемый список: "1.txt"
// Отсортированный список: "2.txt"

// Заданный алфавит для сортировки
const char *alphabet = "aiueoxwympfntslrkhcqbvdzgj";

// Функция для сравнения строк согласно алфавиту
bool compare_strings(const std::string& a, const std::string& b) {
    size_t i = 0;
    while (i < a.size() && i < b.size()) {
        char c1 = tolower(a[i]);
        char c2 = tolower(b[i]);

        const char *pos1 = strchr(alphabet, c1); //адрес симвоал в алфавите
        const char *pos2 = strchr(alphabet, c2);

        int idx1 = pos1 ? pos1 - alphabet : -1; // индекс в алфавите при помощи адресной арифметики
        int idx2 = pos2 ? pos2 - alphabet : -1;

        if (idx1 != idx2) {
            return idx1 < idx2;
        }
        i++;
    }

    return a.size() < b.size();
}

int main() {
    setlocale(LC_ALL, "Russian");  //Активировать вывод на русском их <locale.h>
    FILE *input_file, *output_file;
    std::vector<std::string> lines;
    char buffer[1024];

    // Открываем файл для чтения
    input_file = fopen("1.txt", "r");
    if (input_file == nullptr) {
        perror("Ne udalosy otkryyty fayl 1.txt");
        return 1;
    }

    // Читаем строки из файла
    while (fgets(buffer, sizeof(buffer), input_file) != nullptr) {
        // Удаляем символ новой строки
        buffer[strcspn(buffer, "\n")] = '\0';
        lines.emplace_back(buffer);
    }
    fclose(input_file);

    // Сортируем строки
    std::sort(lines.begin(), lines.end(), compare_strings);

    // Открываем файл для записи
    output_file = fopen("2.txt", "w");
    if (output_file == nullptr) {
        perror("Ne udalosy otkryyty fayl 2.txt");
        return 1;
    }

    // Записываем отсортированные строки в файл
    for (const auto& line : lines) {
        fprintf(output_file, "%s\n", line.c_str());
    }

    fclose(output_file);

    printf("Stroki otsortirovanyy i zapisanyy в 2.txt\n");

    return 0;
}
