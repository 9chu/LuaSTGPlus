#!env python3
# -*- coding: utf-8 -*-
import os
import argparse
import fnmatch


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--output", required=True, type=str, help="Output file list")
    parser.add_argument("-s", "--source", required=True, type=str, help="Source directory")
    parser.add_argument("matches", type=str, nargs='+', help="Glob matches")

    args = parser.parse_args()

    with open(args.output, "w", encoding="utf-8") as f:
        for root, dirs, files in os.walk(args.source):
            for filename in files:
                full_path = os.path.join(root, filename)
                relative_path = os.path.relpath(full_path, args.source)

                match = True
                for m in args.matches:
                    if not fnmatch.fnmatch(relative_path, m):
                        match = False
                        break

                if match:
                    f.write(relative_path)
                    f.write('\n')


if __name__ == "__main__":
    main()
