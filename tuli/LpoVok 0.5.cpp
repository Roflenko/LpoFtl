// ============================================================
// ЗАГОЛОВКИ (стандартные, обычно не трогаем)
// ============================================================
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#ifndef UNICODE
#define UNICODE
#endif

// Для Rich Edit Control нужна дополнительная библиотека
#include <richedit.h>

// Для MinGW явно укажем класс Rich Edit
#ifndef RICHEDIT_CLASS
#define RICHEDIT_CLASS _T("RichEdit20A")
#endif

// ============================================================
// # СЛОТ 1: ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ (иконки, меню, данные)

// Идентификаторы для элементов управления (чтобы различать сообщения)
#define   ID_EDIT_INPUT  1001
#define   ID_RICHEDIT_OUTPUT   1002
#define   ID_BUTTON_CLEAR   1003
#define   IDM_TOGGLE_ONTOP   1004
#define   IDM_TOGGLE_FUL  1005
#define   IDM_XRB_SNS 1006
#define   IDM_RELOAD_DICT    1007

// Дескрипторы элементов управления
HWND g_hEditInput;      // поле ввода
HWND g_hRichEdit;       // поле вывода с форматированием
HWND g_hButtonClear;  //btn сброс ввода
HINSTANCE hInst; // дескриптор приложения (нужен для ресурсов)
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);  // Объявление функции оконной процедуры (стандарт)

static TCHAR szWindowClass[] = _T("DesktopApp");   // Основное имя класса окна (редко трогаем, разве что для нескольких окон)
static TCHAR szTitle[] = _T("Lpo 0.5");   // Заголовок окна (будете менять под каждое приложение)
int OknC = 300; //начальная ширина
const int OknCMin=300; //минимальная ширина
int OknH = 300; //начальная высота
const int OknHMin=135; //минимальная высота
int EdtH;
const int EdtHMin=25;
const int EdtGliLm=22;
const int TxtSzMin=320;
std::wstring g_dictPath = L"0 Lpo.txt";  // Путь к файлу словаря
std::wstring sikVd; //искомое слово
std::wstring g_dictionaryText;  // сам словарь
std::vector<std::wstring> g_dictionaryLines;  // найденные строки
std::map<std::wstring, std::vector<std::wstring>> g_searchCache;   // Кеш результатов поиска (для быстродействия)
char g_cv = 'r'; // | 0
#define NwaGh 200
#define NwaRd 192
bool oi_fh_vd = 0;
bool xrb_sns = 0;   //важны ли большие буквы

//
// ============================================================
// перехват  буфера для запрета переноса стилей в|из полей RichEdit

// Глобальные переменные
WNDPROC g_oldEditProc = nullptr;
WNDPROC g_oldOutputProc = nullptr;

// Новая оконная процедура (для ANSI сборки)
LRESULT CALLBACK EditSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_KEYDOWN: {
            BOOL bCtrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;

            // Ctrl+C (копирование)
            if (bCtrl && wParam == 'C') {
                int start = 0, end = 0;
                SendMessage(hWnd, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);

                if (start != end) {
                    int len = end - start;

                    // Используем ANSI буфер
                    char* buffer = new char[len + 1];

                    // Используем TEXTRANGEA для ANSI
                    TEXTRANGEA tr;
                    tr.chrg.cpMin = start;
                    tr.chrg.cpMax = end;
                    tr.lpstrText = buffer;

                    // Отправляем ANSI версию сообщения
                    SendMessageA(hWnd, EM_GETTEXTRANGE, 0, (LPARAM)&tr);

                    // Помещаем в буфер обмена как текст (CF_TEXT - это ANSI)
                    if (OpenClipboard(hWnd)) {
                        EmptyClipboard();

                        size_t size = (len + 1) * sizeof(char);
                        HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, size);

                        if (hData) {
                            char* pDest = (char*)GlobalLock(hData);
                            strcpy_s(pDest, len + 1, buffer);
                            GlobalUnlock(hData);

                            SetClipboardData(CF_TEXT, hData);
                        }
                        CloseClipboard();
                    }

                    delete[] buffer;
                }
                return 0;  // Запрещаем стандартную обработку
            }

            // Ctrl+V (вставка)
            else if (bCtrl && wParam == 'V') {
                if (OpenClipboard(nullptr)) {
                    // Получаем ANSI текст
                    HANDLE hData = GetClipboardData(CF_TEXT);

                    // Если нет ANSI, пробуем Unicode
                    if (!hData) {
                        hData = GetClipboardData(CF_UNICODETEXT);
                        if (hData) {
                            wchar_t* pszWide = (wchar_t*)GlobalLock(hData);
                            if (pszWide) {
                                // Конвертируем Unicode в ANSI
                                int len = WideCharToMultiByte(1251, 0, pszWide, -1, NULL, 0, NULL, NULL);
                                char* pszAnsi = new char[len];
                                WideCharToMultiByte(1251, 0, pszWide, -1, pszAnsi, len, NULL, NULL);

                                SendMessageA(hWnd, EM_REPLACESEL, TRUE, (LPARAM)pszAnsi);

                                delete[] pszAnsi;
                                GlobalUnlock(hData);
                            }
                        }
                    }
                    else {
                        char* pszText = (char*)GlobalLock(hData);
                        if (pszText) {
                            SendMessageA(hWnd, EM_REPLACESEL, TRUE, (LPARAM)pszText);
                            GlobalUnlock(hData);
                        }
                    }

                    CloseClipboard();
                }
                return 0;
            }
            break;
        }

        case WM_PASTE: {
            // Вставка из контекстного меню
            if (OpenClipboard(nullptr)) {
                HANDLE hData = GetClipboardData(CF_TEXT);
                if (hData) {
                    char* pszText = (char*)GlobalLock(hData);
                    if (pszText) {
                        SendMessageA(hWnd, EM_REPLACESEL, TRUE, (LPARAM)pszText);
                        GlobalUnlock(hData);
                    }
                }
                CloseClipboard();
            }
            return 0;
        }
    }

    return CallWindowProcA(g_oldEditProc, hWnd, uMsg, wParam, lParam);
}

