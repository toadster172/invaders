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

#include "opcode.h"
#include <stdio.h>

void pushWord(i8080 *cpu, uint16_t w);
uint16_t popWord(i8080 *cpu);
//Performs cpu->a + n ( + c for carry) and sets flags accordingly
uint8_t add(i8080 *cpu, uint8_t n, uint8_t c);
uint8_t sub(i8080 *cpu, uint8_t n, uint8_t c);

void setCommonFlags(i8080 *cpu, uint8_t n);
uint8_t parityEven(uint8_t n);

void nop(i8080 *cpu) {
    cpu->pc++;
    cpu->cycles += 4;
}

void loadImmPair(i8080 *cpu) {
    *cpu->registerDecode[cpu->currentOpcode >> 3] = cpu->readAddress(cpu, cpu->pc + 2);
    *cpu->registerDecode[(cpu->currentOpcode >> 3) + 1] = cpu->readAddress(cpu, cpu->pc + 1);
    cpu->pc += 3;
    cpu->cycles += 10;
}

void loadImmSP(i8080 *cpu) {
    uint16_t loadValue = cpu->readAddress(cpu, cpu->pc + 1);
    loadValue += cpu->readAddress(cpu, cpu->pc + 2) << 8;
    cpu->sp = loadValue;
    cpu->pc += 3;
    cpu->cycles += 10;
}

void storeAX(i8080 *cpu) {
    uint16_t pairAddress = *cpu->registerDecode[cpu->currentOpcode >> 3] << 8;
    pairAddress += *cpu->registerDecode[(cpu->currentOpcode >> 3) + 1];
    cpu->writeAddress(cpu, pairAddress, cpu->a);
    cpu->pc += 1;
    cpu->cycles += 7;
}

void storeHL(i8080 *cpu) {
    uint16_t storeAddress = cpu->readAddress(cpu, cpu->pc + 1);
    storeAddress += cpu->readAddress(cpu, cpu->pc + 2) << 8;
    cpu->writeAddress(cpu, storeAddress, cpu->l);
    cpu->writeAddress(cpu, storeAddress + 1, cpu->h);
    cpu->pc += 3;
    cpu->cycles += 16;
}

void storeA(i8080 *cpu) {
    uint16_t storeAddress = cpu->readAddress(cpu, cpu->pc + 1);
    storeAddress += cpu->readAddress(cpu, cpu->pc + 2) << 8;
    cpu->writeAddress(cpu, storeAddress, cpu->a);
    cpu->pc += 3;
    cpu->cycles += 13;
}

void incPair(i8080 *cpu) {
    uint16_t pairNum = *cpu->registerDecode[cpu->currentOpcode >> 3] << 8;
    pairNum += *cpu->registerDecode[(cpu->currentOpcode >> 3) + 1];
    pairNum++;
    *cpu->registerDecode[cpu->currentOpcode >> 3] = (uint8_t)(pairNum >> 8);
    *cpu->registerDecode[(cpu->currentOpcode >> 3) + 1] = (uint8_t)(pairNum & 0xFF);
    cpu->pc++;
    cpu->cycles += 5;
}

void incSP(i8080 *cpu) {
    cpu->sp++;
    cpu->pc++;
    cpu->cycles += 5;
}

void incReg(i8080 *cpu) {
    uint8_t *reg = cpu->registerDecode[cpu->currentOpcode >> 3];
    (*reg)++;
    setCommonFlags(cpu, *reg);
    if(*reg & 0x0F) {
        clearAC(cpu);
    } else {
        setAC(cpu);
    }
    cpu->pc++;
    cpu->cycles += 5;
}

void incMem(i8080 *cpu) {
    uint8_t memValue = cpu->readAddress(cpu, ((uint16_t) cpu->h << 8) + cpu->l);
    cpu->writeAddress(cpu, ((uint16_t) cpu->h << 8) + cpu->l, ++memValue);
    setCommonFlags(cpu, memValue);
    if(memValue & 0x0F) {
        clearAC(cpu);
    } else {
        setAC(cpu);
    }
    cpu->pc++;
    cpu->cycles += 10;
}

void decReg(i8080 *cpu) {
    uint8_t *reg = cpu->registerDecode[cpu->currentOpcode >> 3];
    (*reg)--;
    setCommonFlags(cpu, *reg);
    if((*reg & 0x0F) == 0x0F) {
        clearAC(cpu);
    } else {
        setAC(cpu);
    }
    cpu->pc++;
    cpu->cycles += 5;

}

