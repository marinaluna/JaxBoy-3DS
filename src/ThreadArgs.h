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

namespace FrontEnd {
class SDLContext;
};

namespace Core {
class GameBoy;
}

/* Argument wrapper for the ctru threads */
struct ThreadArgs {
    FrontEnd::SDLContext* sdl_context;
    Core::GameBoy* gameboy;
    bool* update_frame;
    bool* poll_events;
};
