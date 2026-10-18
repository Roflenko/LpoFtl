#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>

// Проверка, начинается ли строка с латинской буквы
bool startsWithLetter(const std::string& line) {
    return !line.empty() && std::isalpha(static_cast<unsigned char>(line[0]));
}

// Обработка одной строки словаря
std::string processLine(const std::string& line) {
    // 1. Найти первый '='
    size_t eqPos = line.find('=');
    if (eqPos == std::string::npos) return line;

    // 2. После '=' найти '@' — стереть всё до '@' включительно
    size_t atPos = line.find('@', eqPos);
    if (atPos == std::string::npos) return line;

    // Часть до '=' (включая '=') + часть после '@'
    std::string result = line.substr(0, eqPos + 1);
    std::string rest = line.substr(atPos + 1);

    // 2.5. Вырезать слот "смотрите также" — последовательность "   ^" и всё после неё
    size_t caretPos = rest.find("   ^");
    if (caretPos != std::string::npos) {
        rest = rest.substr(0, caretPos);
    }

    // 3. Пропустить пробелы в начале остатка (после '@' обычно пробел)
    size_t start = 0;
    while (start < rest.size() && rest[start] == ' ') ++start;
    rest = rest.substr(start);

    // 4. Пропустить участок до первой запятой (оставляя его как есть)
    size_t commaPos = rest.find(',');
    if (commaPos != std::string::npos) {
        rest = rest.substr(0, commaPos);
    }

    // Убрать возможные хвостовые пробелы
    while (!rest.empty() && (rest.back() == ' ' || rest.back() == '\r'))
        rest.pop_back();

    result += rest;
    return result;
}

int main() {
    std::ifstream in("1.txt");
    if (!in.is_open()) {
        std::cerr << "ny 1.txt\n";
        return 1;
    }

    std::ofstream out("2.txt");
    if (!out.is_open()) {
        std::cerr << "ny 2.txt fo pi\n";
        return 1;
    }

    std::vector<std::string> outputLines;
    std::string line;
    bool prevWasCompacted = false;

    while (std::getline(in, line)) {
        // Убрать \r на случай CRLF
        if (!line.empty() && line.back() == '\r') line.pop_back();

        if (startsWithLetter(line)) {
            std::string processed = processLine(line);
            if (prevWasCompacted && !outputLines.empty()) {
                // Заменить перевод строки в предыдущей строке на " . "
                outputLines.back() += " . ";
                outputLines.push_back(processed);
            } else {
                outputLines.push_back(processed);
            }
            prevWasCompacted = true;
        } else {
            outputLines.push_back(line);
            prevWasCompacted = false;
        }
    }
    in.close();

    // Записать результат: строки, которые были "склеены", уже содержат " . "
    // внутри как разделитель, поэтому нужно объединять их в одну строку.
    // Пересоберём: склеенные компактизированные строки идут в одну строку.
    std::string finalText;
    bool needNewline = false;
    // Проще: пройтись по outputLines и вывести, но склеенные строки
    // должны быть на одной строке. Используем маркер: если строка была
    // добавлена после " . ", она должна идти в той же физической строке.

    // Переделаем логику записи напрямую.
    // Сбросим и запишем иначе.
    out.close();
    out.open("2.txt", std::ios::trunc);

    prevWasCompacted = false;
    bool pendingSeparator = false;
    std::string currentPhysicalLine;

    in.open("1.txt");
    std::string l;
    bool first = true;
    while (std::getline(in, l)) {
        if (!l.empty() && l.back() == '\r') l.pop_back();

        if (startsWithLetter(l)) {
            std::string processed = processLine(l);
            if (prevWasCompacted) {
                out << " . " << processed;
            } else {
                if (!first) out << "\n";
                out << processed;
            }
            prevWasCompacted = true;
            first = false;
        } else {
            // Описательная строка — всегда с новой строки
            out << "\n" << l;
            prevWasCompacted = false;
            first = false;
        }
    }
    in.close();
    out.close();

    std::cout << "rdi . rz f 2.txt\n";
    return 0;
}