void decMem(i8080 *cpu) {
    uint8_t memValue = cpu->readAddress(cpu, ((uint16_t) cpu->h << 8) + cpu->l);
    cpu->writeAddress(cpu, ((uint16_t) cpu->h << 8) + cpu->l, --memValue);
    setCommonFlags(cpu, memValue);
    if((memValue & 0x0F) == 0x0F) {
        clearAC(cpu);
    } else {
        setAC(cpu);
    }
    cpu->pc++;
    cpu->cycles += 10;
}

void decPair(i8080 *cpu) {
    uint16_t pairNum = *cpu->registerDecode[(cpu->currentOpcode & 0xF0) >> 3] << 8;
    pairNum += *cpu->registerDecode[((cpu->currentOpcode & 0xF0) >> 3) + 1];
    pairNum--;
    *cpu->registerDecode[(cpu->currentOpcode & 0xF0) >> 3] = (uint8_t)(pairNum >> 8);
    *cpu->registerDecode[((cpu->currentOpcode & 0xF0) >> 3) + 1] = (uint8_t)(pairNum & 0xFF);
    cpu->pc++;
    cpu->cycles += 5;
}

void decSP(i8080 *cpu) {
    cpu->sp--;
    cpu->pc++;
    cpu->cycles += 5;
}

void addPair(i8080 *cpu) {
    uint16_t hlValue = (cpu->h << 8) + cpu->l;
    uint16_t addOperand = *cpu->registerDecode[(cpu->currentOpcode & 0xF0) >> 3] << 8;
    addOperand += *cpu->registerDecode[((cpu->currentOpcode & 0xF0) >> 3) + 1];
    if(hlValue + addOperand > 0xFFFF) {
        setCarry(cpu);
    } else {
        clearCarry(cpu);
    }
    hlValue += addOperand;
    cpu->h = (uint8_t) (hlValue >> 8);
    cpu->l = (uint8_t) (hlValue & 0x00FF);
    cpu->pc++;
    cpu->cycles += 10;
}

void addSP(i8080 *cpu) {
    uint16_t hlValue = (cpu->h << 8) + cpu->l;
    if(hlValue + cpu->sp > 0xFFFF) {
        setCarry(cpu);
    } else {
        clearCarry(cpu);
    }
    hlValue += cpu->sp;
    cpu->h = (uint8_t) (hlValue >> 8);
    cpu->l = (uint8_t) (hlValue & 0x00FF);
    cpu->pc++;
    cpu->cycles += 10;
}

void rotateLeft(i8080 *cpu) {
    cpu->a = (cpu->a << 1) | ((cpu->a & 0x80) >> 7);
    if(cpu->a & 0x01) {
        setCarry(cpu);
    } else {
        clearCarry(cpu);
    }
    cpu->pc++;
    cpu->cycles += 4;
}

void rotateLeftC(i8080 *cpu) {
    uint8_t carryBit = cpu->a & 0x80;
    cpu->a = (cpu->a << 1) | (cpu->flag & 0x01);
    if(carryBit) {
        setCarry(cpu);
    } else {
        clearCarry(cpu);
    }
    cpu->pc++;
    cpu->cycles += 4;
}

void rotateRight(i8080 *cpu) {
    cpu->a = (cpu->a >> 1) | ((cpu->a & 0x01) << 7);
    if(cpu->a & 0x80) {
        setCarry(cpu);
    } else {
        clearCarry(cpu);
    }
    cpu->pc++;
    cpu->cycles += 4;
}

void rotateRightC(i8080 *cpu) {
    uint8_t carryBit = cpu->a & 0x01;
    cpu->a = (cpu->a >> 1) | ((cpu->flag & 0x01) << 7);
    if(carryBit) {
        setCarry(cpu);
    } else {
        clearCarry(cpu);
    }
    cpu->pc++;
    cpu->cycles += 4;
}

void decimalAdjust(i8080 *cpu) {
    if(((cpu->a & 0x0F) > 9) | checkAC(cpu)) {
        if(!checkAC(cpu)) {
            setAC(cpu);
        } else {
            clearAC(cpu);
        }
        cpu->a += 6;
    } else {
        clearAC(cpu);
    }
    if(((cpu->a & 0xF0) > 0x90) | checkCarry(cpu)) {
        setCarry(cpu);
        cpu->a += 0x60;
    } else {
        clearCarry(cpu);
    }
    setCommonFlags(cpu, cpu->a);
    cpu->pc++;
    cpu->cycles += 4;
}

