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

#include "SDLContext.h"
#include "ThreadArgs.h"

#include "core/GameBoy.h"

#include "common/Types.h"
#include "common/Globals.h"

#include <3ds.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>

static u8 bootrom_raw[] = {
	#include "roms/DMG_ROM.h"
};

static u8 rom_raw[] = {
	#include "roms/rom.h"
};


int main(int argc, char* argv[])
{
    gfxInit(GSP_RGBA8_OES, GSP_RGBA8_OES, false);
    gfxSetDoubleBuffering(GFX_TOP, true);
    consoleInit(GFX_BOTTOM, NULL);


    std::vector<u8> rom(rom_raw, rom_raw + sizeof(rom_raw));
    std::vector<u8> bootrom(bootrom_raw, bootrom_raw + sizeof(bootrom_raw));


    // Setup system options
    Core::GameBoy::Options options;

    int width = 160;
    int height = 144;
    // Create the system instance
    Core::GameBoy* gameboy = ( new Core::GameBoy(options, width, height, rom, bootrom) );
    // Initalize Render Context
    FrontEnd::SDLContext* sdl_context = new FrontEnd::SDLContext(width, height, options.scale, gameboy);

    printf("Welcome to JaxBoy 3DS!\n");

    // TODO: Proper mutexes
    bool update_frame = true;
    bool poll_events = true;
    // Start the SDL thread
    Thread sdl_thread;
    Thread core_thread;

    s32 main_thread_prio = 0;
    svcGetThreadPriority(&main_thread_prio, CUR_THREAD_HANDLE);
    printf("Main thread priority: 0x%lx\n", main_thread_prio);

    ThreadArgs thread_args {sdl_context, gameboy, &update_frame, &poll_events};

    /* Thread function, args, stack size, priority, cpu core, detached */
    //sdl_thread = threadCreate(FrontEnd::SDLContext::ThreadMain, (void*) &thread_args, 4096, main_thread_prio-1, -2, false);

    // Start the main thread
    {
        while(aptMainLoop())
        {
            gameboy->Cycle();

            //sdl_context->Update(gameboy->GetPPU()->GetBackBuffer());
            /*update_frame = true;

            if(poll_events) {
                sdl_context->PollEvents(gameboy);
                poll_events = false;
            }*/
        }
    }

    //threadJoin(sdl_thread, U64_MAX);
    //threadFree(sdl_thread);
    if(gameboy)
        delete gameboy;
    if(sdl_context) {
        //sdl_context->Destroy();
        //delete sdl_context;
    }

    gfxExit();

    return 0;
}
