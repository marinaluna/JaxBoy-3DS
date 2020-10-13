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

#include "PPU.h"
#include "memory/MemoryBus.h"

#include "../common/Globals.h"

#include <3ds.h>

#include <cstdio>


namespace Core {

PPU::PPU(GameBoy* gameboy, int width, int height,
         std::shared_ptr<Memory::MemoryBus>& memory_bus)
:
    gameboy (gameboy),
    memory_bus (memory_bus),
    width (width),
    height (height)
{
    // initialize buffers
    back_buffer = std::vector<Color>(width * height);
    BGTileset = std::vector<Graphics::Tile>(256);
    OBJTileset = std::vector<Graphics::Tile>(256);
    // Start in DISPLAY_VBLANK
    STAT |= DISPLAY_VBLANK;
    LCDC = 0x91;
    // Setup blank palettes
    BGPalette[0] = BGPalette[1] = BGPalette[2] = BGPalette[3] = gColors[0x00];
    OBJ0Palette[0] = OBJ0Palette[1] = OBJ0Palette[2] = OBJ0Palette[3] = gColors[0x00];
    OBJ1Palette[0] = OBJ1Palette[1] = OBJ1Palette[2] = OBJ1Palette[3] = gColors[0x00];
}

std::vector<Color>& PPU::GetBackBuffer()
{
    return back_buffer;
}

int PPU::Update(int cycles)
{
    int return_code = 0;
    if(LCDC & 0x80)
    {
        frameCycles += cycles;
        switch(STAT & 0x03)
        {
            case DISPLAY_HBLANK:
                // TODO: Accurate cycles?
                if(frameCycles > 207)
                {
                    // Draw this scanline
                    DrawScanline();
                    // Carry leftover cycles into next mode
                    frameCycles %= 207;
                    if(++LY == 144)
                    {
                        // At the last line; enter V-Blank
                        STAT = (STAT & ~0x03) | DISPLAY_VBLANK;
                        // request V-Blank interrupt
                        memory_bus->Write8(0xFF0F, memory_bus->Read8(0xFF0F) | 0x01);
                    }
                    else
                    {
                        // Proceed to the next line
                        STAT = (STAT & ~0x03) | DISPLAY_OAMACCESS;
                    }

                    if(LY == LYC) {
                        // If LY == LYC set the coincidence bits in STAT and trigger the interrupt
                        // (I don't know why there are two coincidence bits)
                        STAT |= 0x44;
                        memory_bus->Write8(0xFF0F, memory_bus->Read8(0xFF0F) | 0x02);
                    } else {
                        STAT &= ~0x44;
                    }
                }
                break;
            case DISPLAY_VBLANK:
                // Have we completed a scanline?
                if((static_cast<int>(frameCycles / 465) + 144) > LY)
                {
                    if(++LY > 153)
                    {
                        frameCycles %= 4560;
                        STAT = (STAT & ~0x03) | DISPLAY_OAMACCESS;
                        LY = 0;
                        // Flush and swap framebuffers
                        gfxFlushBuffers();
                        gfxSwapBuffers();

                        //Wait for VBlank
                        gspWaitForVBlank();
                    }
                }
                break;
            case DISPLAY_OAMACCESS:
                if(frameCycles > 83)
                {
                    FetchScanlineSprites();
                    frameCycles %= 83;
                    STAT = (STAT & ~0x03) | DISPLAY_UPDATE;
                }
                break;
            case DISPLAY_UPDATE:
                if(frameCycles > 175)
                {
                    frameCycles %= 175;
                    STAT = (STAT & ~0x03) | DISPLAY_HBLANK;
                    DecodeTiles();
                }
                break;
        }
    }
    else
    {
        // If LCDC is disabled, reset all this stuff
        frameCycles = 0;
        LY = 0;
    }

    return return_code;
}

void PPU::DrawScanline()
{
    for(int x = 0; x < width; x++)
    {
        int y = LY;

        // Tile and pixel to draw
        u8 tileY = y / 8;
        u8 tileX = x / 8;
        u8 pixelY = y % 8;
        u8 pixelX = x % 8;
        // Scroll offsets
        u8 tileYoff = SCY / 8;
        u8 tileXoff = SCX / 8;
        u8 pixelYoff = SCY % 8;
        u8 pixelXoff = SCX % 8;
        // boundary between the scroll tiles
        u8 upperHalf = (8 - pixelYoff);
        u8 leftHalf = (8 - pixelXoff);
        // tile to take from the BG map
        u8 fetchY = tileY+tileYoff;
        u8 fetchX = tileX+tileXoff;
        if(pixelY >= upperHalf)
        {
            // start drawing from the top
            // of the next tile down
            fetchY++;
            pixelY -= upperHalf;
            pixelYoff = 0;
        }
        if(pixelX >= leftHalf)
        {
            fetchX++;
            pixelX -= leftHalf;
            pixelXoff = 0;
        }
        // wrap around
        if(fetchY >= 32)
            fetchY %= 32;
        if(fetchX >= 32)
            fetchX %= 32;
        // fetch the tile to draw
        u16 base = 0x9800;
        if(LCDC & 0x08) {
            base += 0x0400;
        }
        u8 tileID = memory_bus->Read8(base + (fetchY * 32) + fetchX);
        // Draw the pixel
        int drawY = LY * width;
        int drawX = x;

        u16 screenWidth;
        u16 screenHeight;
        Color* fb = (Color*) gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &screenHeight, &screenWidth);
        fb[(x * screenHeight) + (height - LY + (screenWidth * 2))] = BGPalette[BGTileset[tileID].GetPixel(pixelX+pixelXoff, pixelY+pixelYoff)];
    }