void complementA(i8080 *cpu) {
    cpu->a = ~(cpu->a);
    cpu->pc++;
    cpu->cycles += 4;
}

void setC(i8080 *cpu) {
    setCarry(cpu);
    cpu->pc++;
    cpu->cycles += 4;
}

void complementC(i8080 *cpu) {
    cpu->flag ^= 0x01;
    cpu->pc++;
    cpu->cycles += 4;
}

void loadAX(i8080 *cpu) {
    uint16_t pairAddress = *cpu->registerDecode[(cpu->currentOpcode & 0xF0) >> 3] << 8;
    pairAddress += *cpu->registerDecode[((cpu->currentOpcode & 0xF0) >> 3) + 1];
    cpu->a = cpu->readAddress(cpu, pairAddress);
    cpu->pc += 1;
    cpu->cycles += 7;
}

void loadHL(i8080 *cpu) {
    uint16_t loadAddress = cpu->readAddress(cpu, cpu->pc + 1);
    loadAddress += cpu->readAddress(cpu, cpu->pc + 2) << 8;
    cpu->h = cpu->readAddress(cpu, loadAddress + 1);
    cpu->l = cpu->readAddress(cpu, loadAddress);
    cpu->pc += 3;
    cpu->cycles += 16;
}

void loadA(i8080 *cpu) {
    uint16_t loadAddress = cpu->readAddress(cpu, cpu->pc + 1);
    loadAddress += cpu->readAddress(cpu, cpu->pc + 2) << 8;
    cpu->a = cpu->readAddress(cpu, loadAddress);
    cpu->pc += 3;
    cpu->cycles += 13;
}

void moveImmReg(i8080 *cpu) {
    uint8_t *destReg = cpu->registerDecode[cpu->currentOpcode >> 3];
    *destReg = cpu->readAddress(cpu, cpu->pc + 1);
    cpu->pc += 2;
    cpu->cycles += 7;
}

void moveImmMem(i8080 *cpu) {
    uint8_t wByte = cpu->readAddress(cpu, cpu->pc + 1);
    cpu->writeAddress(cpu, ((uint16_t) cpu->h << 8) + cpu->l, wByte);
    cpu->pc += 2;
    cpu->cycles += 10;
}

void moveRegister(i8080 *cpu) {
    uint8_t *destReg = cpu->registerDecode[(cpu->currentOpcode & 0x3F) >> 3];
    *destReg = *(cpu->registerDecode[cpu->currentOpcode & 0x07]);
    cpu->pc++;
    cpu->cycles += 5;
}

void moveRAMtoReg(i8080 *cpu) {
    uint8_t *destReg = cpu->registerDecode[(cpu->currentOpcode & 0x3F) >> 3];
    *destReg = cpu->readAddress(cpu, ((uint16_t) cpu->h << 8) + cpu->l);
    cpu->pc += 1;
    cpu->cycles += 7;
}

void moveRAM(i8080 *cpu) {
    uint16_t destAddress = ((uint16_t) cpu->h << 8) + cpu->l;
    cpu->writeAddress(cpu, destAddress, *(cpu->registerDecode[cpu->currentOpcode & 0x07]));
    cpu->pc += 1;
    cpu->cycles += 7;
}

void halt(i8080 *cpu) {
    cpu->halt = 1;
    cpu->pc++;
    cpu->cycles += 7;
}

void addRegister(i8080 *cpu) {
    uint8_t *sourceReg = cpu->registerDecode[(cpu->currentOpcode & 0x07)];
    if(!(cpu->currentOpcode & 0x08)) { //0x80-87 don't use carry, 0x88-8F do
        cpu->a = add(cpu, *sourceReg, 0);
    } else {
        cpu->a = add(cpu, *sourceReg, checkCarry(cpu));
    }
    cpu->pc++;
    cpu->cycles += 4;
}

void addRAM(i8080 *cpu) {
    uint16_t sourceAddress = ((uint16_t) cpu->h << 8) + cpu->l;
    if(!(cpu->currentOpcode & 0x08)) { //0x80-87 don't use carry, 0x88-8F do
        cpu->a = add(cpu, cpu->readAddress(cpu, sourceAddress), 0);
    } else {
        cpu->a = add(cpu, cpu->readAddress(cpu, sourceAddress), checkCarry(cpu));
    }
    cpu->pc++;
    cpu->cycles += 7;
}

