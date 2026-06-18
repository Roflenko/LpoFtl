#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
using namespace std;

int main() {
    ifstream file("1.txt");
    if (!file.is_open()) {
        cerr << "Не удалось открыть файл 1.txt\n";
        return 1;
    }

    string line;
    int lemmas = 0;
    int proper = 0;
    int total = 0;
    int compounds = 0;  // Счётчик составных слов

    while (getline(file, line)) {
        if (line.empty()) continue; // пропускаем пустые строки

        char firstChar = line[0];
        if (isalpha(static_cast<unsigned char>(firstChar))) {
            total++; // учитываем только строки, начинающиеся с буквы

            // Проверяем, является ли строка леммой (начинается с маленькой буквы)
            if (islower(static_cast<unsigned char>(firstChar))) {
                lemmas++;

                // Проверяем строку на наличие специальных символов
                bool hasQuote = (line.find('"') != string::npos);
                bool hasDoubleEquals = (line.find("==") != string::npos);

                if (hasQuote) {
                    compounds++;  // Увеличиваем счётчик составных слов
                }
                else if (!hasDoubleEquals) {
                    // Если нет ни ", ни ==, то это "плохая" строка
                    cout << "bad line:   " << line << "\n";
                }
                // Если есть ==, ничего не делаем
            }
            else if (isupper(static_cast<unsigned char>(firstChar))) {
                proper++;
            }
        }
    }

    file.close();

    cout << "-- statistics:\n";
    cout << "lemmas: " << lemmas << endl;
    cout << "proper nouns and abbreviations: " << proper << endl;
    cout << "total: " << total << endl;
    cout << "compounds: " << compounds << endl;
    cout << "simple words: " << (lemmas - compounds) << endl;
}
