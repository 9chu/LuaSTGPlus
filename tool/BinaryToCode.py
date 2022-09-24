#!env python3
# -*- coding: utf-8 -*-
# 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
import argparse
import re
import sys


INDENT_LEN = 4
LINE_SIZE = 120
BYTE_LEN = len("0x00, ")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--input", required=True, type=str, help="Input file to be converted")
    parser.add_argument("-n", "--name", required=True, type=str, help="Variable name")
    parser.add_argument("-o", "--output", required=True, type=str, help="Output filename")

    args = parser.parse_args()

    if not re.match("[a-zA-Z_][a-zA-Z0-9_]*", args.name):
        print("Invalid identifier '%s'" % args.name, file=sys.stderr)
        return

    # Read input file
    with open(args.input, "rb") as f:
        data = f.read()

    with open(args.output, "w", encoding="utf-8") as fout:
        fout.write("#include <cstdint>\n")
        fout.write("\n")
        fout.write("alignas(16) extern const uint8_t %s[%d] = {\n" % (args.name, len(data)))
        line = []
        for byte in data:
            line.append("0x%02xu, " % byte)
            if INDENT_LEN + (len(line) + 1) * BYTE_LEN > LINE_SIZE:
                fout.write(" " * INDENT_LEN)
                fout.write("".join(line))
                fout.write("\n")
                line.clear()
        if len(line) > 0:
            fout.write(" " * INDENT_LEN)
            fout.write("".join(line))
            fout.write("\n")
            line.clear()
        fout.write("};\n")


if __name__ == "__main__":
    main()