void subRegister(i8080 *cpu) {
    uint8_t *sourceReg = cpu->registerDecode[cpu->currentOpcode & 0x07];
    if(!(cpu->currentOpcode & 0x08)) { //0x90-97 don't use carry, 0x98-9F do
        cpu->a = sub(cpu, *sourceReg, 0);
    } else {
        cpu->a = sub(cpu, *sourceReg, checkCarry(cpu));
    }
    cpu->pc++;
    cpu->cycles += 4;
}

void subRAM(i8080 *cpu) {
    uint16_t sourceAddress = ((uint16_t) cpu->h << 8) + cpu->l;
    if(!(cpu->currentOpcode & 0x08)) { //0x90-97 don't use carry, 0x98-9F do
        cpu->a = sub(cpu, cpu->readAddress(cpu, sourceAddress), 0);
    } else {
        cpu->a = sub(cpu, cpu->readAddress(cpu, sourceAddress), checkCarry(cpu));
    }
    cpu->pc++;
    cpu->cycles += 7;
}

void logicalAnd(i8080 *cpu) {
    uint8_t andOperand = 0;
    if((cpu->currentOpcode & 0x07) == 6) {
        andOperand = cpu->readAddress(cpu, ((uint16_t) cpu->h << 8) + cpu->l);
        cpu->cycles += 3;
    } else {
        andOperand = *(cpu->registerDecode[cpu->currentOpcode & 0x07]);
    }

    if((cpu->a | andOperand) & 0x08) {
        setAC(cpu);
    } else {
        clearAC(cpu);
    }
    clearCarry(cpu);

    cpu->a &= andOperand;
    setCommonFlags(cpu, cpu->a);

    cpu->pc++;
    cpu->cycles += 4;
}

void logicalXor(i8080 *cpu) {
    uint8_t xorOperand = 0;
    if((cpu->currentOpcode & 0x07) == 6) {
        xorOperand = cpu->readAddress(cpu, ((uint16_t) cpu->h << 8) + cpu->l);
        cpu->cycles += 3;
    } else {
        xorOperand = *(cpu->registerDecode[cpu->currentOpcode & 0x07]);
    }

    clearAC(cpu);
    clearCarry(cpu);

    cpu->a ^= xorOperand;
    setCommonFlags(cpu, cpu->a);

    cpu->pc++;
    cpu->cycles += 4;
}

void logicalOr(i8080 *cpu) {
    uint8_t orOperand = 0;
    if((cpu->currentOpcode & 0x07) == 6) {
        orOperand = cpu->readAddress(cpu, ((uint16_t) cpu->h << 8) + cpu->l);
        cpu->cycles += 3;
    } else {
        orOperand = *(cpu->registerDecode[cpu->currentOpcode & 0x07]);
    }

    clearAC(cpu);
    clearCarry(cpu);

    cpu->a |= orOperand;
    setCommonFlags(cpu, cpu->a);

    cpu->pc++;
    cpu->cycles += 4;
}

void compare(i8080 *cpu) {
    uint8_t cmpOperand = 0;
    if((cpu->currentOpcode & 0x07) == 6) {
        cmpOperand = cpu->readAddress(cpu, ((uint16_t) cpu->h << 8) + cpu->l);
        cpu->cycles += 3;
    } else {
        cmpOperand = *(cpu->registerDecode[cpu->currentOpcode & 0x07]);
    }

    sub(cpu, cmpOperand, 0);
    
    cpu->pc++;
    cpu->cycles += 4;
}

void pushPair(i8080 *cpu) {
    uint16_t pairWord = *cpu->registerDecode[(cpu->currentOpcode & 0x30) >> 3] << 8;
    pairWord += *cpu->registerDecode[((cpu->currentOpcode & 0x30) >> 3) + 1];
    pushWord(cpu, pairWord);
    cpu->pc++;
    cpu->cycles += 11;
}

void pushPSW(i8080 *cpu) {
    pushWord(cpu, (cpu->a << 8) + cpu->flag);
    cpu->pc++;
    cpu->cycles += 11;
}

void popPair(i8080 *cpu) {
    uint16_t pairWord = popWord(cpu);
    *cpu->registerDecode[(cpu->currentOpcode & 0x30) >> 3] = (pairWord & 0xFF00) >> 8;
    *cpu->registerDecode[((cpu->currentOpcode & 0x30) >> 3) + 1] = pairWord & 0x00FF;
    cpu->pc++;
    cpu->cycles += 10;
}