//
// ============================================================
// ФУНКЦИИ:
int EdtHFnk()
{
    int H = OknH/16;
    if (H < EdtHMin) H = EdtHMin;
    return H;
}

// Конвертация из Windows-1251 в UTF-16 (Unicode)
std::wstring ConvertWin1251ToUnicode(const std::string& win1251Text)
{
    // Получаем необходимый размер буфера для Unicode строки
    int sizeNeeded = MultiByteToWideChar(1251, 0, win1251Text.c_str(),
                                         (int)win1251Text.length(), NULL, 0);
    // Выделяем буфер
    std::wstring unicodeText(sizeNeeded, 0);

    // Выполняем конвертацию
    MultiByteToWideChar(1251, 0, win1251Text.c_str(),
                        (int)win1251Text.length(),
                        &unicodeText[0], sizeNeeded);
    return unicodeText;
}

// Загрузка файла частями (для очень больших файлов)
bool LoadLargeTextFileToRichEdit(HWND hRichEdit, const wchar_t* filePath)
{
    HANDLE hFile = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        g_dictionaryText.clear();  // Очищаем глобальную переменную
        return false;
    }

    // Получаем размер файла
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == 0)
    {
        CloseHandle(hFile);
        SetWindowTextW(hRichEdit, L"");
        g_dictionaryText.clear();
        return true;
    }

    // Выделяем буфер для чтения
    std::vector<char> buffer(fileSize + 1);
    DWORD bytesRead = 0;

    if (!ReadFile(hFile, buffer.data(), fileSize, &bytesRead, NULL) || bytesRead == 0)
    {
        CloseHandle(hFile);
        g_dictionaryText.clear();
        return false;
    }
    buffer[bytesRead] = '\0';
    CloseHandle(hFile);

    // Конвертируем и сохраняем в ГЛОБАЛЬНУЮ переменную
    g_dictionaryText = ConvertWin1251ToUnicode(std::string(buffer.data(), bytesRead));

    // Выводим в RichEdit
    SendMessageW(hRichEdit, WM_SETTEXT, 0, (LPARAM)g_dictionaryText.c_str());

    return true;
}

// Разбиваем глобальный текст на строки
void SplitDictionaryToLines()
{
    g_dictionaryLines.clear();

    if (g_dictionaryText.empty())
    {
        MessageBoxW(NULL, L"g_dictionaryText пуст!", L"Ошибка", MB_OK);
        return;
    }

    std::wstringstream ss(g_dictionaryText);
    std::wstring line;

    while (std::getline(ss, line))
    {
        // Удаляем символ '\r' в конце если есть
        if (!line.empty() && line.back() == L'\r')
            line.pop_back();

        g_dictionaryLines.push_back(line);
    }
}

