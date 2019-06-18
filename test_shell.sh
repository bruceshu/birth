#! /bin/sh

echo "hello world"
echo "===== test para ====="
echo "the num of para is:$#"
echo "the content of para is:$*"
echo "the :$@"
echo "the result of last is:$?"
echo "===== test > and >> ====="
echo "scene one" >standard.txt
echo "scene two" >error.txt 2>&1