void popPSW(i8080 *cpu) {
    uint16_t pairWord = popWord(cpu);
    cpu->a = (pairWord & 0xFF00) >> 8;
    cpu->flag = pairWord & 0x00FF;
    cpu->flag |= 0x02; //Always set
    cpu->flag &= 0xD7; //Bits 3 and 5 are never set
    cpu->pc++;
    cpu->cycles += 10;
}


void out(i8080 *cpu) {
    cpu->writePort(cpu, cpu->readAddress(cpu, cpu->pc + 1));
    cpu->pc += 2;
    cpu->cycles += 10;
}

void in(i8080 *cpu) {
    cpu->a = cpu->readPort(cpu, cpu->readAddress(cpu, cpu->pc + 1));
    cpu->pc += 2;
    cpu->cycles += 10;
}

void conditionalRet(i8080 *cpu) {
    uint8_t reversed = cpu->currentOpcode & 0x08;
    
    cpu->cycles++;
    if((cpu->conditionalTable[(cpu->currentOpcode >> 4) & 0x03](cpu) && !reversed) ||
        (!(cpu->conditionalTable[(cpu->currentOpcode >> 4) & 0x03](cpu)) && reversed)) {
        ret(cpu);
    } else {
        cpu->cycles += 4;
        cpu->pc++;
    }
}

void conditionalJmp(i8080 *cpu) {
    uint8_t reversed = cpu->currentOpcode & 0x08;
    if((cpu->conditionalTable[(cpu->currentOpcode >> 4) & 0x03](cpu) && !reversed) ||
        (!cpu->conditionalTable[(cpu->currentOpcode >> 4) & 0x03](cpu) && reversed)) {
        jump(cpu);
    } else {
        cpu->cycles += 10;
        cpu->pc += 3;
    }
}

void jump(i8080 *cpu) {
    uint16_t jmpAddress = cpu->readAddress(cpu, cpu->pc + 1);
    jmpAddress += (cpu->readAddress(cpu, cpu->pc + 2) << 8);
    cpu->pc = jmpAddress;
    cpu->cycles += 10;
}

void conditionalCall(i8080 *cpu) {
    uint8_t reversed = cpu->currentOpcode & 0x08;
    if((cpu->conditionalTable[(cpu->currentOpcode >> 4) & 0x03](cpu) && !reversed) ||
        (!cpu->conditionalTable[(cpu->currentOpcode >> 4) & 0x03](cpu) && reversed)) {
        call(cpu);
    } else {
        cpu->cycles += 11;
        cpu->pc += 3;
    }
}

void ret(i8080 *cpu) {
    cpu->pc = popWord(cpu);
    cpu->cycles += 10;
}

void call(i8080 *cpu) {
    uint16_t jmpAddress = cpu->readAddress(cpu, cpu->pc + 1);
    jmpAddress += (cpu->readAddress(cpu, cpu->pc + 2) << 8);
    cpu->pc += 3;
    pushWord(cpu, cpu->pc);
    cpu->pc = jmpAddress;
    cpu->cycles += 17;
}

void restart(i8080 *cpu) {
    cpu->pc += 1;
    pushWord(cpu, cpu->pc);
    cpu->pc = cpu->currentOpcode & 0x38; //0b00111000
    cpu->cycles += 11;
}

void addImm(i8080 *cpu) {
    uint8_t addValue = cpu->readAddress(cpu, cpu->pc + 1);
    cpu->a = add(cpu, addValue, (checkCarry(cpu) && cpu->currentOpcode & 0x08));
    cpu->pc += 2;
    cpu->cycles += 7;
}

void subImm(i8080 *cpu) {
    uint8_t subValue = cpu->readAddress(cpu, cpu->pc + 1);
    cpu->a = sub(cpu, subValue, (checkCarry(cpu) && cpu->currentOpcode & 0x08));
    cpu->pc += 2;
    cpu->cycles += 7;
}

void exchangeHL(i8080 *cpu) {
    uint16_t stackWord = popWord(cpu);
    uint16_t hlValue = (cpu->h << 8) + cpu->l;
    pushWord(cpu, hlValue);
    cpu->h = (uint8_t)((stackWord & 0xFF00) >> 8);
    cpu->l = (uint8_t)(stackWord & 0x00FF);
    cpu->pc++;
    cpu->cycles += 18;
}

