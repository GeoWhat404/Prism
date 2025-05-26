#!/usr/bin/python3

import os

def count_lines_in_file(file_path):
    with open(file_path, 'r', errors='ignore') as file:
        return sum(1 for _ in file)

def main():
    total_lines = 0
    file_count = 0

    for root, _, files in os.walk('.'):
        for file in files:
            if file.endswith('.c') or file.endswith('.h') or file.endswith('.asm') or file.endswith('.inc'):
                file_path = os.path.join(root, file)

                if file_path.__contains__('limine'):
                    continue

                line_count = count_lines_in_file(file_path)

                print(f"File: {file_path} - {line_count} lines")

                total_lines += line_count
                file_count += 1
    print(f"Total lines in all {file_count} files: {total_lines}")

if __name__ == "__main__":
    main()