// Функция загрузки словаря с отображением статуса
void LoadDictionary(HWND hRichEdit, const std::wstring& filePath)
{
    if (LoadLargeTextFileToRichEdit(hRichEdit, filePath.c_str()))
    {
        // Успешно загрузили - теперь РАЗБИВАЕМ НА СТРОКИ
        SplitDictionaryToLines();
    }
    else
    {
        // Не удалось загрузить - выводим сообщение об ошибке
        SetWindowTextW(hRichEdit, L"положи «0 Lpo.txt» в папку с программой");
        g_dictionaryLines.clear();  // Очищаем на всякий случай
    }
}

// Функция определения, является ли символ частью слова (буква, цифра, подчеркивание)
bool IsWordCharacter(wchar_t ch)
{
    // Буквы (включая кириллицу), цифры и подчеркивание
    return (ch >= L'A' && ch <= L'Z') ||
           (ch >= L'a' && ch <= L'z') ||
           (ch >= L'А' && ch <= L'я') ||  // Кириллица
           ch == L'ё' || ch == L'Ё' ||
           (ch >= L'0' && ch <= L'9') ||
           ch == L'_';
}

// Функция проверки, является ли слово точным совпадением в строке
bool IsExactWordInString(const std::wstring& text, const std::wstring& word)
{
    if (word.empty() || text.length() < word.length())
        return false;

    size_t pos = 0;
    while ((pos = text.find(word, pos)) != std::wstring::npos)
    {
        // Проверяем, что перед словом - граница (начало строки или не буква/цифра)
        bool leftBoundary = (pos == 0) || !IsWordCharacter(text[pos - 1]);

        // Проверяем, что после слова - граница (конец строки или не буква/цифра)
        bool rightBoundary = (pos + word.length() == text.length()) ||
                             !IsWordCharacter(text[pos + word.length()]);

        if (leftBoundary && rightBoundary){
            return true;  // Нашли точное слово
        }

        pos += word.length();  // Продолжаем поиск
    }

    return false;
}

//
//
//

//// Функция для поиска строк, содержащих точное слово целиком
//std::vector<std::wstring> FindLinesWithExactWord(const std::wstring& searchWord)
//{
//    std::vector<std::wstring> foundLines;
//
//    if (searchWord.empty() || g_dictionaryLines.empty())
//        return foundLines;
//
//    for (const auto& line : g_dictionaryLines)
//    {
//        // Ищем слово в строке (как отдельное слово, а не часть слова)
//        if (IsExactWordInString(line, searchWord))
//        {
//            foundLines.push_back(line);
//        }
//    }
//    return foundLines;
//}
//
////для нечётких совпадений:
//std::vector<std::wstring> FindLinesWithPartialMatch(const std::wstring& searchWord)
//{
//    std::vector<std::wstring> foundLines;
//
//    if (searchWord.empty() || g_dictionaryLines.empty())
//        return foundLines;
//
//    for (const auto& line : g_dictionaryLines)
//    {
//        // Простой поиск подстроки (регистрозависимый)
//        if (line.find(searchWord) != std::wstring::npos)
//        {
//            foundLines.push_back(line);
//        }
//    }
//
//    return foundLines;
//}

//
//
//

// Вспомогательная функция для регистронезависимого поиска подстроки
bool ContainsSubstringIgnoreCase(const std::wstring& text, const std::wstring& search)
{
    auto it = std::search(
        text.begin(), text.end(),
        search.begin(), search.end(),
        [](wchar_t ch1, wchar_t ch2) {
            return std::towlower(ch1) == std::towlower(ch2);
        }
    );
    return it != text.end();
}

// Вспомогательная функция для регистронезависимой проверки целого слова
bool IsExactWordIgnoreCase(const std::wstring& line, const std::wstring& word)
{
    size_t pos = 0;
    while (true)
    {
        // Ищем слово без учёта регистра
        auto it = std::search(
            line.begin() + pos, line.end(),
            word.begin(), word.end(),
            [](wchar_t ch1, wchar_t ch2) {
                return std::towlower(ch1) == std::towlower(ch2);
            }
        );

        if (it == line.end())
            return false;

        pos = it - line.begin();
        size_t endPos = pos + word.length();

        // Проверяем границы слова
        bool leftOk = (pos == 0 || !std::iswalnum(line[pos - 1]));
        bool rightOk = (endPos == line.length() || !std::iswalnum(line[endPos]));

        if (leftOk && rightOk)
            return true;

        pos++; // Продолжаем поиск
    }
}

