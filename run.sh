#!/bin/bash

# Biên dịch chương trình chính
make all

# Biên dịch file capture.c
gcc capture.c -o capture

# Chạy chương trình capture
./capture
