import array
import argparse
import json

def append_string(buffer, string):
    buffer.append(len(string))
    for c in string:
        buffer.append(ord(c))


def main():
    parser = argparse.ArgumentParser(description='Read JSON array from file')
    parser.add_argument('-f', '--file', required=True, type=str, help='Path to the JSON file')

    args = parser.parse_args()

    with open(args.file, 'r') as file:
        tiles = json.load(file)
    
    # Create an array of unsigned integers (uint8)
    buffer = array.array('B')


    buffer.append(len(tiles))
    for tile in tiles:
        buffer.append(tile["image_idx"])
        append_string(buffer, tile["name"])
        request_mask = 0
        if "action_request" in tile:
            request_mask |= 1 << 0
        if "status_request" in tile:
            request_mask |= 1 << 1
        buffer.append(request_mask)
        if "action_request" in tile:
            request = tile["action_request"]
            append_string(buffer, request["method"])
            append_string(buffer, request["endpoint"])
            append_string(buffer, request["json_body"])
        if "status_request" in tile:
            request = tile["status_request"]
            append_string(buffer, request["method"])
            append_string(buffer, request["endpoint"])
            append_string(buffer, request["json_body"])
            append_string(buffer, request["key"])
            append_string(buffer, request["on_value"])
            append_string(buffer, request["off_value"])

    # for byte in buffer:
    #     if 33 <= byte <= 126:
    #         print(f"'{chr(byte)}'", end=",")
    #     else:
    #         print(byte, end=",")
    # print()

    print(f"static const char tiles_data[{len(buffer)}] PROGMEM = {{\n    {', '.join(f'0x{b:02x}' for b in buffer)}\n}};")


if __name__ == "__main__":
    main()