// Функция для поиска строк, содержащих точное слово целиком
std::vector<std::wstring> FindLinesWithExactWord(const std::wstring& searchWord, bool caseSensitive = true)
{
    std::vector<std::wstring> foundLines;

    if (searchWord.empty() || g_dictionaryLines.empty())
        return foundLines;

    if (caseSensitive)
    {
        for (const auto& line : g_dictionaryLines)
        {
            if (IsExactWordInString(line, searchWord))
            {
                foundLines.push_back(line);
            }
        }
    }
    else
    {
        for (const auto& line : g_dictionaryLines)
        {
            if (IsExactWordIgnoreCase(line, searchWord))
            {
                foundLines.push_back(line);
            }
        }
    }

    return foundLines;
}

// Функция для нечётких совпадений
std::vector<std::wstring> FindLinesWithPartialMatch(const std::wstring& searchWord, bool caseSensitive = true)
{
    std::vector<std::wstring> foundLines;

    if (searchWord.empty() || g_dictionaryLines.empty())
        return foundLines;

    if (caseSensitive)
    {
        for (const auto& line : g_dictionaryLines)
        {
            if (line.find(searchWord) != std::wstring::npos)
            {
                foundLines.push_back(line);
            }
        }
    }
    else
    {
        for (const auto& line : g_dictionaryLines)
        {
            if (ContainsSubstringIgnoreCase(line, searchWord))
            {
                foundLines.push_back(line);
            }
        }
    }

    return foundLines;
}

//std::vector<std::wstring> FindLinesWithExactWordCached(const std::wstring& searchWord)
//{
//    // Проверяем кеш
//    auto it = g_searchCache.find(searchWord);
//    if (it != g_searchCache.end())
//    {
//        return it->second;  // Возвращаем из кеша
//    }
//
//    // Выполняем поиск
//    std::vector<std::wstring> foundLines;
//
//    if (searchWord.empty() || g_dictionaryLines.empty())
//        return foundLines;
//
//    for (const auto& line : g_dictionaryLines)
//    {
//        if (IsExactWordInString(line, searchWord))
//        {
//            foundLines.push_back(line);
//        }
//    }
//
//    // Сохраняем в кеш (ограничиваем размер кеша 100 элементами)
//    if (g_searchCache.size() > 100)
//    {
//        g_searchCache.clear();  // Простая стратегия очистки
//    }
//    g_searchCache[searchWord] = foundLines;
//
//    return foundLines;
//}

// Альтернативная версия: поиск слова без учета регистра
bool IsExactWordInStringCaseInsensitive(const std::wstring& text, const std::wstring& word)
{
    std::wstring textLower = text;
    std::wstring wordLower = word;

    std::transform(textLower.begin(), textLower.end(), textLower.begin(), ::towlower);
    std::transform(wordLower.begin(), wordLower.end(), wordLower.begin(), ::towlower);

    return IsExactWordInString(textLower, wordLower);
}

