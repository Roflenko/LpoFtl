#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <cctype>

bool isLowerLatin(char c) {
    return c >= 'a' && c <= 'z';
}

int main() {
    std::ifstream file("1.txt");
    if (!file.is_open()) {
        std::cerr << "no 1.txt\n";
        return 1;
    }

    std::unordered_set<std::string> seen;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // Проверяем, начинается ли строка со строчной латинской буквы
        if (!isLowerLatin(line[0])) continue;

        // Находим конец области слов
        size_t endPos = line.find("==");
        size_t quotePos = line.find('"');

        if (quotePos != std::string::npos)
            endPos = std::min(endPos, quotePos);

        if (endPos == std::string::npos)
            endPos = line.size();

        // Берём только часть строки, где находятся слова
        std::string wordsPart = line.substr(0, endPos);

        // Разделяем по " = "
        size_t pos = 0;
        while (true) {
            size_t next = wordsPart.find(" = ", pos);
            std::string word;

            if (next == std::string::npos) {
                word = wordsPart.substr(pos);
            } else {
                word = wordsPart.substr(pos, next - pos);
            }

            // Убираем пробелы по краям
            while (!word.empty() && std::isspace(word.front())) word.erase(word.begin());
            while (!word.empty() && std::isspace(word.back())) word.pop_back();

            if (!word.empty()) {
                if (seen.count(word)) {
                    std::cout << "homonim: " << word << "\n";
                } else {
                    seen.insert(word);
                }
            }

            if (next == std::string::npos) break;
            pos = next + 3; // длина " = "
        }
    }
    std::cout << "words sum: " << seen.size() << "\n";
}
