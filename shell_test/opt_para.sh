#! /bin/sh

while getopts "ab:" opt; do
	case $opt in
		a)
			echo "this is opt -a. $OPTARG"
			;;
		b)
			echo "this is opt -b. $OPTARG"
			;;
		\?)
			echo "invalid option: -$OPTARG"
			;;
	esac
done

# description: shuhuan 2019/8/8
# getopts optstring name [args]
# optstring [ab:] There is a ":" follow opt b that means b opt should be followed a para.
# optstring maybe [:ab:] and [ab:] There is a ":" before opt a that means no error infomations are output when b opt isn't followed by a para
# there are two kind of errors. one is "invalid option" and other is "miss option argument".
			
