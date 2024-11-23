#!/usr/bin/env python3

from sys import argv
import os, re

# usage: $0 <input-dir> <output-dir> <json-file>

with open(argv[3], "w") as f:
    f.write('{ "images": [\n')

    file_list = os.listdir(argv[1])
    i = 0
    for level_file in file_list:
        strmatch = re.fullmatch(r'(\d+).png', level_file)
        if not strmatch:
            continue
        level_id = strmatch.group(1)

        f.write('{')
        f.write(f'"name": "level_{level_id}",')

        f.write(f'"input": "{argv[1]}/{level_id}.png",')
        f.write(f'"output": "{argv[2]}/{level_id}.c",')

        f.write(f'"palette": "{argv[1]}/pix-to-tile.png",')
        f.write('"bpp": 8')

        f.write('}')

        i += 1
        if i < len(file_list) - 1:
            f.write(',\n')
    f.write(']}')
