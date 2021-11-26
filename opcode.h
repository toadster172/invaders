#ifndef OPCODE_H
#define OPCODE_H

#include "i8080Type.h"

void nop(i8080 *cpu);
void loadImmPair(i8080 *cpu);
void loadImmSP(i8080 *cpu);
void storeAX(i8080 *cpu);
void storeHL(i8080 *cpu);
void storeA(i8080 *cpu);
void incPair(i8080 *cpu);
void incSP(i8080 *cpu);
void incReg(i8080 *cpu);
void incMem(i8080 *cpu);
void decReg(i8080 *cpu);
void decMem(i8080 *cpu);
void decPair(i8080 *cpu);
void decSP(i8080 *cpu);
void addPair(i8080 *cpu);
void addSP(i8080 *cpu);
void rotateLeft(i8080 *cpu);
void rotateLeftC(i8080 *cpu);
void rotateRight(i8080 *cpu);
void rotateRightC(i8080 *cpu);
void decimalAdjust(i8080 *cpu);
void complementA(i8080 *cpu);
void setC(i8080 *cpu);
void complementC(i8080 *cpu);
void loadAX(i8080 *cpu); //Load a from memory address in register pair
void loadHL(i8080 *cpu); //Load hl from immediate memory address
void loadA(i8080 *cpu); //Load a from immediate memory address
void moveImmReg(i8080 *cpu);
void moveImmMem(i8080 *cpu);
void moveRegister(i8080 *cpu); //Used for any movement of one register to another
void moveRAMtoReg(i8080 *cpu); //Used for movement of (hl) to a register
void moveRAM(i8080 *cpu); //Used for movement of a register to (hl)
void halt(i8080 *cpu);
void addRegister(i8080 *cpu);
void addRAM(i8080 *cpu);
void subRegister(i8080 *cpu);
void subRAM(i8080 *cpu);
void logicalAnd(i8080 *cpu);
void logicalXor(i8080 *cpu);
void logicalOr(i8080 *cpu);
void compare(i8080 *cpu);
void pushPair(i8080 *cpu);
void pushPSW(i8080 *cpu);
void popPair(i8080 *cpu);
void popPSW(i8080 *cpu);
void out(i8080 *cpu);
void in(i8080 *cpu);
void conditionalRet(i8080 *cpu);
void conditionalJmp(i8080 *cpu);
void jump(i8080 *cpu);
void conditionalCall(i8080 *cpu);
void ret(i8080 *cpu);
void call(i8080 *cpu);
void restart(i8080 *cpu);
void addImm(i8080 *cpu);
void subImm(i8080 *cpu);
void exchangeHL(i8080 *cpu);
void exchangeHLDE(i8080 *cpu);
void loadPCHL(i8080 *cpu);
void loadSPHL(i8080 *cpu);
void disableInt(i8080 *cpu);
void enableInt(i8080 *cpu);
void andImm(i8080 *cpu);
void xorImm(i8080 *cpu);
void orImm(i8080 *cpu);
void compareImm(i8080 *cpu);

#endif
