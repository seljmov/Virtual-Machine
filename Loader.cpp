#include "Loader.h"

std::string Loader::get_path() noexcept {
    // - Получаю абсолютный путь
    std::string path = std::filesystem::current_path();
    // - Так как у меня Cmake, то исполняемый файл находится в папке
    // - cmake-build-debug, поэтому я обрезаю этот каталог из пути
    for (size_t i = path.size()-1; i >= 0; --i) {
        if (path[i] == '/') {
            path = path.substr(0, i);
            break;
        }
    }
    return path;
}

void Loader::upload(const std::string &filename, Processor &processor) {
    std::ifstream file;
    file.open(get_path() + filename);

    if (!file.is_open()) {
        throw invalid_path("Указан неверный путь к файлу!");
    }

    // - Базовый адрес. Нужен для того, чтобы
    // - в операндах адрес задавать относительно
    address_t base_address = 0;
    address_t address = 0;
    char symbol = ' ';

    while (symbol != 'e')
    {
        std::string read;
        // - Считываем строку
        getline(file, read);
        std::istringstream istring(read);
        // - Берем префикс и проверяем
        istring >> symbol;

        switch (symbol)
        {
            // - Указываем IP
            // - Формат: a <адрес>
            case 'a': read_address(istring, address); base_address = address; break;
            // - Формат: i <размер> <число>
            case 'i': read_integer(istring, processor, address); break;
            // - Формат: r <число>
            case 'r': read_real(istring, processor, address); break;
            // - Формат: c <opcode> <ss> <dd> <операнды>
            case 'c': read_command(istring, processor, address, base_address); break;
            // - Формат: e <адрес памяти>
            case 'e': read_last(istring, processor, address); break;
        }
    }

    file.clear();
    file.close();
}

bool Loader::is_register(const std::string &str) noexcept {
    // - Максимальный номер регистра 8, поэтому вид операнда: %%8
    // - Достаточно проверить, что первые два символа = %,
    // - а третий символ = цифра
    // - Проверка префикса
    bool prefix = ((str[0] == '%') && (str[1] == '%'));
    // - Проверка цифры
    bool digit = isdigit(str[2]);
    return (prefix && digit);
}

void Loader::read_address(std::istringstream &strm, address_t &addr) noexcept {
    strm >> addr;
}

void Loader::read_integer(std::istringstream &strm, Processor &proc, address_t &addr) noexcept {
    word_t word;
    uint8_t s;
    strm >> s;
    // - Если размер 16 бит
    if (s == 0) strm >> word.word16->int16;
    // - Иначе 32 бита
    else strm >> word.word32.int32;
    proc.memory[addr] = {word};
    ++addr;
}

void Loader::read_real(std::istringstream &strm, Processor &proc, address_t &addr) noexcept {
    word_t word;
    strm >> word.word32.real32;
    proc.memory[addr] = {word};
    ++addr;
}

void Loader::read_command(std::istringstream &strm, Processor &proc, address_t &addr, address_t b_addr) noexcept {
    cmd_t command;

    // - Получаем вектор строк, где каждый элемент = элемент команды
    std::vector<std::string> _cmd = split(strm.str(), ' ');
    // - Удаляем префикс
    _cmd.erase(_cmd.begin());
    // - Удаляем пробелы и комментарий, если он есть
    remove_trash(_cmd);

    command.code = uint8_t(std::stoi(_cmd[0]));
    command.s = uint8_t(std::stoi(_cmd[1]));
    command.dd = uint8_t(std::stoi(_cmd[2]));

    // - Проверка операндов
    // - %%1 - регистр
    // - %1 - память
    // - Первый операнд
    std::string f_operand = _cmd[3];
    // - Если регистр
    if (is_register(f_operand))
    {
        f_operand = f_operand.substr(2);
        command.r1 = uint8_t(std::stoi(f_operand));
    }
    // - Иначе память
    else
    {
        f_operand = f_operand.substr(1);
        command.o1 = b_addr + address_t (std::stoi(f_operand));
    }

    // - Второй операнд
    std::string s_operand = _cmd[4];
    // - Если регистр
    if (is_register(s_operand))
    {
        s_operand = s_operand.substr(2);
        command.r2 = uint8_t(std::stoi(s_operand));
    }
    // - Иначе память
    else
    {
        s_operand = s_operand.substr(1);
        command.o2 = b_addr + address_t (std::stoi(s_operand));
    }

    data_t to_mem;
    to_mem.cmd = command;
    proc.memory[addr] = to_mem;
    ++addr;
}

void Loader::read_last(std::istringstream &strm, Processor &proc, address_t &addr) noexcept {
    data_t data;
    data.cmd.code = 0;
    proc.memory[addr] = data;
    address_t IP;
    strm >> IP;
    proc.psw.IP = IP;
}

std::vector<std::string> Loader::split(const std::string &str, char delim) {
    std::vector<std::string> elems;
    std::stringstream string(str);
    std::string item;
    while (getline(string, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

void Loader::remove_trash(std::vector<std::string> &cmd) {
    // - Удаление комментария
    // - Тут int, потому что число может быть < 0
    for (int i = cmd.size()-1; i >= 0; --i) {
        if (cmd[i] == ";") {
            cmd.erase(cmd.begin()+i, cmd.end());
            break;
        }
    }

    // - Удаление пустот
    std::size_t i = 0;
    while (i < cmd.size())
    {
        if (cmd[i] == "") {
            cmd.erase(cmd.begin()+i);
            // - Тут continue, так как следующий за удаленным элемент,
            // - тоже может быть пустотой, поэтому не увеличиваем счетчик
            continue;
        }
        ++i;
    }
}