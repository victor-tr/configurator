#ifndef CRC_H
#define CRC_H

#include <QByteArray>

// Evaluate CRC for packet. "len" is length of the packet WITHOUT CRC byte.
inline quint8 evaluateCRC(const QByteArray &data, int len)
{
    quint8 crc = 0;
    bool last_bit;
    for (int i = 0; i < len; ++i) {
        last_bit = 0x80 & crc;
        crc = (crc << 1) | (last_bit ? 1 : 0);
        crc += (quint8)data.at(i);
    }

    return crc;
}

// Check CRC for packet. "len" is length of the packet WITH CRC byte.
inline bool checkCRC(const QByteArray &data, int len)
{
    return evaluateCRC(data, len - 1) == (quint8)data.at(len - 1);
}

#endif // CRC_H
