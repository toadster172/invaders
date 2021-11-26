/* Copyright (C) 2021 Alice Shelton <AliceAShelton@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include "i8080.h"
#include "opcode.h"

uint8_t testNZ(i8080 *cpu);
uint8_t testNC(i8080 *cpu);
uint8_t testPO(i8080 *cpu);
uint8_t testP(i8080 *cpu);

i8080 *create8080(void) {
    i8080 *cpu = malloc(sizeof(i8080));
    cpu->pc = 0x00;
    cpu->flag = 2; //Bit one is alway set
    cpu->cycles = 0;

    cpu->interruptEnable = 0;
    cpu->pendingInterrupt = 0;

    cpu->registerDecode[0] = &(cpu->b);
    cpu->registerDecode[1] = &(cpu->c);
    cpu->registerDecode[2] = &(cpu->d);
    cpu->registerDecode[3] = &(cpu->e);
    cpu->registerDecode[4] = &(cpu->h);
    cpu->registerDecode[5] = &(cpu->l);
    cpu->registerDecode[6] = NULL;      //Index 6 represents (hl)
    cpu->registerDecode[7] = &(cpu->a);

    cpu->conditionalTable[0] = testNZ;
    cpu->conditionalTable[1] = testNC;
    cpu->conditionalTable[2] = testPO;
    cpu->conditionalTable[3] = testP;

    cpu->halt = 0;

    for(uint16_t i = 0; i < 0x100; i++) {
        cpu->opcodeTable[i] = NULL;
    }

    for(uint8_t i = 0x00; i < 0x40; i+= 8) {
        cpu->opcodeTable[i] = nop;
    }

    for(uint8_t i = 0x01; i < 0x31; i += 16) {
        cpu->opcodeTable[i] = loadImmPair;
    }
    cpu->opcodeTable[0x31] = loadImmSP;

    cpu->opcodeTable[0x02] = storeAX;
    cpu->opcodeTable[0x12] = storeAX;
    cpu->opcodeTable[0x22] = storeHL;
    cpu->opcodeTable[0x32] = storeA;

    for(uint8_t i = 0x03; i < 0x30; i += 16) {
        cpu->opcodeTable[i] = incPair;
    }
    cpu->opcodeTable[0x33] = incSP;
    for(uint8_t i = 0x04; i < 0x40; i += 8) {
        cpu->opcodeTable[i] = incReg;
    }
    cpu->opcodeTable[0x34] = incMem;
    for(uint8_t i = 0x05; i < 0x40; i += 8) {
        cpu->opcodeTable[i] = decReg;
    }
    cpu->opcodeTable[0x35] = decMem;
    for(uint8_t i = 0x0B; i < 0x3B; i += 16) {
        cpu->opcodeTable[i] = decPair;
    }
    cpu->opcodeTable[0x3B] = decSP;
    for(uint8_t i = 0x09; i < 0x39; i += 16) {
        cpu->opcodeTable[i] = addPair;
    }
    cpu->opcodeTable[0x39] = addSP;
    cpu->opcodeTable[0x27] = decimalAdjust;
    cpu->opcodeTable[0x2F] = complementA;
    cpu->opcodeTable[0x37] = setC;
    cpu->opcodeTable[0x3F] = complementC;
    cpu->opcodeTable[0x0A] = loadAX;
    cpu->opcodeTable[0x1A] = loadAX;
    cpu->opcodeTable[0x2A] = loadHL;
    cpu->opcodeTable[0x3A] = loadA;

    for(uint8_t i = 0x06; i < 0x40; i += 8) {
        cpu->opcodeTable[i] = moveImmReg;
    }
    cpu->opcodeTable[0x36] = moveImmMem;
    cpu->opcodeTable[0x07] = rotateLeft;
    cpu->opcodeTable[0x0F] = rotateRight;
    cpu->opcodeTable[0x17] = rotateLeftC;
    cpu->opcodeTable[0x1F] = rotateRightC;

    for(uint8_t i = 0x40; i < 0x80; i++) {
        cpu->opcodeTable[i] = moveRegister;
    }

    for(uint8_t i = 0x46; i < 0x80; i += 8) {
        cpu->opcodeTable[i] = moveRAMtoReg;
    }

    for(uint8_t i = 0x70; i < 0x78; i++) {
        cpu->opcodeTable[i] = moveRAM;
    }

    cpu->opcodeTable[0x76] = halt;

    for(uint8_t i = 0x80; i < 0x90; i++) {
        cpu->opcodeTable[i] = addRegister;
    }

    cpu->opcodeTable[0x86] = addRAM;
    cpu->opcodeTable[0x8E] = addRAM;

    for(uint8_t i = 0x90; i < 0xA0; i++) {
        cpu->opcodeTable[i] = subRegister;
    }

    cpu->opcodeTable[0x96] = subRAM;
    cpu->opcodeTable[0x9E] = subRAM;

    for(uint8_t i = 0xA0; i < 0xA8; i++) {
        cpu->opcodeTable[i] = logicalAnd;
    }
    for(uint8_t i = 0xA8; i < 0xB0; i++) {
        cpu->opcodeTable[i] = logicalXor;
    }
    for(uint8_t i = 0xB0; i < 0xB8; i++) {
        cpu->opcodeTable[i] = logicalOr;
    }
    for(uint8_t i = 0xB8; i < 0xC0; i++) {
        cpu->opcodeTable[i] = compare;
    }

    cpu->opcodeTable[0xD3] = out;
    cpu->opcodeTable[0xDB] = in;

    cpu->opcodeTable[0xC3] = jump;
    cpu->opcodeTable[0xC8] = jump;
    cpu->opcodeTable[0xC9] = ret;
    cpu->opcodeTable[0xD9] = ret;
    for(int i = 0xCD; i < 0xFF; i += 16) {
        cpu->opcodeTable[i] = call;
    }
    for(int i = 0xC7; i < 0x100; i += 8) {
        cpu->opcodeTable[i] = restart;
    }

    for(int i = 0xC1; i < 0xF0; i += 16) {
        cpu->opcodeTable[i] = popPair;
    }
    cpu->opcodeTable[0xF1] = popPSW;
    for(int i = 0xC5; i < 0xF0; i += 16) {
        cpu->opcodeTable[i] = pushPair;
    }
    cpu->opcodeTable[0xF5] = pushPSW;
    for(int i = 0xC0; i < 0x100; i += 8) {
        cpu->opcodeTable[i] = conditionalRet;
    }
    for(int i = 0xC2; i < 0x100; i += 8) {
        cpu->opcodeTable[i] = conditionalJmp;
    }
    for(int i = 0xC4; i < 0x100; i += 8) {
        cpu->opcodeTable[i] = conditionalCall;
    }
    cpu->opcodeTable[0xC6] = addImm;
    cpu->opcodeTable[0xCE] = addImm;
    cpu->opcodeTable[0xD6] = subImm;
    cpu->opcodeTable[0xDE] = subImm;
    cpu->opcodeTable[0xE3] = exchangeHL;
    cpu->opcodeTable[0xEB] = exchangeHLDE;
    cpu->opcodeTable[0xE9] = loadPCHL;
    cpu->opcodeTable[0xF9] = loadSPHL;
    cpu->opcodeTable[0xF3] = disableInt;
    cpu->opcodeTable[0xFB] = enableInt;
    cpu->opcodeTable[0xE6] = andImm;
    cpu->opcodeTable[0xEE] = xorImm;
    cpu->opcodeTable[0xF6] = orImm;
    cpu->opcodeTable[0xFE] = compareImm;

    return cpu;
}

void executeInstruction(i8080 *cpu) {
    if(!cpu->halt) {
        cpu->currentOpcode = cpu->readAddress(cpu, cpu->pc);
        if(cpu->interruptEnable && cpu->pendingInterrupt) {
            cpu->pc--;
            cpu->interruptEnable = 0;
            cpu->currentOpcode = cpu->pendingInterrupt;
            cpu->opcodeTable[cpu->pendingInterrupt](cpu);
            cpu->pendingInterrupt = 0;
        } else {
            cpu->opcodeTable[cpu->currentOpcode](cpu);
        }
    }
}

uint8_t testNZ(i8080 *cpu) {
    return ~(checkZero(cpu)) & 0x01;
}

uint8_t testNC(i8080 *cpu) {
    return ~(checkCarry(cpu)) & 0x01;
}

uint8_t testPO(i8080 *cpu) {
    return ~(checkParity(cpu)) & 0x01;
}

uint8_t testP(i8080 *cpu) {
    return ~(checkSign(cpu)) & 0x01;
}
