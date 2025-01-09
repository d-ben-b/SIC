# 使用 GCC 編譯器
CC = gcc

# 可追加的編譯選項
CFLAGS = -Wall -Wextra -O2

# 專案目標
TARGET = assembler

# 原始程式碼檔
SRCS = assembler.c

# 產生的目標檔（物件檔）
OBJS = $(SRCS:.c=.o)

# 預設執行 make 時，會執行 all 目標
.PHONY: all clean

all: $(TARGET)

# 編譯規則：.o 檔如何由 .c 檔而來
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 連結規則：產生執行檔
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# 清理編譯產生的檔案
clean:
	rm -f $(OBJS) $(TARGET)
