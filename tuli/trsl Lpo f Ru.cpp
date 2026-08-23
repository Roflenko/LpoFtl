#include <iostream>
#include <string>
#include <cctype>
#include <windows.h>

//файл:
#include <fstream>

bool isConsonant(char c) {
    std::string consonants = "бвгджзйклмнпрстфхцчшщ";
    return consonants.find(c) != std::string::npos;
}

bool isVowel(char c) {
    std::string vowels = "аеёиоуыэюяaiueo";
    return vowels.find(c) != std::string::npos;
}

bool isSpecialPalatal(char c) {
    return c == 'ш' || c == 'ч' || c == 'ж';
}

bool isVoicedConsonant(char c) {
    std::string voiced = "бвджзг";
    return voiced.find(c) != std::string::npos;
}

//bool biBv(char c) {
//    std::string bvi = "aiueoxwympfntslrkhcqbvdzgj";
//    return bvi.find(c) != std::string::npos;
//}

std::string processWord(const std::string& word) {
    std::string result;

    // 1. Заменяем буквы согласно правилам
    for (size_t i = 0; i < word.length(); ++i) {
        char c = word[i];

        // Проверяем диграфы
        if (i + 1 < word.length()) {
            std::string digraph = word.substr(i, 2);
            if (digraph == "ts") {
                result += 'ц';
                i++; // пропускаем вторую букву
                continue;
            } else if (digraph == "qq") {
                result += 'щ';
                i++;
                continue;
            } else if (i == 0 && digraph == "ie") {
                result += 'е';
                i++;
                continue;
            }
        }

        // Заменяем буквы
        switch (c) {
            case 'a': result += 'а'; break;
            case 'i': result += 'и'; break;
            case 'u': result += 'у'; break;
            case 'o': result += 'о'; break;

            case 'e':
                // Если после согласной - "е", иначе "э"
                if (i > 0 && isConsonant(result[i-1]) && !isSpecialPalatal(result[i-1]) ) {
                    result += 'е';
                } else if (isSpecialPalatal(result[i-1]) && !isVowel( word[i+1] ) ){
                    result += 'ь';
                    result += 'е';
                } else {
                    result += 'э';
                }
                break;

            case 'x': result += 'я'; break;
            case 'w': result += 'ю'; break;
            case 'y': result += 'ё'; break;

            case 'm': result += 'м'; break;
            case 'p': result += 'п'; break;
            case 'f': result += 'ф'; break;

            case 'n': result += 'н'; break;
            case 't': result += 'т'; break;
            case 's': result += 'с'; break;
            case 'l': result += 'л'; break;
            case 'r': result += 'р'; break;

            case 'k': result += 'к'; break;
            case 'h': result += 'х'; break;

            case 'c': result += 'ш'; break;
            case 'q': result += 'ч'; break;

            case 'b': result += 'б'; break;
            case 'v': result += 'в'; break;
            case 'd': result += 'д'; break;
            case 'z': result += 'з'; break;
            case 'g': result += 'г'; break;
            case 'j': result += 'ж'; break;

            default: result += c; break;
        }
    }

    // 2. Обработка палатализаций
    std::string finalResult;
    for (size_t i = 0; i < result.length(); ++i) {
        char c = result[i];

        // Проверяем палатализацию: после "c,q,j" идут "x,w,y"
        if (i > 0 && isSpecialPalatal(result[i-1])) {
            char prev = result[i-1];
            if (c == 'я' || c == 'ю' || c == 'ё') {
                // Вставляем "ь" перед гласной
                finalResult.pop_back(); // убираем предыдущую согласную
                switch (prev) {
                    case 'ш': finalResult += "шь"; break;
                    case 'ч': finalResult += "чь"; break;
                    case 'ж': finalResult += "жь"; break;
                    default: finalResult += prev; break;
                }
                finalResult += c;
                continue;
            }
        }
        //символы
        switch (c) {
           case '1': finalResult += "ун"; continue;
           case '2': finalResult += "дуф"; continue;
           case '3': finalResult += "сри"; continue;
           case '4': finalResult += "фор"; continue;
           case '5': finalResult += "фиф"; continue;
           case '6': finalResult += "шс"; continue;
           case '7': finalResult += "сэф"; continue;
           case '8': finalResult += "эйт"; continue;
           case '9': finalResult += "най"; continue;
           case 'Њ': finalResult += "хой"; continue;
           case 'Ћ': finalResult += "гиш"; continue;
           case 'Ќ': finalResult += "ках"; continue;
           case 'њ': finalResult += "боф"; continue;
           case 'ћ': finalResult += "мхе"; continue;
           case 'ќ': finalResult += "пиф"; continue;
           case '&': finalResult += "эн"; continue;
           case '|': finalResult += "чи"; continue;
           case '›': finalResult += "so"; continue;
           case '‹': finalResult += "os"; continue;
           case '%': finalResult += "псе"; continue;
           case '+': finalResult += "пю"; continue;
           case '`': finalResult += "мул"; continue;
           case '/': finalResult += "див"; continue;
           case '=': finalResult += "ра"; continue;
           case '”': finalResult += "мочр"; continue;
           case '“': finalResult += "эсчр"; continue;
           case '~': finalResult += "ипо"; continue;
           case 'Ѓ': finalResult += "апор"; continue;
           case '¬': finalResult += "нэ "; continue;
           case '°': finalResult += "гра"; continue;
           case 'Љ': finalResult += "ифи"; continue;

           case '-':
              if (result[i+1] == '>'){
                  finalResult += "со";
                  i++;
              }else if (result[i+1] == '<'){
                  finalResult += "ос";
                  i++;
              }else if (result[i+1] == '-'){
                  finalResult += "   ";
                  i++;
              }else{
                  finalResult += "су";
              }
           continue;

           case '.':
              if ( result[i+1] ) {
                 finalResult += " ";
              } else{
                 finalResult += ".";
              }
           continue;

           case '>':
              if (result[i+1] == '='){
                  finalResult += "мочр";
                  i++;
              }else{
                  finalResult += "моч";
              }
           continue;

           case '<':
              if (result[i+1] == '='){
                  finalResult += "эсчр";
                  i++;
              }else{
                  finalResult += "эсч";
              }
           continue;

           case ':':
              if (result[i+1] == ':'){
                  finalResult += " bi ";
                  i++;
              }else{
                  finalResult += " хв ";
              }
           continue;
        }
        finalResult += c;
    }

    // 3. Вставляем "э" между согласными
    std::string withEpenesis;
    for (size_t i = 0; i < finalResult.length(); ++i) {
        char c = finalResult[i];
        withEpenesis += c;

        // Если текущая и следующая - согласные, вставляем "э"
        if (i + 1 < finalResult.length()) {
            if (isConsonant(c) && isConsonant(finalResult[i+1])) {
                withEpenesis += 'э';
            }
        }
    }

    // 4. Обработка "i" -> "й" после гласной, но не перед гласной
    std::string finalFinal;
    for (size_t i = 0; i < withEpenesis.length(); ++i) {
        char c = withEpenesis[i];
        if (c == 'и' && i > 0 && isVowel(withEpenesis[i-1])) {
            // Проверяем, что следующая не гласная
            if (i + 1 >= withEpenesis.length() || !isVowel(withEpenesis[i+1])) {
                finalFinal += 'й';
                continue;
            }
        }
        finalFinal += c;
    }

    // 5. Концевая эпентеза для звонких
    std::string resultWithEndE;
    for (size_t i = 0; i < finalFinal.length(); ++i) {
        char c = finalFinal[i];
        resultWithEndE += c;

        // Если это последняя буква и она звонкая
        if (i == finalFinal.length() - 1 && isVoicedConsonant(c)) {
            resultWithEndE += 'э';
        }
    }

    // 6. Слово из одной согласной
    if (resultWithEndE.length() == 1 && isConsonant(resultWithEndE[0])) {
        return resultWithEndE + "э";
    }

    return resultWithEndE;
}

std::string transliterate(const std::string& input) {
    std::string result;
    std::string word;

    // Проходим по каждому символу
    for (size_t i = 0; i < input.length(); ++i) {
        char c = std::tolower(input[i]);

        if (c == ' ') {
            // Обрабатываем накопленное слово
            if (!word.empty()) {
                result += processWord(word);
                word.clear();
            }
            result += ' ';
        } else {
            word += c;
        }
    }

    // Обрабатываем последнее слово
    if (!word.empty()) {
        result += processWord(word);
    }

    return result;
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    std::string input = "2. bnsa fx sx Kana, bard";

    std::string content = std::string(std::istreambuf_iterator<char>(
    std::ifstream("1.txt").rdbuf()), std::istreambuf_iterator<char>());

    std::cout << content << "\n\n=\n\n";
    std::cout << transliterate(content) << std::endl;

    return 0;
}
