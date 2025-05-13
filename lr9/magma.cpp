#include "magma.h"
#include <QDebug>

const uint8_t Magma::sboxes[8][16] = {
    {12, 4, 6, 2, 10, 5, 11, 9, 14, 8, 13, 7, 0, 3, 15, 1},
    {6, 8, 2, 3, 9, 10, 5, 12, 1, 14, 4, 7, 11, 13, 0, 15},
    {11, 3, 5, 8, 2, 15, 10, 13, 14, 1, 7, 4, 12, 9, 6, 0},
    {12, 8, 2, 1, 13, 4, 15, 6, 7, 0, 10, 5, 3, 14, 9, 11},
    {7, 15, 5, 10, 8, 1, 6, 13, 0, 9, 3, 14, 11, 4, 2, 12},
    {5, 13, 15, 6, 9, 2, 12, 10, 11, 7, 8, 1, 4, 3, 14, 0},
    {8, 14, 2, 5, 6, 9, 1, 12, 15, 4, 11, 13, 13, 10, 7, 0},
    {1, 7, 14, 13, 0, 5, 8, 3, 4, 15, 10, 6, 9, 12, 11, 2}
};

Magma::Magma() {}

void Magma::setKey(const QByteArray &key)
{
    if (key.size() != 32) {
        qWarning() << "Invalid key size:" << key.size();
        return;
    }
    subkeys.resize(32);
    for (int i = 0; i < 8; i++) {
        uint32_t k = 0;
        for (int j = 0; j < 4; j++) {
            k |= static_cast<uint32_t>(static_cast<uint8_t>(key[i*4 + j])) << ((3-j)*8);
        }
        subkeys[i] = k;
    }
    // Первые 24 раунда: ключи K1..K8 повторяются 3 раза
    for (int i = 0; i < 24; i++) {
        subkeys[i] = subkeys[i % 8];
    }
    // Последние 8 раундов: ключи K8..K1
    for (int i = 24; i < 32; i++) {
        subkeys[i] = subkeys[31 - i];
    }
}

uint32_t Magma::t(uint32_t a)
{
    uint32_t result = 0;
    for (int i = 0; i < 8; i++) {
        uint8_t nibble = (a >> (i*4)) & 0xF;
        uint8_t sbox_val = sboxes[7-i][nibble];
        result |= static_cast<uint32_t>(sbox_val) << (i*4);
    }
    return result;
}

uint32_t Magma::G(uint32_t a, uint32_t k)
{
    uint32_t temp = (a + k) & 0xFFFFFFFF; // Модуль 2^32
    temp = t(temp);
    temp = (temp << 11) | (temp >> 21);
    return temp;
}

QByteArray Magma::encrypt(const QByteArray &block)
{
    if (block.size() != 8) {
        qWarning() << "Invalid block size for encryption:" << block.size();
        return QByteArray();
    }

    // Разбиваем блок на две 32-битные части (big-endian)
    uint32_t left = 0, right = 0;
    for (int i = 0; i < 4; i++) {
        left |= static_cast<uint32_t>(static_cast<uint8_t>(block[i])) << ((3-i)*8);
        right |= static_cast<uint32_t>(static_cast<uint8_t>(block[i+4])) << ((3-i)*8);
    }

    // 32 раунда шифрования
    for (int i = 0; i < 32; i++) {
        uint32_t temp = right;
        right = left ^ G(right, subkeys[i]);
        left = temp;
    }

    // Формируем выходной блок
    QByteArray result(8, 0);
    for (int i = 0; i < 4; i++) {
        result[i] = static_cast<char>((right >> ((3-i)*8)) & 0xFF);
        result[i+4] = static_cast<char>((left >> ((3-i)*8)) & 0xFF);
    }
    return result;
}

QByteArray Magma::decrypt(const QByteArray &block)
{
    if (block.size() != 8) {
        qWarning() << "Invalid block size for decryption:" << block.size();
        return QByteArray();
    }

    // Разбиваем блок на две 32-битные части (big-endian)
    uint32_t left = 0, right = 0;
    for (int i = 0; i < 4; i++) {
        left |= static_cast<uint32_t>(static_cast<uint8_t>(block[i])) << ((3-i)*8);
        right |= static_cast<uint32_t>(static_cast<uint8_t>(block[i+4])) << ((3-i)*8);
    }

    // 32 раунда дешифрования (обратный порядок ключей)
    for (int i = 0; i < 32; i++) {
        uint32_t temp = right;
        right = left ^ G(right, subkeys[31-i]);
        left = temp;
    }

    // Формируем выходной блок
    QByteArray result(8, 0);
    for (int i = 0; i < 4; i++) {
        result[i] = static_cast<char>((right >> ((3-i)*8)) & 0xFF);
        result[i+4] = static_cast<char>((left >> ((3-i)*8)) & 0xFF);
    }
    return result;
}
