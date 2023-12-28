#!/usr/bin/env python3

def main(args):
    try:
        path = args[1]
        print(path)
        with open(path, mode='r', encoding='utf-8') as jsonfile: 
            lines = jsonfile.readlines()
    except:
        print("failed reading file")
        return -1

    # my_str = "thisissometextthatiwrote"
    # substr = "text"
    # inserttxt = "XX"

    # my_str.replace(substr, substr + inserttxt)

    oneliner = str()
    for line in lines:
        for word in line.split():
            oneliner += word

    oneliner = oneliner.replace("\"", "\\\"")
    print(oneliner)
    # print("\\\"")

    return 0

if __name__ == '__main__':
    import sys
    import re
    import random
    import argparse
    import json
    sys.exit(main(sys.argv))