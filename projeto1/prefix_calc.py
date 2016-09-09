#!/usr/bin/python3
# -*- coding: utf-8 -*-

"""prefix_calc.py

A simple prefix calculator. Usage:

    while read line ; do python prefix_calc.py "$line" ; done < tests.in

"""

from sys import argv


def process_input(_input):
    
    stack = []
    operations = {
        '+': lambda x, y: x + y,
        '-': lambda x, y: x - y,
        '*': lambda x, y: x * y,
        '/': lambda x, y: x / y,
    }

    for token in _input.split()[::-1]:
        if token in operations:
            stack.append(operations[token](stack.pop(), stack.pop()))
        else:
            stack.append(float(token))

    return stack.pop()

if __name__ == '__main__':
    print(process_input(argv[1]))
