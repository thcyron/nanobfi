nanobfi – Brainfuck Interpreter
===============================

**nanobfi** is a [Brainfuck](http://en.wikipedia.org/wiki/Brainfuck)
interpreter written in ANSI C. It uses 8-bit data cells of type
`uint8_t`.

Features
--------

* Increase data memory on demand
* Protect from accessing foreign memory
* Protect from interger underflows and overflows
* Basic detection of unbalanced brackets
* Debug mode to print each step along with current position,
  loop nesting level, pointer and pointer value

Usage
-----

    Usage: nanobfi [-dhn] [-i inc] [-s size] [file]

    Options:
        -d       Enable debug mode
        -h       Show this help
        -i inc   Increment memory by <inc> bytes (default: 100)
        -n       Do not increase memory on demand (fixed memory size)
        -s size  Initialize data memory with <size> bytes (default: 30000)

Examples
--------

    % nanobfi helloworld.b
    Hello World

    % echo "x*<" | nanobfi
    nanobfi: trying to access foreign memory at 0x1008075ff

    % echo "+[-[]]]" | nanobfi
    nanobfi: unbalanced brackets

    % echo "+[-[><]]]" | nanobfi -d
    (1) command + pos 0 level 0 ptr 0 *ptr 0
    (2) command [ pos 1 level 0 ptr 0 *ptr 1
    (3) command - pos 2 level 1 ptr 0 *ptr 1
    (4) command [ pos 3 level 1 ptr 0 *ptr 0
    (5) command ] pos 7 level 1 ptr 0 *ptr 0
    (6) command ] pos 8 level 0 ptr 0 *ptr 0
    nanobfi: unbalanced brackets
