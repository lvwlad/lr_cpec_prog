#ifndef MAGMA_H
#define MAGMA_H

#include <QByteArray>
#include <vector>

class Magma {
public:
    Magma();
    void setKey(const QByteArray &key);
    QByteArray encrypt(const QByteArray &block);
    QByteArray decrypt(const QByteArray &block);

private:
    std::vector<uint32_t> subkeys;
    static const uint8_t sboxes[8][16];
    uint32_t G(uint32_t a, uint32_t k);
    uint32_t t(uint32_t a);
};

#endif // MAGMA_H
