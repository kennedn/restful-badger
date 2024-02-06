#!/usr/bin/env python3
import sys
from pathlib import Path


def generate_c_array(input_file, array_name):
    with open(input_file, 'rb') as binary_file:
        binary_data = binary_file.read()

        print(f'const uint8_t {array_name}[] PROGMEM = {{')
        for i, byte in enumerate(binary_data):
            if i == 0:
                print("\t", end="")
            print(f'0x{byte:02X},', end="")
        print('\n};')

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py bin")
        sys.exit(1)

    input_file = sys.argv[1]
    array_name = Path(input_file).stem

    generate_c_array(input_file, array_name)