// Обновление поля вывода на основе поиска
void UpdateRichEditWithSearchResults(const std::wstring& searchWord){
    if (!g_hRichEdit) return;

    if (searchWord.empty())
    {
        // Если поле поиска пустое, показываем весь словарь
        SetWindowTextW(g_hRichEdit, g_dictionaryText.c_str());
        return;
    }

    // Ищем строки с точным совпадением слова
    // «xrb_sns» = флаг «чувствителен к регистру»
    std::vector<std::wstring> foundLines = FindLinesWithExactWord(searchWord, xrb_sns);

    //«oi_fh_vd» = флаг «только целые слова»
    if (foundLines.empty() && !oi_fh_vd){  //только если не нашли - ищем частичные совпадения
        foundLines = FindLinesWithPartialMatch(searchWord, xrb_sns);
    }

    if (foundLines.empty())  //если и так не нашли:
    {
        SetWindowTextW(g_hRichEdit, L"ny maq i");  //сообщение: «не найдено совпадений»

        if (g_cv != 'r')   //если не был красным - сделать красным
        {
            g_cv = 'r';
            CHARFORMAT cf;
            ZeroMemory(&cf, sizeof(cf));
            cf.cbSize = sizeof(cf);
            cf.dwMask = CFM_COLOR;
            cf.crTextColor = RGB(NwaRd, 0, 0);
            SendMessage(g_hRichEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
        }
        return;
    }
    else{
        if (g_cv == 'r'){  //если был красным - сделай серым
            g_cv = 0;
            CHARFORMAT cf;
            ZeroMemory(&cf, sizeof(cf));
            cf.cbSize = sizeof(cf);
            cf.dwMask = CFM_COLOR;
            cf.crTextColor = RGB(NwaGh, NwaGh, NwaGh);
            SendMessage(g_hRichEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
        }
    }

    // Формируем результирующий текст
    std::wstring result;
    for (size_t i = 0; i < foundLines.size(); ++i)
    {
        result += foundLines[i];
        if (i != foundLines.size() - 1)
        {
            result += L"\r\n";  // Добавляем перенос строки (Windows стиль)
        }
    }

    // Выводим результат
    SetWindowTextW(g_hRichEdit, result.c_str());
}

// Обновленная функция с кешированием
//void UpdateRichEditWithSearchResultsCached(const std::wstring& searchWord)
//{
//    if (!g_hRichEdit) return;
//
//    if (searchWord.empty())
//    {
//        SetWindowTextW(g_hRichEdit, g_dictionaryText.c_str());
//        return;
//    }
//
//    std::vector<std::wstring> foundLines = FindLinesWithExactWordCached(searchWord);
//
//    if (foundLines.empty())
//    {
//        SetWindowTextW(g_hRichEdit, L"Совпадений не найдено");
//        return;
//    }
//
//    std::wstring result;
//    for (size_t i = 0; i < foundLines.size(); ++i)
//    {
//        result += foundLines[i];
//        if (i != foundLines.size() - 1)
//            result += L"\r\n";
//    }
//
//    SetWindowTextW(g_hRichEdit, result.c_str());
//}

void ToggleTopMost(HWND hWnd)
{
    // 1. Проверяем, является ли окно уже "поверх всех"
    LONG exStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
    BOOL isTopmost = (exStyle & WS_EX_TOPMOST) != 0;

    // 2. Устанавливаем новый режим
    // Если окно было "поверх всех" (isTopmost = TRUE), то убираем этот статус (HWND_NOTOPMOST)
    // Если не было (isTopmost = FALSE), то включаем (HWND_TOPMOST)
    SetWindowPos(hWnd, isTopmost ? HWND_NOTOPMOST : HWND_TOPMOST,
                 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    // 3. Обновляем галочку в меню для визуальной обратной связи
    HMENU hMainMenu = GetMenu(hWnd);
    if (hMainMenu != NULL)
    {
        // Получаем меню "Настройки" (оно первое, индекс 0)
        HMENU hSettingsMenu = GetSubMenu(hMainMenu, 0);
        // Ставим или убираем галочку (MF_CHECKED / MF_UNCHECKED)
        CheckMenuItem(hSettingsMenu, IDM_TOGGLE_ONTOP,
                      isTopmost ? MF_UNCHECKED : MF_CHECKED);
    }
}


// ============================================================

// ============================================================
// # ТОЧКА ВХОДА: WinMain (стартовая настройка окна)
int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nCmdShow
)
{
    WNDCLASSEX wcex;

    // ---------------------------------------------------
    // -- НАСТРОЙКА КЛАССА ОКНА (меняем редко, но можно)

    //wndClassEx.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;                // связь с обработчиком сообщений
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    //wcex.hIcon          = LoadIcon(wcex.hInstance, "Lpo.ico"); // <<< СЛОТ: своя иконка
    wcex.hIcon   = (HICON)LoadImage(NULL, "Lpo.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);               // <<< СЛОТ: вид курсора
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);                  // <<< СЛОТ: цвет фона (или CreateSolidBrush)
    wcex.lpszMenuName   = NULL;         // <<< СЛОТ: если есть меню -> "IDR_MYMENU"
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, _T("aaaa")); // <<< СЛОТ: малая иконка

    if (!RegisterClassEx(&wcex))   //регистрируем класс
    {
        MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T("Error"), 0);
        return 1;
    }

    hInst = hInstance; // сохраняем глобально

    // -- СОЗДАНИЕ ОКНА (здесь много слотов)
    HWND hWnd = CreateWindowEx(
                    WS_EX_OVERLAPPEDWINDOW,
                    szWindowClass,
                    szTitle,
                    WS_OVERLAPPEDWINDOW,
                    CW_USEDEFAULT, CW_USEDEFAULT,
                    OknC, OknH,
                    NULL,
                    NULL,
                    hInstance,
                    NULL
                );

    if (!hWnd)
    {
        MessageBox(NULL, _T("Call to CreateWindowEx failed!"), _T("Error"), 0);
        return 1;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // ============================================================
    // # СЛОТ 2: ДОПОЛНИТЕЛЬНЫЕ ДЕЙСТВИЯ ПОСЛЕ ПОКАЗА ОКНА
    // -- Например:
    // SetTimer(hWnd, 1, 1000, NULL);   // таймер каждую секунду
    // PlaySound(...);
    // ============================================================

    // -- ГЛАВНЫЙ ЦИКЛ СООБЩЕНИЙ (не трогаем)
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

// ============================================================
// ОКОННАЯ ПРОЦЕДУРА: ОБРАБОТКА СООБЩЕНИЙ (WndProc)
// ============================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    // Пример локального текста (можно вынести в глобальные)
    //TCHAR greeting[] = _T("Hello, Windows desktop!");

    switch (message)
    {
    // ========================================================
    // <<< СЛОТ 3: WM_CREATE (инициализация: создаем дочерние окна, кнопки, таймеры) >>>
    // ========================================================
    case WM_CREATE:
    {
        // 1. Загружаем библиотеку Rich Edit
        LoadLibrary(_T("riched32.dll"));

        // 2. Создаем поле ввода (однострочное, с рамкой)
        g_hEditInput = CreateWindow(
                           RICHEDIT_CLASS,              // класс - стандартное поле ввода
                           _T(""),                  // начальный текст
                           WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT,
                           0, 0,                    // x, y
                           100, 25,                 // временные размеры, задаётся в WM_SIZE
                           hWnd,                    // родительское окно
                           (HMENU)ID_EDIT_INPUT,    // идентификатор
                           hInst,
                           NULL
                       );
        // Устанавливаем лимит в 100 символов
        SendMessage(g_hEditInput, EM_LIMITTEXT, EdtGliLm, 0);
      // Устанавливаем подкласс для запрета стилей из буфера
       // Устанавливаем подкласс для поля ввода
       g_oldEditProc = (WNDPROC)SetWindowLongPtr(
           g_hEditInput,
           GWLP_WNDPROC,
           (LONG_PTR)EditSubclassProc
       );

        // 3. Создаем поле вывода с форматированием
        g_hRichEdit = CreateWindow(
                          RICHEDIT_CLASS,          // класс Rich Edit
                          _T(""),                  // начальный текст
                          WS_VISIBLE | WS_CHILD | WS_BORDER |
                          ES_MULTILINE |           // многострочный
                          ES_READONLY |            // только для чтения
                          WS_VSCROLL |             // вертикальный скролл
                          WS_HSCROLL |             // горизонтальный скролл
                          ES_AUTOVSCROLL |         // авто-вертикальный скролл
                          ES_AUTOHSCROLL,          // авто-горизонтальный скролл
                          0, 0, 100, 100,  // задаётся в WM_SIZE
                          hWnd,
                          (HMENU)ID_RICHEDIT_OUTPUT,
                          hInst,
                          NULL
                      );
         // Устанавливаем ТОТ ЖЕ САМЫЙ подкласс для поля вывода
          // (нужно сохранить другую глобальную переменную)
          g_oldOutputProc = (WNDPROC)SetWindowLongPtr(
              g_hRichEdit,
              GWLP_WNDPROC,
              (LONG_PTR)EditSubclassProc  // Та же функция!
          );

        // ЗАГРУЖАЕМ СЛОВАРЬ СРАЗУ ПОСЛЕ СОЗДАНИЯ
        LoadDictionary(g_hRichEdit, g_dictPath);

        // Создаем кнопку "X" рядом с полем ввода
        g_hButtonClear = CreateWindow(
                             _T("BUTTON"),                 // класс кнопки
                             _T("x"),
                             WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                             215, 10,                   // позиция, задаётся в WM_SIZE
                             25, 25,                   // размер кнопки
                             hWnd,
                             (HMENU)ID_BUTTON_CLEAR,   // идентификатор кнопки
                             hInst,
                             NULL
                         );

        // Устанавливаем начальное форматирование
        CHARFORMAT cf;
        ZeroMemory(&cf, sizeof(cf));
        cf.cbSize = sizeof(cf);
        cf.dwMask = CFM_COLOR | CFM_FACE | CFM_BOLD;
        cf.crTextColor = RGB(NwaGh, NwaGh, NwaGh);
        lstrcpy(cf.szFaceName, _T("An"));  //шрифт (мой кастомный)
        cf.dwEffects = 0;  // Явно убираем жирное начертание

        SendMessage(g_hEditInput, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
        SendMessage(g_hRichEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);

        // Чёрный фон
        SendMessage(g_hEditInput, EM_SETBKGNDCOLOR, 0, RGB(0, 0, 0));
        SendMessage(g_hRichEdit, EM_SETBKGNDCOLOR, 0, RGB(0, 0, 0));

        // --- Создание меню ---
        // 1. Создаем главную строку меню
        HMENU hMainMenu = CreateMenu();
        // 2. Создаем выпадающее меню (пункт "Настройки")
        HMENU hSettingsMenu = CreatePopupMenu();

        // 3. Добавляем пункт в выпадающее меню
        // IDM_TOGGLE_ONTOP - это уникальный ID, который мы определим ниже
        AppendMenuW(hSettingsMenu, MF_STRING, IDM_TOGGLE_ONTOP, L"uv ol okn");

        //только фул слова
        AppendMenuW(hSettingsMenu, MF_STRING, IDM_TOGGLE_FUL, L"oi fh vd");

        //чувствительность регистра
        AppendMenuW(hSettingsMenu, MF_STRING, IDM_XRB_SNS, L"xrb sns");

        // 4. Добавляем выпадающее меню в главную строку
        AppendMenuW(hMainMenu, MF_POPUP, (UINT_PTR)hSettingsMenu, L"st");

        AppendMenuW(hSettingsMenu, MF_SEPARATOR, 0, NULL);  // Разделитель (необязательно)
         AppendMenuW(hSettingsMenu, MF_STRING, IDM_RELOAD_DICT, L"rld vok");

        // 5. Привязываем готовое меню к окну
        SetMenu(hWnd, hMainMenu);

        ToggleTopMost(hWnd);

        return 0;
    }

    // ========================================================
    // <<< СЛОТ 4: WM_COMMAND (реакция на кнопки, меню, акселераторы) >>>
    // ========================================================
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);

        // Если сообщение от поля ввода и оно изменилось
        if (wmId == ID_EDIT_INPUT && wmEvent == EN_UPDATE)
        {
            // Получаем текст из поля ввода (Unicode версия)
            wchar_t buffer[256];
            GetWindowTextW(g_hEditInput, buffer, 256);

            std::wstring searchWord(buffer);

            // Удаляем пробелы в начале и конце
            size_t start = searchWord.find_first_not_of(L" \t\n\r");
            size_t end = searchWord.find_last_not_of(L" \t\n\r");

            if (start != std::wstring::npos && end != std::wstring::npos)
            {
                searchWord = searchWord.substr(start, end - start + 1);
            }
            else if (start == std::wstring::npos)
            {
                searchWord.clear();  // Строка состоит только из пробелов
            }

            // Обновляем результаты поиска
            UpdateRichEditWithSearchResults(searchWord);
            sikVd = searchWord; //записываем в глобал, для обновления поиска после настроек

            return 0;
        }

        // Обработка нажатия кнопки "X"
        if (wmId == ID_BUTTON_CLEAR)
        {
            SetWindowTextW(g_hEditInput, L"");
            SetFocus(g_hEditInput);  // Возвращаем фокус в поле ввода
            UpdateRichEditWithSearchResults(L"");  // Показываем весь словарь
            return 0;
        }

        // обработка пункта меню
        if (wmId == IDM_TOGGLE_ONTOP)
        {
            ToggleTopMost(hWnd);
            return 0;
        }
        if (wmId == IDM_TOGGLE_FUL)
        {
            HMENU hMainMenu = GetMenu(hWnd);
            if (hMainMenu != NULL)
            {
                // Получаем меню "Настройки"
                HMENU hSettingsMenu = GetSubMenu(hMainMenu, 0);
                // Ставим или убираем галочку (MF_CHECKED / MF_UNCHECKED)
                CheckMenuItem(hSettingsMenu, IDM_TOGGLE_FUL,
                              oi_fh_vd ? MF_UNCHECKED : MF_CHECKED);
            }
            oi_fh_vd = !oi_fh_vd;
            UpdateRichEditWithSearchResults(sikVd);
            return 0;
        }
        if (wmId == IDM_XRB_SNS){
            HMENU hMainMenu = GetMenu(hWnd);
            if (hMainMenu != NULL)
            {
                // Получаем меню "Настройки"
                HMENU hSettingsMenu = GetSubMenu(hMainMenu, 0);
                // Ставим или убираем галочку (MF_CHECKED / MF_UNCHECKED)
                CheckMenuItem(hSettingsMenu, IDM_XRB_SNS,
                              xrb_sns ? MF_UNCHECKED : MF_CHECKED);
            }
            xrb_sns = !xrb_sns;
            UpdateRichEditWithSearchResults(sikVd);
        }

       if (wmId == IDM_RELOAD_DICT)  // Перезагружаем словарь
       {
           LoadDictionary(g_hRichEdit, g_dictPath);
           return 0;
       }
        break;
    }

    // ========================================================
    // <<< СЛОТ 5: WM_TIMER (обработка таймеров) >>>
    // ========================================================
    case WM_TIMER:
        // if (wParam == 1) { /* каждую секунду */ }
        // InvalidateRect(hWnd, NULL, TRUE); // перерисовать
        break;

    // ========================================================
    // <<< СЛОТ 6: WM_LBUTTONDOWN / WM_KEYDOWN (мышь/клавиатура) >>>
    // ========================================================
    case WM_LBUTTONDOWN:
    {
        // int x = LOWORD(lParam);
        // int y = HIWORD(lParam);
        // SetWindowText(hWnd, _T("Клик!"));
        break;
    }

    // ========================================================
    // <<< СЛОТ 7: WM_SIZE (изменение размеров окна) >>>
    // ========================================================
    case WM_SIZE:
    {
        // Получаем новую ширину и высоту окна
        OknC = LOWORD(lParam);
        OknH = HIWORD(lParam);
        EdtH = EdtHFnk();

        // Перемещаем и изменяем размеры поля ввода (высота 30 пикселей, по ширине окна)
        if (g_hEditInput != NULL)
        {
            SetWindowPos(g_hEditInput, NULL,
                         0, 0,                    // x, y
                         OknC-EdtH, EdtH,       // ширина, высота
                         SWP_NOZORDER);
        }

        // Перемещаем и изменяем поле вывода (ниже поля ввода на всю оставшуюся область)
        if (g_hRichEdit != NULL)
        {
            SetWindowPos(g_hRichEdit, NULL,
                         0, EdtH,                   // x, y (под полем ввода)
                         OknC,           // ширина = ширина окна
                         OknH-EdtH,     // высота = всё оставшееся
                         SWP_NOZORDER);
        }
        if (g_hButtonClear != NULL)
        {
            SetWindowPos(g_hButtonClear, NULL,
                         OknC-EdtH, 0,                   // x, y (под полем ввода)
                         EdtH,           // ширина
                         EdtH,     // высота
                         SWP_NOZORDER);
        }

        // Рассчитываем размер шрифта в зависимости от высоты окна
        LONG newYHeight = (OknH / EdtHMin) * 20;  // примерно, нужны корректировки
        if (newYHeight < TxtSzMin) newYHeight = TxtSzMin;

        CHARFORMAT cf = {0};
        cf.cbSize = sizeof(cf);
        cf.dwMask = CFM_SIZE;
        cf.yHeight = newYHeight;

        SendMessage(g_hRichEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
        SendMessage(g_hEditInput, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);

        return 0;
    }
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* pMMI = (MINMAXINFO*)lParam;

        //  МИНИМАЛЬНЫЙ размер
        pMMI->ptMinTrackSize.x = OknCMin;
        pMMI->ptMinTrackSize.y = OknHMin;

        return 0;
    }

    // ========================================================
    // <<< СЛОТ 8: WM_PAINT (ВСЯ ГРАФИКА И ТЕКСТ) >>>
    // ========================================================

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);

        // ----- ВСТАВЛЯЙТЕ СЮДА ВСЮ РИСОВАЛКУ -----
        TextOutW(hdc, 5, 5, L"", 0); //заглушка от ворнинга: ««hdc» не используется»

        // Примеры:
        // Rectangle(hdc, 10, 30, 200, 100);
        // Ellipse(hdc, 250, 30, 350, 130);
        // SetBkMode(hdc, TRANSPARENT);
        // TextOut(hdc, 20, 150, _T("Динамический текст"), 18);
        // ----- КОНЕЦ БЛОКА РИСОВАНИЯ -----

        EndPaint(hWnd, &ps);
        break;

    // ========================================================
    // <<< СЛОТ 9: WM_DESTROY (чистка, таймеры, кисти) >>>
    // ========================================================
    case WM_DESTROY:
        //FreeLibrary(GetModuleHandle(_T("riched32.dll") ));  //не нужно
        PostQuitMessage(0);
        break;

    // ========================================================
    // <<< СЛОТ 10: ПОЛЬЗОВАТЕЛЬСКИЕ СООБЩЕНИЯ (WM_USER + x) >>>
    // ========================================================
    // case WM_USER + 100:
    //    // обновить что-то
    //    break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
