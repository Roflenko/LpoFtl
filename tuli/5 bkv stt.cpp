#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <cctype>
#include <vector>
#include <iomanip>

bool isLowerLatin(char c) {
    return c >= 'a' && c <= 'z';
}

int main() {
    std::ifstream dictFile("1.txt");
    if (!dictFile.is_open()) {
        std::cerr << "Не удалось открыть файл 1.txt\n";
        return 1;
    }

    // Алфавит конланга
    std::string alphabet = "aiueoxwympfntslrkhcqbvdzgj";

    // Счётчики букв
    std::unordered_map<char, size_t> counts;
    size_t totalLetters = 0;

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

            // считаем буквы
            for (char c : word) {
                if (isLowerLatin(c)) {
                    counts[c]++;
                    totalLetters++;
                }
            }

            if (next == std::string::npos) break;
            pos = next + 3;
        }
    }
    dictFile.close();

    // --- Вывод статистики ---
    std::cout << "--- statistika bukv:\n";
    for (char c : alphabet) {
        size_t cnt = counts[c];
        double percent = totalLetters > 0 ? (100.0 * cnt / totalLetters) : 0.0;
        std::cout << c << " : " << std::setw(4) << cnt
                  << " (" << std::fixed << std::setprecision(2) << percent << "%)\n";
    }
}
