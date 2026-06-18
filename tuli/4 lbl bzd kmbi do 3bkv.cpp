#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <cctype>
#include <vector>

bool isLowerLatin(char c) {
    return c >= 'a' && c <= 'z';
}

int main() {
    int sum = 0;

    // --- 1. Чтение словаря 1.txt ---
    std::ifstream dictFile("1.txt");
    if (!dictFile.is_open()) {
        std::cerr << "Не удалось открыть файл 1.txt\n";
        return 1;
    }

    std::unordered_set<std::string> shortWords;
    std::string line;

    while (std::getline(dictFile, line)) {
        if (line.empty()) continue;

        if (!isLowerLatin(line[0])) continue;

        size_t endPos = line.find("==");
        size_t quotePos = line.find('"');

        if (quotePos != std::string::npos)
            endPos = std::min(endPos, quotePos);

        if (endPos == std::string::npos)
            endPos = line.size();

        std::string wordsPart = line.substr(0, endPos);

        size_t pos = 0;
        while (true) {
            size_t next = wordsPart.find(" = ", pos);
            std::string word;

            if (next == std::string::npos)
                word = wordsPart.substr(pos);
            else
                word = wordsPart.substr(pos, next - pos);

            // trim
            while (!word.empty() && std::isspace(word.front())) word.erase(word.begin());
            while (!word.empty() && std::isspace(word.back())) word.pop_back();

            if (!word.empty() && word.size() < 4) {
                shortWords.insert(word);
            }

            if (next == std::string::npos) break;
            pos = next + 3;
        }
    }

    dictFile.close();

    // --- 2. Чтение 2.txt ---
    std::ifstream file2("2.txt");
    if (!file2.is_open()) {
        std::cerr << "Не удалось открыть файл 2.txt\n";
        return 1;
    }

    std::vector<std::string> lines2;
    while (std::getline(file2, line)) {
        if (!line.empty()) {
            std::string word = line;

            // Если слово есть среди коротких — добавляем " +"
            if (shortWords.count(word)) {
                line += " +";
                sum++;
            }
        }
        lines2.push_back(line);
    }

    file2.close();

    // --- 3. Перезапись 2.txt ---
    std::ofstream out("2.txt");
    if (!out.is_open()) {
        std::cerr << "Не удалось открыть файл 2.txt для записи\n";
        return 1;
    }

    for (const auto& l : lines2) {
        out << l << "\n";
    }

    out.close();
    std::cout << "total 1..3 symbol words: " << sum << "\n";
}
