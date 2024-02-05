import sys

def generate_c_array(input_file, output_file, array_name):
    with open(input_file, 'rb') as binary_file:
        binary_data = binary_file.read()

    with open(output_file, 'w') as c_file:
        c_file.write(f'const uint8_t {array_name}[] PROGMEM = {{')
        for i, byte in enumerate(binary_data):
            c_file.write(f'0x{byte:02X},')
        c_file.write('};\n')

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python script.py input_binary_file output_c_file array_name")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]
    array_name = sys.argv[3]

    generate_c_array(input_file, output_file, array_name)

    print(f"C array generated and saved to {output_file} with the name {array_name}")

