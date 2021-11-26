#ifndef i8080TYPE_H
#define i8080TYPE_H

#include <stdint.h>

struct _invaders;

typedef struct _i8080 {
    uint8_t a, b, c, d, e, h, l;
    uint8_t *registerDecode[8];

    uint8_t flag;

    #define setSign(X) (X)->flag |= 0x80
    #define setZero(X) (X)->flag |= 0x40
    #define setAC(X)   (X)->flag |= 0x10
    #define setParity(X) (X)->flag |= 0x04
    #define setCarry(X) (X)->flag |= 0x01

    #define clearSign(X) (X)->flag &= 0x7F
    #define clearZero(X) (X)->flag &= 0xBF
    #define clearAC(X)   (X)->flag &= 0xEF
    #define clearParity(X) (X)->flag &= 0xFB
    #define clearCarry(X) (X)->flag &= 0xFE

    #define checkSign(X) (((X)->flag & 0x80) >> 0x07)
    #define checkZero(X) (((X)->flag & 0x40) >> 0x06)
    #define checkAC(X)   (((X)->flag & 0x10) >> 0x04)
    #define checkParity(X) (((X)->flag & 0x04) >> 0x02)
    #define checkCarry(X) ((X)->flag & 0x01)

    uint16_t sp, pc;

    uint8_t (*readAddress)(struct _i8080 *cpu, uint16_t address);
    void (*writeAddress)(struct _i8080 *cpu, uint16_t address, uint8_t value);

    uint8_t (*readPort)(struct _i8080 *cpu, uint8_t port);
    void (*writePort)(struct _i8080 *cpu, uint8_t port);

    void (*opcodeTable[256])(struct _i8080 *cpu);
    uint8_t currentOpcode;

    uint8_t (*conditionalTable[4])(struct _i8080 *cpu);
    
    uint8_t halt;
    uint32_t cycles;

    uint8_t interruptEnable;
    uint8_t pendingInterrupt;

    struct _invaders *machine;
} i8080;

#endif
