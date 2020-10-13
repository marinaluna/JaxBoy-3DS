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
#include "Types.h"

// constants

// Colors are in little endian because of
// how I'm swapping buffers
const Color gColors[4] =
{
    // White
    0x9BBC0FFF,
    // Light Grey
    0X8BAC0FFF,
    // Dark Grey
    0x306230FF,
    // Black
    0x0F380FFF
};
