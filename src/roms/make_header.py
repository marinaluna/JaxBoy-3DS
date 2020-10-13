import os
import struct
import numpy

ROM_PATH = "./rom.gb"

def main():
    rom = open(ROM_PATH, "rb")
    header = open("./rom.h", "w")
    rom_size = 0

    try:
        rom_size = os.path.getsize(ROM_PATH)
    except FileNotFoundError:
        print("ERROR: " + ROM_PATH + "Not found! Does it exist?")
    except OSError:
        print("ERROR: Could not get size of " + ROM_PATH + "! Does it exist?")
    finally:
        print("Opening " + ROM_PATH + " size " + str(rom_size))

    try:
        for i in range(rom_size // 16):
            for j in range(16):
                byte = struct.unpack('b', rom.read(1))[0]
                if(byte == ""):
                    break
                header.write(hex(numpy.uint8(byte)))
                if(((i * 16) + j) != rom_size - 1):
                    header.write(',')
            header.write("\n")
    finally:
        rom.close()
        header.close()

main()
