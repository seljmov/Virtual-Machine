#include "RMath.h"

void RMath::operator()(Processor &processor) {
    // - Узнает формат операндов
    const uint8_t dd = processor.cmd.dd;
    // - Обрабатываем команду в зависимости от формата
    switch (dd) {
        // - Регистр-Регистр (dd = 00/0)
        case 0: handle_reg_to_reg(processor); break;
        // - Регистр-Память (dd = 01/1)
        case 1: handle_reg_to_mem(processor); break;
        // - Память-Регистр (dd = 10/2)
        case 2: handle_mem_to_reg(processor); break;
        // - Память-Память (dd = 11/3)
        case 3: handle_mem_to_mem(processor); break;
    }
    // - Устанавливаем флаги
    set_flags(processor);
}

void RMath::set_flags(Processor &processor) noexcept {
    // - Узнаем, где лежит ответ: в регистре или в памяти
    const uint8_t dd = processor.cmd.dd;
    int32_t result;
    // - Если в регистре
    if (dd == 0 || dd == 2)
    {
        const uint8_t r2_i = processor.cmd.r2;
        result = get_real32(processor, r2_i);
    }
    // - Если в памяти
    else
    {
        const uint8_t o2_i = processor.cmd.o2;
        result = processor.memory[o2_i].word.word32.real32;
    }
    // - Устанавливаем флаги
    processor.psw.set_ZF(result);
    processor.psw.set_SF(result);
}

void RMath::handle_reg_to_reg(Processor &processor) noexcept {
    // - Узнаем индексы регистров
    const uint8_t r1_i = processor.cmd.r1;
    const uint8_t r2_i = processor.cmd.r2;
    // - Определяем переменные
    auto var1 = get_real32(processor, r1_i);
    auto var2 = get_real32(processor, r2_i);
    // - Вычисляем
    float real32 = execute(var1, var2);
    // - Записываем результат
    set_real32(processor, real32, r2_i);
}

void RMath::handle_reg_to_mem(Processor &processor) noexcept {
    // - Узнаем индекс регистра и адрес памяти
    const uint8_t r1_i = processor.cmd.r1;
    const address_t o2_i = processor.cmd.o2;
    // - Потому что сохраняем в память
    data_t new_data;
    // - Берем требуемые данные из памяти
    data_t from_mem = processor.memory[o2_i];
    // - Определяем переменные
    auto var1 = get_real32(processor, r1_i);
    auto var2 = from_mem.word.word32.real32;
    // - Вычисляем
    new_data.word.word32.real32 = execute(var1, var2);
    // - Отправляем результат по адресу
    processor.memory[o2_i] = new_data;
}

void RMath::handle_mem_to_reg(Processor &processor) noexcept {
    // - Узнаем индекс регистра и адрес памяти
    const address_t o1_i = processor.cmd.o1;
    const uint8_t r2_i = processor.cmd.r2;
    // - Берем требуемые данные из памяти
    data_t from_mem = processor.memory[o1_i];
    // - Определяем переменные
    auto var1 = from_mem.word.word32.real32;
    auto var2 = get_real32(processor, r2_i);
    // - Вычисляем
    float real32 = execute(var1, var2);
    // - Записываем результат
    set_real32(processor, real32, r2_i);
}

void RMath::handle_mem_to_mem(Processor &processor) noexcept {
    // - Узнаем адреса памяти
    const address_t o1_i = processor.cmd.o1;
    const address_t o2_i = processor.cmd.o2;
    // - Потому что сохраняем в память
    data_t new_data;
    // - Берем требуемые данные из памяти
    data_t from_mem_1 = processor.memory[o1_i];
    data_t from_mem_2 = processor.memory[o2_i];
    // - Определяем переменные
    auto var1 = from_mem_1.word.word32.real32;
    auto var2 = from_mem_2.word.word32.real32;
    // - Вычисляем
    new_data.word.word32.real32 = execute(var1, var2);
    // - Отправляем результат по адресу
    processor.memory[o2_i] = new_data;
}