void exchangeHLDE(i8080 *cpu) {
    uint8_t tmpRegister = cpu->h;
    cpu->h = cpu->d;
    cpu->d = tmpRegister;
    tmpRegister = cpu->l;
    cpu->l = cpu->e;
    cpu->e = tmpRegister;
    cpu->pc++;
    cpu->cycles += 5;
}

void loadPCHL(i8080 *cpu) {
    cpu->pc = (((uint16_t) cpu->h) << 8);
    cpu->pc |= cpu->l;
    cpu->cycles += 5;
}

void loadSPHL(i8080 *cpu) {
    cpu->sp = (((uint16_t) cpu->h) << 8);
    cpu->sp |= cpu->l;
    cpu->pc++;
    cpu->cycles += 5;
}

void disableInt(i8080 *cpu) {
    cpu->interruptEnable = 0;
    cpu->pc++;
    cpu->cycles += 4;
}

void enableInt(i8080 *cpu) {
    cpu->interruptEnable = 1;
    cpu->pendingInterrupt = 0;
    cpu->pc++;
    cpu->cycles += 4;
}

void andImm(i8080 *cpu) {
    uint8_t andOperand = cpu->readAddress(cpu, cpu->pc + 1);
    if((cpu->a | andOperand) & 0x08) {
        setAC(cpu);
    } else {
        clearAC(cpu);
    }
    clearCarry(cpu);

    cpu->a &= andOperand;
    setCommonFlags(cpu, cpu->a);

    cpu->pc += 2;
    cpu->cycles += 7;
}

void xorImm(i8080 *cpu) {
    uint8_t xorOperand = cpu->readAddress(cpu, cpu->pc + 1);

    clearAC(cpu);
    clearCarry(cpu);

    cpu->a ^= xorOperand;
    setCommonFlags(cpu, cpu->a);

    cpu->pc += 2;
    cpu->cycles += 7;
}

void orImm(i8080 *cpu) {
    uint8_t orOperand = cpu->readAddress(cpu, cpu->pc + 1);

    clearAC(cpu);
    clearCarry(cpu);

    cpu->a |= orOperand;
    setCommonFlags(cpu, cpu->a);

    cpu->pc += 2;
    cpu->cycles += 7;
}

void compareImm(i8080 *cpu) {
    uint8_t cmpOperand = cpu->readAddress(cpu, cpu->pc + 1);
    
    sub(cpu, cmpOperand, 0);
    
    cpu->pc += 2;
    cpu->cycles += 7;
}

void pushWord(i8080 *cpu, uint16_t w) {
    cpu->sp -= 2;
    cpu->writeAddress(cpu, cpu->sp, w & 0x00FF);
    cpu->writeAddress(cpu, cpu->sp + 1, w >> 8);
}

uint16_t popWord(i8080 *cpu) {
    uint16_t stackReturn = cpu->readAddress(cpu, cpu->sp);
    stackReturn += (cpu->readAddress(cpu, cpu->sp + 1) << 8);
    cpu->sp += 2;
    return stackReturn;
}

uint8_t add(i8080 *cpu, uint8_t n, uint8_t c) {
    uint8_t result = cpu->a + n + c;
    if((((cpu->a & 0x0F) + (n & 0x0F)) + c) & 0x10) {
        setAC(cpu);
    } else {
        clearAC(cpu);
    }
    if((cpu->a + n + c) & 0x100) {
        setCarry(cpu);
    } else {
        clearCarry(cpu);
    }

    setCommonFlags(cpu, result);

    return result;
}

uint8_t sub(i8080 *cpu, uint8_t n, uint8_t c) {
    uint8_t result = add(cpu, ~n, !c);
    if(checkCarry(cpu)) {
        clearCarry(cpu);
    } else {
        setCarry(cpu);
    }
    
    return result;
}

void setCommonFlags(i8080 *cpu, uint8_t n) {
    if(n & 0x80) {
        setSign(cpu);
    } else {
        clearSign(cpu);
    }
    if(!n) {
        setZero(cpu);
    } else {
        clearZero(cpu);
    }
    if(parityEven(n)) {
        setParity(cpu);
    } else {
        clearParity(cpu);
    }
}

//Returns 1 if parity is even, 0 if parity is odd
uint8_t parityEven(uint8_t n) {
    uint8_t p = 1;
    for(uint8_t i = 0; i < 8; i++) {
        p ^= n & 1;
        n >>= 1;
    }
    return p;
}