    if((LCDC & 0x20) && LY >= WY) {
        DrawScanlineWindow();
    }
    if(LCDC & 0x02) {
        DrawScanlineSprites();
    }
}

void PPU::DrawScanlineWindow()
{
    // TODO: Track progress since window drawing
    // can be stopped and started again at a later LY

    // TODO: It seems I've reached my first impass not
    // having perfect cycle-count accuracy:
    // If the window is disabled partway down the screen,
    // it doesn't draw the last line of the window.
    // (window is disabled before window finishes drawing)
    for(int x = WX; x < width + 7; x++)
    {
        int y = LY;
        // Tile and pixel to draw
        u8 tileY = (LY - WY) / 8;
        u8 tileX = (x - WX) / 8;
        u8 pixelY = (LY - WY) % 8;
        u8 pixelX = (x - WX) % 8;
        // fetch the tile to draw
        u16 base = 0x9800;
        if(LCDC & 0x40) {
            base += 0x0400;
        }
        u8 tileID = memory_bus->Read8(base + (tileY * 32) + tileX);
        // Draw the pixel
        int drawY = y * width;
        int drawX = x - 7;
        if(drawX < 0)
            continue;
        
        back_buffer[drawY + drawX] = BGPalette[BGTileset[tileID].GetPixel(pixelX, pixelY)];
    }
}

void PPU::DrawScanlineSprites()
{
    const int SPRITE_HEIGHT = (LCDC & 04)? 16 : 8;

    for(auto it = ScanlineSprites.begin(); it != ScanlineSprites.end(); it++)
    {
        Graphics::Sprite& sprite = *it;
        // offset by 16 to align with Sprite y
        int adjScanline = LY + 16;
        int y = sprite._y;
        int x = sprite._x;
        const Color* palette = (sprite.palette == 0)? OBJ0Palette : OBJ1Palette;
        for(int px = 0; px < 8; px++)
        {
            // don't draw the x pixels if they are offscreen
            if(x+px < 8 || x+px >= 168)
                continue;
            // flip sprites
            int oamX = (sprite.flipX)? (7 - px) : px;
            int oamY = (sprite.flipY)? ((SPRITE_HEIGHT - 1) - (adjScanline - y)) : (adjScanline - y);
            u8 color = OBJTileset[sprite.id].GetPixel(oamX, oamY);
            // 00 is transparent for sprites: use the color of the background instead
            if(color == 0x00)
                continue;
            int drawY = LY * width;
            int drawX = (x - 8) + px;
            back_buffer[drawY + drawX] = palette[color];
        }
    }
    ScanlineSprites.clear();
}

void PPU::FetchScanlineSprites()
{
    const int OAM_SIZE = 4;
    const int OAM_COUNT = 40;
    const int SPRITE_HEIGHT = (LCDC & 04)? 16 : 8;

    int spriteCounter = 0;
    for(int i = 0; i < OAM_COUNT; i++)
    {
        Graphics::Sprite sprite;
        u8 buffer[4];
        memory_bus->ReadBytes(buffer, 0xFE00 + (i * OAM_SIZE), OAM_SIZE);
        sprite.Decode(buffer);
        // offset by 16 to align with Sprite y
        u8 adjScanline = LY + 16;
        u8 y = sprite._y;
        u8 x = sprite._x;
        // if the sprite is offscreen
        // sprites start at (8, 16) so you can scroll them in
        if((y == 0 || y >= 160) || (x == 0 || x >= 168))
            continue;
        // if the sprite is not within range of this scanline
        if(adjScanline < y || (adjScanline - y) >= SPRITE_HEIGHT)
            continue;
        // only 10 sprites per scanline
        if(++spriteCounter > 10)
            return;

        ScanlineSprites.push_back(sprite);
    }
}

void PPU::DecodeTiles()
{
    // fetch each tile in the tileset from VRAM
    // and decode their 2-bit colors
    const int BG_SIZE = 16;
    const int OBJ_SIZE = 16;

    // Background Tileset
    for(int bg = 0; bg < 256; bg++)
    {
        // if LCDC bit 4 == 0,
        // BG tiles 00-7F start at 0x9000
        // and 80-FF start at 0x8800
        // Otherwise, BG tiles align with
        // OBJ tiles at 0x8000
        u16 base = 0x8000;
        u8 rawTile = bg;
        if((LCDC & 0x10) == 0)
        {
            if(bg >= 128)
            {
                base = 0x8800;
                // start fetching from 0x8800,
                // not 0x8800 + 128 tiles
                rawTile -= 128;
            }
            else
                base = 0x9000;
        }

        u8 buffer[BG_SIZE];
        memory_bus->ReadBytes(buffer, base + (rawTile * BG_SIZE), BG_SIZE);
        BGTileset.at(bg).Decode(buffer);
    }
    // Sprite tileset
    for(int obj = 0; obj < 256; obj++)
    {
        u8 buffer[OBJ_SIZE];
        memory_bus->ReadBytes(buffer, 0x8000 + (obj * OBJ_SIZE), OBJ_SIZE);
        OBJTileset.at(obj).Decode(buffer);
    }
}

}; // namespace Core
