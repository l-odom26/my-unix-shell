# My Unix Shell (mysh)

A simple Unix shell written in C for an Operating Systems course.

## Features
- Command execution (fork + exec)
- Built-in commands: cd, exit
- Output redirection (>)
- Input redirection (<)
- Pipes (|)
- Multiple pipe support (bonus)

## Example Usage

ls | wc -l
sort < input.txt > output.txt

## How to Compile

gcc -o mysh mysh.c
./mysh
