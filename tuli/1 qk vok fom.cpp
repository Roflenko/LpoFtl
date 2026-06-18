#include <iostream>
#include <fstream>
#include <string>

int main() {
    std::ifstream in("1.txt");
    if (!in.is_open()) {
        std::cerr << "no file 1.txt\n";
        return 1;
    }

    std::ofstream out("1.tmp");
    if (!out.is_open()) {
        std::cerr << "cannot create temp file\n";
        return 1;
    }

    std::string line;
    int lineNumber = 0;

    while (std::getline(in, line)) {
        lineNumber++;

        if (!line.empty() && std::isalpha(static_cast<unsigned char>(line[0]))) {
            size_t pos = line.find('=');

            if (pos != std::string::npos) {
                size_t lastPos = line.rfind('=');

                if (lastPos > 0 && line[lastPos - 1] != '=') {
                    line.insert(lastPos, "=");
                    std::cout << "fixed \"==\" in line " << lineNumber << "\n";
                }
            }
        }

        out << line << "\n";
    }

    in.close();
    out.close();

    std::remove("1.txt");
    std::rename("1.tmp", "1.txt");
}
