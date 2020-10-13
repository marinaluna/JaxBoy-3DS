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

#pragma once
#include "Rom.h"
#include "PPU.h"
#include "processor/Processor.h"

#include "../common/Types.h"

#include <memory>
#include <vector>


namespace Memory {
    class MemoryBus;
}; // namespace Memory

namespace Core {
class Processor;
class PPU;
class Rom;

class GameBoy
{
public:
    static const int FRAMELIMITER_MAX = 50;
    int framelimiter = FRAMELIMITER_MAX;

    struct Options
    {
        bool debug = false;
        int scale = 1;
        int force_mbc = -1;
        bool skip_bootrom = false;
        bool framelimiter_hack = false;
    };
    Options& GetOptions()
        { return _Options; }

    GameBoy(GameBoy::Options& options,
            int width,
            int height,
            const std::vector<u8>& rom,
            const std::vector<u8>& bootrom);

    void Cycle();
    void Stop()
        { Stopped = true; }
    bool IsStopped()
        { return Stopped == true; }
    bool IsInBootROM()
        { return InBootROM; }
    std::unique_ptr<Rom>& GetCurrentROM()
        { return game_rom; };
    std::unique_ptr<PPU>& GetPPU()
        { return ppu; }

    void UpdateKeys();
    void KeyPressed(u8 key);
    void KeyReleased(u8 key);

    bool SpeedEnabled = false;
    void EnableSpeed();
    void DisableSpeed();

    void SystemError(const std::string& error_msg);

private:
    friend class Memory::MemoryBus;

    // P1 IO Register
    u8 P1;
    // Keys currently pressed
    u8 Keys;
    // Timer registers
    u8 TIMA; // timer counter; incremented at TAC frequency
    u8 TMA;  // timer modulo; when timer overflows, loads this value
    u8 TAC;  // timer control; control speed of TIMA

    // Options configuration
    GameBoy::Options _Options;
    // Components
    std::unique_ptr<Processor> processor;
    std::unique_ptr<PPU> ppu;
    std::unique_ptr<Rom> game_rom;
    // System memory map
    std::shared_ptr<Memory::MemoryBus> memory_bus;

    bool InBootROM = false;
    bool Stopped = false;
};

}; // namespace Core
