#!/usr/bin/env python3

import array
import argparse
import json

def append_string(buffer, string):
    buffer.append(len(string))
    if (len(string) == 0):
        return False
    for c in string:
        buffer.append(ord(c))
    return True


def main():
    parser = argparse.ArgumentParser(description='Read JSON array from file')
    parser.add_argument('-f', '--file', default=None, type=str, help='Path to the JSON file')
    parser.add_argument('-w', '--wrap', default=False, action="store_true", help='Wrap array in C variable declaration')

    args = parser.parse_args()

    if args.file is None:
        args.file = "config/tiles.json"

    with open(args.file, 'r') as file:
        columns = json.load(file)
    
    # Create an array of unsigned integers (uint8)
    buffer = array.array('B')

    tiles = []
    headings = []
    for c in columns:
        headings.append({
            "heading": c.get("heading"),
            "icon_idx": c.get("icon_idx")
        })
        tiles.append(c.get("tile_a"))
        tiles.append(c.get("tile_b"))
        tiles.append(c.get("tile_c"))

    buffer.append(len(headings))
    for heading in headings:
        if heading is None:
            buffer.append(0)
            continue
        append_string(buffer, heading["heading"])
        buffer.append(heading["icon_idx"])
        
    buffer.append(len(tiles))
    for tile in tiles:
        if tile is None:
            buffer.append(0)
            continue
        append_string(buffer, tile["name"])
        buffer.append(tile["image_idx"])
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

    if args.wrap:
        print(f"static const char tiles_data[{len(buffer)}] PROGMEM = {{\n    {', '.join(str(b) for b in buffer)}\n}};")
    else:
        print(','.join(str(b) for b in buffer), end="")

if __name__ == "__main__":
    main()