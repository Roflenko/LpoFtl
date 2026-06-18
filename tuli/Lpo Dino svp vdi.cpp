#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cctype>

//«1.txt» :: pss
//«2.txt» :: ruli
const bool R = 0;   // R==0 -> vdiL -> vdiP; nl: vdiP -> vdiL

bool isWordBoundary(char c) {
    return !std::isalnum(static_cast<unsigned char>(c)) && c != '_';
}

bool isWholeWord(const std::string& text, size_t pos, const std::string& word) {
    // Проверяем левую границу
    bool leftBoundary = (pos == 0) || isWordBoundary(text[pos - 1]);

    // Проверяем правую границу
    bool rightBoundary = (pos + word.length() == text.length()) ||
                         isWordBoundary(text[pos + word.length()]);

    return leftBoundary && rightBoundary;
}

void replaceWholeSequences(std::string& text,
                           const std::unordered_map<std::string, std::string>& rules) {

    std::string result;
    size_t i = 0;

    while (i < text.length()) {
        bool found = false;

        // Пробуем найти самое длинное совпадение в текущей позиции
        for (const auto& rule : rules) {
            const std::string& from = rule.first;

            if (i + from.length() <= text.length()) {
                std::string potential = text.substr(i, from.length());

                if (potential == from && isWholeWord(text, i, from)) {
                    // Нашли совпадение целой последовательности
                    result += rule.second;
                    i += from.length();
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            // Если не нашли правило, копируем текущий символ
            result += text[i];
            i++;
        }
    }

    text = result;
}

void processFile(const std::string& inputFilename,
                 const std::string& rulesFilename,
                 bool reverseMode) {

    // Чтение правил из файла 2.txt
    std::unordered_map<std::string, std::string> rules;
    std::ifstream rulesFile(rulesFilename);

    if (!rulesFile.is_open()) {
        std::cerr << "n  rulesFilename" << rulesFilename << std::endl;
        return;
    }

    std::string line;

    while (std::getline(rulesFile, line)) {
        size_t separatorPos = line.find(" = ");

        // Проверяем наличие разделителя и наличие слов с обеих сторон
        if (separatorPos == std::string::npos ||
            separatorPos == 0 ||
            separatorPos + 3 >= line.length()) {
            continue;
        }

        std::string words1 = line.substr(0, separatorPos);
        std::string words2 = line.substr(separatorPos + 3);

        // Проверяем, что строки не пустые (уже есть слова)
        if (words1.empty() || words2.empty()) {
            continue;
        }

        // В зависимости от режима сохраняем правило
        if (reverseMode) {
            // R: заменяем набор_слов1 на набор_слов2
            rules[words1] = words2;
        } else {
            // Не R: заменяем набор_слов2 на набор_слов1
            rules[words2] = words1;
        }
    }

    rulesFile.close();

    if (rules.empty()) {
        std::cerr << "n rul i f rulesFilename" << rulesFilename << std::endl;
        return;
    }

    // Чтение и обработка файла 1.txt
    std::ifstream inputFile(inputFilename);
    if (!inputFile.is_open()) {
        std::cerr << "n inputFilename" << inputFilename << std::endl;
        return;
    }

    std::ofstream tempFile("tmp.txt");
    if (!tempFile.is_open()) {
        std::cerr << "n tempFile" << std::endl;
        inputFile.close();
        return;
    }

    while (std::getline(inputFile, line)) {
        std::string result = line;
        replaceWholeSequences(result, rules);
        tempFile << result << std::endl;
    }

    inputFile.close();
    tempFile.close();

    std::remove(inputFilename.c_str());
    std::rename("tmp.txt", inputFilename.c_str());

    std::cout << "og, rj "
              << (reverseMode ? "R: vdi1 -> vdi2" : "N: vdi2 -> vdi1")
              << std::endl;
}

int main() {
    processFile("1.txt", "2.txt", R);
}
