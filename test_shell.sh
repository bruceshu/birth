#! /bin/sh

echo "hello world"
echo "===== test para ====="
echo "the num of para is:$#"
echo "the content of para is:$*"
echo "the :$@"
echo "the result of last is:$?"
echo "===== test > and >> ====="
echo "scene one" 1>&2
echo "scene two" 1>&2
