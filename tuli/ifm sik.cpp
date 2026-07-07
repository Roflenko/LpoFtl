#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

// Функция для проверки, начинается ли строка с буквы (кириллица или латиница)
bool startsWithLetter(const string& str) {
    if (str.empty()) return false;
    unsigned char ch = str[0];
    // Проверяем на кириллицу (Win1251)
    if (ch >= 0x80 && ch <= 0xFF) return true;
    // Проверяем на латиницу
    if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) return true;
    return false;
}

// Функция для извлечения слов из строки до маркеров "==" или '"'
vector<string> extractWords(const string& line) {
    vector<string> words;
    size_t pos = 0;

    while (pos < line.length()) {
        // Ищем конец текущего слова
        size_t endPos = line.find(" = ", pos);
        if (endPos == string::npos) break;

        // Извлекаем слово
        string word = line.substr(pos, endPos - pos);
        // Удаляем пробелы в начале и конце
        word.erase(0, word.find_first_not_of(" \t"));
        word.erase(word.find_last_not_of(" \t") + 1);

        // Проверяем, не является ли слово маркером "==" или не содержит '"'
        if (word == "==" || word.find('"') != string::npos) {
            break;
        }

        if (!word.empty()) {
            words.push_back(word);
        }

        // Перемещаемся за " = "
        pos = endPos + 3;
    }

    return words;
}

// Функция для проверки, заканчивается ли слово на заданное буквосочетание
bool endsWith(const string& word, const string& suffix) {
    if (suffix.length() > word.length()) return false;
    return word.compare(word.length() - suffix.length(), suffix.length(), suffix) == 0;
}

// Функция для очистки консоли
void clearConsole() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

int main() {
    // Устанавливаем кодировку для консоли Windows
#ifdef _WIN32
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
#endif

    cout << "Программа поиска рифм в словаре" << endl;
    cout << "Словарь должен быть в файле '1.txt'" << endl;
    cout << "=========================================" << endl;

    while (true) {
        // Читаем словарь
        ifstream file("1.txt");
        if (!file.is_open()) {
            cerr << "Ошибка: не удалось открыть файл '1.txt'" << endl;
            system("pause");
            return 1;
        }

        // Запрос буквосочетания
        string pattern;
        cout << "\nВведите буквосочетание для поиска рифм: ";
        getline(cin, pattern);

        if (pattern.empty()) {
            cout << "Буквосочетание не может быть пустым!" << endl;
            file.close();
            cout << "\nНажмите Enter для продолжения...";
            cin.get();
            clearConsole();
            continue;
        }

        // Поиск по словарю
        vector<string> foundLines;
        string line;
        int lineNumber = 0;

        while (getline(file, line)) {
            lineNumber++;

            // Проверяем первое условие: строка должна начинаться с буквы
            if (!startsWithLetter(line)) {
                continue;
            }

            // Извлекаем слова из строки
            vector<string> words = extractWords(line);

            // Проверяем каждое слово на рифму
            bool hasRhyme = false;
            for (const string& word : words) {
                if (endsWith(word, pattern)) {
                    hasRhyme = true;
                    break;
                }
            }

            if (hasRhyme) {
                foundLines.push_back(line);
            }
        }

        file.close();

        // Вывод результатов
        if (foundLines.empty()) {
            cout << "\nСтрок с рифмой на '" << pattern << "' не найдено." << endl;
        } else {
            cout << "\nНайдены строки с рифмой на '" << pattern << "':" << endl;
            cout << "=========================================" << endl;
            for (const string& line : foundLines) {
                cout << line << endl;
            }
            cout << "=========================================" << endl;
            cout << "Всего найдено строк: " << foundLines.size() << endl;
        }

        // Предложение продолжить
        cout << "\nНажмите Enter, чтобы очистить консоль и продолжить поиск...";
        cin.get();
        clearConsole();
        cout << "Программа поиска рифм в словаре" << endl;
        cout << "Словарь должен быть в файле '1.txt'" << endl;
        cout << "=========================================" << endl;
    }

    return 0;
}
