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

#include "common/Types.h"

#include <SDL.h>
#include <vector>

namespace Core {
class GameBoy;
}; // namespace Core


namespace FrontEnd {

class SDLContext
{
    // window size
    int width;
    int height;
    int scale;
    // SDL Components
    SDL_Surface* screen;
    SDL_Event window_event;

    bool Stopped = false;

public:
    SDLContext(int width, int height, int scale, Core::GameBoy* gameboy);
    void Destroy();

    void Stop()
        { Stopped = true;}
    bool IsStopped()
        { return Stopped; }

    void Update(std::vector<Color>& back_buffer);
    void PollEvents(Core::GameBoy* gameboy);
    // Thread entry point
    static void ThreadMain(void* arg);
};

}; // namespace FrontEnd
