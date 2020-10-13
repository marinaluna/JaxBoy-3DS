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
#include "core/PPU.h"
#include "core/Rom.h"

#include <stdexcept>
#include <string>


namespace FrontEnd {

SDLContext::SDLContext(int width, int height, int scale, Core::GameBoy* gameboy)
: width(width),
  height(height),
  scale(scale)
{
    /*if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        //throw std::runtime_error("Error initializing render context! " + std::string(SDL_GetError()));
    }

    screen = SDL_SetVideoMode(width*scale, height*scale, 32, SDL_TOPSCR | SDL_CONSOLEBOTTOM | SDL_HWSURFACE);

    if(!screen) {
        SDL_Quit();
        //throw std::runtime_error("Error creating window! " + std::string(SDL_GetError()));
    }

    SDL_ShowCursor(SDL_DISABLE);*/
}

void SDLContext::Destroy()
{
    SDL_Quit();
}

void SDLContext::Update(std::vector<Color>& back_buffer)
{
    memcpy((void*) screen->pixels,
           (void*) back_buffer.data(),
           width*height*sizeof(Color));

    SDL_Flip(screen);
}

// This is actually called on the main thread.
// I hate that I have to do this, but SDL_PollEvent
// can only be executed on the main thread
void SDLContext::PollEvents(Core::GameBoy* gameboy)
{
    while(SDL_PollEvent(&window_event)) {
        switch(window_event.type) {
        case SDL_QUIT:
            Stop();
            break;
        /*case SDL_KEYDOWN:
            switch(window_event.key.keysym.sym) {
            case SDLK_o:
                gameboy->KeyPressed(Key::KEY_A); break;
            case SDLK_p:
                gameboy->KeyPressed(Key::KEY_B); break;
            case SDLK_QUOTE:
                gameboy->KeyPressed(Key::KEY_SELECT); break;
            case SDLK_RETURN:
                gameboy->KeyPressed(Key::KEY_START); break;
            case SDLK_w:
                gameboy->KeyPressed(Key::KEY_UP); break;
            case SDLK_s:
                gameboy->KeyPressed(Key::KEY_DOWN); break;
            case SDLK_a:
                gameboy->KeyPressed(Key::KEY_LEFT); break;
            case SDLK_d:
                gameboy->KeyPressed(Key::KEY_RIGHT); break;
            case SDLK_SPACE:
                gameboy->EnableSpeed(); break;
            }
            break;
        case SDL_KEYUP:
            switch(window_event.key.keysym.sym) {
            case SDLK_o:
                gameboy->KeyReleased(Key::KEY_A); break;
            case SDLK_p:
                gameboy->KeyReleased(Key::KEY_B); break;
            case SDLK_QUOTE:
                gameboy->KeyReleased(Key::KEY_SELECT); break;
            case SDLK_RETURN:
                gameboy->KeyReleased(Key::KEY_START); break;
            case SDLK_w:
                gameboy->KeyReleased(Key::KEY_UP); break;
            case SDLK_s:
                gameboy->KeyReleased(Key::KEY_DOWN); break;
            case SDLK_a:
                gameboy->KeyReleased(Key::KEY_LEFT); break;
            case SDLK_d:
                gameboy->KeyReleased(Key::KEY_RIGHT); break;
            case SDLK_SPACE:
                gameboy->DisableSpeed(); break;
            }
            break;*/
        }
    }
}

void SDLContext::ThreadMain(void* arg) {
    ThreadArgs* args_ptr = (ThreadArgs*) arg;
    FrontEnd::SDLContext* sdl_context = args_ptr->sdl_context;
    Core::GameBoy* gameboy = args_ptr->gameboy;
    bool update_frame = *(args_ptr->update_frame);
    bool poll_events = *(args_ptr->poll_events);

    while(!gameboy->IsStopped() && !sdl_context->IsStopped()) {
        if(update_frame) {
            sdl_context->Update(gameboy->GetPPU()->GetBackBuffer());
            update_frame = false;
            poll_events = true;
        }
    }
}

}; // namespace FrontEnd
