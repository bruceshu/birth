#! /bin/sh

echo "test command > [filename]"
echo "input content into file" > InFile.txt

echo "test command >> [filename]"
echo "add other content into the tail of the file" >> InFile.txt

echo "test error log to standard pipeline. 2 >&1"
echo "error log" > log 2>&1
