#pragma once
// https://stackoverflow.com/questions/105252/how-do-i-convert-between-big-endian-and-little-endian-values-in-c
template <typename T>
T SwapEndian(T u)
{
    static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

    union
    {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = u;

    for (size_t k = 0; k < sizeof(T); k++)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];

    return dest.u;
}

// https://stackoverflow.com/questions/11815894/how-to-read-write-arbitrary-bits-in-c-c
#define GETMASK(index, size) (((1 << (size)) - 1) << (index))
#define READFROM(data, index, size) (((data)&GETMASK((index), (size))) >> (index))
#define WRITETO(data, index, size, value) ((data) = ((data) & (~GETMASK((index), (size)))) | ((value) << (index)))