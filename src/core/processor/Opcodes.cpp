// Copyright (C) 2017 Ryan Terry
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Processor.h"
#include "../memory/MemoryBus.h"

#include "../../common/Globals.h"



namespace Core {

// load
void Processor::ld(Reg8& reg, u8 value)
{
    reg = value;
}
void Processor::ld(Reg16& reg, u16 value)
{
    reg.word = value;
}
void Processor::ld_sp_plus(Reg16& reg, s8 value)
{
    SetZero( false );
    SetSubtract( false );
    SetHalfCarry( ((reg_SP.word & 0x000F) + (static_cast<u8>(value) & 0x0F)) & 0x0010 );
    SetCarry( ((reg_SP.word & 0x00FF) + static_cast<u8>(value)) & 0x0100 );
    reg.word = static_cast<u16>( reg_SP.word + value );
}

// write
void Processor::ldAt(u16 addr, u8 value)
{
    memory_bus->Write8(addr, value);
}
void Processor::ldAt(u16 addr, u16 value)
{
    memory_bus->Write16(addr, value);
}

// inc/dec
void Processor::inc(Reg8& reg)
{
    SetSubtract( false );
    SetHalfCarry( (reg & 0x0F) == 0x0F );
    ++reg;

    SetZero( reg == 0x00 );
}
void Processor::inc(Reg16& reg16)
{
    ++reg16.word;
}
void Processor::incAt(u16 addr)
{
    u8 value = memory_bus->Read8(addr);

    SetSubtract( false );
    SetHalfCarry( (value & 0x0F) == 0x0F );
    
    memory_bus->Write8(addr, ++value);

    SetZero( value == 0x00 );
}

void Processor::dec(Reg8& reg)
{
    SetSubtract( true );
    SetHalfCarry( (reg & 0x0F) == 0x00 );
    --reg;

    SetZero( reg == 0x00 );
}
void Processor::dec(Reg16& reg16)
{
    --reg16.word;
}
void Processor::decAt(u16 addr)
{
    u8 value = memory_bus->Read8(addr);
    
    SetSubtract( true );
    SetHalfCarry( (value & 0x0F) == 0x00 );

    memory_bus->Write8(addr, --value);

    SetZero( value == 0x00 );
}

// add
void Processor::add(Reg8& reg, u8 value)
{
    u16 tempAdd = reg + value;
    SetSubtract( false );
    SetHalfCarry( ((reg & 0x0F) + (value & 0x0F)) & 0x10 );
    SetCarry( tempAdd & 0x0100 );
    SetZero( (tempAdd & 0x00FF) == 0x00 );
    reg = static_cast<u8>(tempAdd);
}
void Processor::add(Reg16& reg, u16 value)
{
    SetSubtract( false );
    SetHalfCarry( ((reg.word & 0x0FFF) + (value & 0x0FFF)) & 0x1000 );
    SetCarry( value > (0xFFFF - reg.word) );
    reg.word += value;
}
void Processor::add(Reg16& reg, s8 value)
{
    SetSubtract( false );
    SetZero( false );
    SetHalfCarry( ((reg.word & 0x000F) + (static_cast<u8>(value) & 0x0F)) & 0x0010 );
    SetCarry( ((reg.word & 0x00FF) + static_cast<u8>(value)) & 0x0100 );
    reg.word += value;
}
void Processor::adc(Reg8& reg, u8 value)
{
    u16 tempAdd = reg + value + Carry();
    SetSubtract( false );
    SetHalfCarry( ((reg & 0x0F) + (value & 0x0F) + Carry()) & 0x10 );
    SetCarry( tempAdd & 0x0100 );
    SetZero( (tempAdd & 0x00FF) == 0x00 );
    reg = tempAdd & 0x00FF;
}

// sub
void Processor::sub(Reg8& reg, u8 value)
{
    SetSubtract( true );
    SetHalfCarry( (reg & 0x0F) < (value & 0x0F) );
    SetCarry( (reg < value) );
    reg -= value;

    SetZero( reg == 0x00 );
}
void Processor::sbc(Reg8& reg, u8 value)
{
    u8 adder = Carry()? 1 : 0;
    SetSubtract( true );
    SetHalfCarry( (reg & 0x0F) < ((value & 0x0F) + adder) );
    SetCarry( reg < (value + adder) );
    reg -= value + adder;

    SetZero( reg == 0x00 );
}

// bitwise
void Processor::and8(Reg8& reg, u8 value)
{
    reg &= value;

    SetSubtract( false );
    SetHalfCarry( true );
    SetCarry( false );
    SetZero( reg == 0x00 );
}

void Processor::xor8(Reg8& reg, u8 value)
{
    reg ^= value;

    SetSubtract( false );
    SetHalfCarry( false );
    SetCarry( false );
    SetZero( reg == 0x00 );
}

void Processor::or8(Reg8& reg, u8 value)
{
    reg |= value;

    SetSubtract( false );
    SetHalfCarry( false );
    SetCarry( false );
    SetZero( reg == 0x00 );
}

// daa
void Processor::daa()
{
    if(Subtract())
    {
        if(Carry())
        {
            reg_A -= 0x60;
        }
        if(HalfCarry())
        {
            reg_A -= 0x06;
        }
    }
    else
    {
        if(Carry() || reg_A > 0x99)
        {
            reg_A += 0x60;
            SetCarry( true );
        }
        if(HalfCarry() || (reg_A & 0x0F) > 0x09)
        {
            reg_A += 0x06;
        }
    }
    SetZero( reg_A == 0x00 );
    SetHalfCarry( false );
}

// compare
void Processor::cp(u8 value)
{
    SetSubtract( true );
    SetHalfCarry( (reg_A & 0x0F) < (value & 0x0F) );
    SetCarry( reg_A < value );
    SetZero( (reg_A == value) );
}

// jump
void Processor::jr(s8 amt)
{
    reg_PC.word += amt;
}

void Processor::jp(u16 addr)
{
    reg_PC.word = addr;
}

void Processor::call(u16 addr)
{
    push(reg_PC.word);
    reg_PC.word = addr;
}

void Processor::ret()
{
    pop(reg_PC);
}

// stack
void Processor::push(u16 value)
{
    reg_SP.word -= 2;
    memory_bus->Write16(reg_SP.word, value);
}

void Processor::pop(Reg16& reg16)
{
    reg16.word = memory_bus->Read16(reg_SP.word);
    reg_SP.word += 2;
}

// CB opcodes
// rotate
// The CB versions of these instructions modify Zero
void Processor::rlc(Reg8& reg, bool zero)
{
    bool newCarry = reg & 0b10000000;
    reg <<= 1;
    reg |= newCarry;
    SetCarry( newCarry );

    SetSubtract( false );
    SetHalfCarry( false );
    SetZero( zero? reg == 0x00 : false );
}
void Processor::rlcAt(u16 addr, bool zero)
{
    u8 value = memory_bus->Read8(addr);
    bool newCarry = value & 0b10000000;
    value <<= 1;
    value |= newCarry;
    SetCarry( newCarry );

    SetSubtract( false );
    SetHalfCarry( false );
    SetZero( zero? value == 0x00 : false );
    memory_bus->Write8(addr, value);
}
void Processor::rl(Reg8& reg, bool zero)
{
    bool newCarry = reg & 0b10000000;
    reg <<= 1;
    reg |= Carry();
    SetCarry( newCarry );

    SetSubtract( false );
    SetHalfCarry( false );
    SetZero( zero? reg == 0x00 : false );
}
void Processor::rlAt(u16 addr, bool zero)
{
    u8 value = memory_bus->Read8(addr);
    bool newCarry = value & 0b10000000;
    value <<= 1;
    value |= Carry();
    SetCarry( newCarry );

    SetSubtract( false );
    SetHalfCarry( false );
    SetZero( zero? value == 0x00 : false );
    memory_bus->Write8(addr, value);
}
void Processor::rrc(Reg8& reg, bool zero)
{
    bool newCarry = reg & 0b00000001;
    reg >>= 1;
    reg |= newCarry << 7;
    SetCarry( newCarry );

    SetSubtract( false );
    SetHalfCarry( false );
    SetZero( zero? reg == 0x00 : false );
}
void Processor::rrcAt(u16 addr, bool zero)
{
    u8 value = memory_bus->Read8(addr);
    bool newCarry = value & 0b00000001;
    value >>= 1;
    value |= newCarry << 7;
    SetCarry( newCarry );

    SetSubtract( false );
    SetHalfCarry( false );
    SetZero( zero? value == 0x00 : false );
    memory_bus->Write8(addr, value);
}
void Processor::rr(Reg8& reg, bool zero)
{
    bool newCarry = reg & 0b00000001;
    reg >>= 1;
    reg |= Carry() << 7;
    SetCarry( newCarry );

    SetSubtract( false );
    SetHalfCarry( false );
    SetZero( zero? reg == 0x00 : false );
}
void Processor::rrAt(u16 addr, bool zero)
{
    u8 value = memory_bus->Read8(addr);
    bool newCarry = value & 0b00000001;
    value >>= 1;
    value |= Carry() << 7;
    SetCarry( newCarry );

    SetSubtract( false );
    SetHalfCarry( false );
    SetZero( zero? value == 0x00 : false );
    memory_bus->Write8(addr, value);
}

// shift
void Processor::sla(Reg8& reg)
{
    SetCarry( reg & 0b10000000 );
    reg <<= 1;

    SetSubtract( false );
    SetHalfCarry( false );
    SetZero( reg == 0x00 );
}
void Processor::slaAt(u16 addr)
{
    u8 value = memory_bus->Read8(addr);
    SetCarry( value & 0b10000000 );
    value <<= 1;

    SetSubtract( false );
    SetHalfCarry( false );
    SetZero( value == 0x00 );
    memory_bus->Write8(addr, value);
}
void Processor::srl(Reg8& reg)
{
    SetCarry( reg & 0b00000001 );
    reg >>= 1;

    SetSubtract( false );
    SetHalfCarry( false );
    SetZero( reg == 0x00 );
}
void Processor::srlAt(u16 addr)
{
    u8 value = memory_bus->Read8(addr);
    SetCarry( value & 0b00000001 );
    value >>= 1;

    SetSubtract( false );
    SetHalfCarry( false );
    SetZero( value == 0x00 );
    memory_bus->Write8(addr, value);
}
void Processor::sra(Reg8& reg)
{
    SetCarry( reg & 0b00000001 );
    reg >>= 1;
    reg |= (reg & 0b01000000) << 1;

    SetSubtract( false );
    SetHalfCarry( false );
    SetZero( reg == 0x00 );
}
void Processor::sraAt(u16 addr)
{
    u8 value = memory_bus->Read8(addr);
    SetCarry( value & 0b00000001 );
    value >>= 1;
    value |= (value & 0b01000000) << 1;

    SetSubtract( false );
    SetHalfCarry( false );
    SetZero( value == 0x00 );
    memory_bus->Write8(addr, value);
}

// bit
void Processor::bit(u8 byte, u8 bit)
{
    SetSubtract( false );
    SetHalfCarry( true );
    SetZero(true);//(byte & (0x1 << bit)) == 0 );
}

// swap
void Processor::swap(Reg8& reg)
{
    reg = ((reg & 0x0F) << 4) | ((reg & 0xF0) >> 4);

    SetSubtract( false );
    SetHalfCarry( false );
    SetCarry( false );
    SetZero( reg == 0 );
}
void Processor::swapAt(u16 addr)
{
    u8 value = memory_bus->Read8(addr);
    value = ((value & 0x0F) << 4) | ((value & 0xF0) >> 4);

    SetSubtract( false );
    SetHalfCarry( false );
    SetCarry( false );
    SetZero( value == 0 );
    memory_bus->Write8(addr, value);
}

// complement reg
void Processor::cpl(Reg8& reg)
{
    reg = ~reg;
    SetSubtract( true );
    SetHalfCarry( true );
}
void Processor::ccf()
{
    SetCarry( !Carry() );
    SetSubtract( false );
    SetHalfCarry( false );
}

// set carry
void Processor::scf()
{
    SetCarry( true );
    SetSubtract( false );
    SetHalfCarry( false );
}

// reset bit
void Processor::res(Reg8& reg, u8 bit)
{
    reg &= ~(0x1 << bit);
}
void Processor::resAt(u16 addr, u8 bit)
{
    memory_bus->Write8(addr, memory_bus->Read8(addr) & ~(0x1 << bit));
}

// set bit
void Processor::set(Reg8& reg, u8 bit)
{
    reg |= (0x1 << bit);
}
void Processor::setAt(u16 addr, u8 bit)
{
    memory_bus->Write8(addr, memory_bus->Read8(addr) | (0x1 << bit));
}

}; // namespace Core
