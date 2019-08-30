# Copyright (c) 2019 shuhuan shuhuan.bruce@gmail.com
#

# 编译工具
CC=gcc

# 编译选项
CFLAGS=

# 依赖的系统库
LDFLAGS=

# 依赖的头文件
INCLUDE=

# 依赖的第三方库
LIBS=


source=$(wildcard *.c)
#$(warning $(source))

obj=$(subst .c,.o,$(source))
#$(warning $(obj))

all:out

$(obj):%.o:%.c
	$(CC) -c $< -o $@

out:$(obj)
	$(CC) -o $@ $^

clean:
	rm -rf out $(obj) 

.PHONY: all clean

