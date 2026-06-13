# =================================================
# 1. DETEKSI OTOMATIS SISTEM OPERASI (OS DETECTION)
# =================================================

# $(OS) adalah variabel lingkungan bawaan yang selalu bernilai "Windows_NT" di Windows
ifeq ($(OS),Windows_NT)
    # --------------------------
    # KONFIGURASI KHUSUS WINDOWS
    # --------------------------
    CC = gcc
    TARGET = big_challenge.exe

    # Perintah clean menggunakan 'del' bawaan CMD Windows dengan separator backslash (\)
    CLEAN_CMD = del /q /f src\main.o src\heap.o src\utils.o $(TARGET) 2>nul || exit 0

else
    # ---------------------------------------
    # KONFIGURASI KHUSUS UNIX (macOS / LINUX)
    # ---------------------------------------
    CC = clang
    TARGET = big_challenge
	
    # Perintah clean menggunakan 'rm' bawaan Unix
    CLEAN_CMD = rm -f $(OBJS) $(TARGET)
endif

# ======================================================
# 2. CONFIGURASI COMPILER FLAGS (BERLAKU UNTUK KEDUA OS)
# ======================================================
CFLAGS = -O3 -Wall -Wextra

# ==============================================
# 3. MANAJEMEN BERKAS SUMBER (SOURCES & OBJECTS)
# ==============================================
SRCS = src/main.c src/heap.c src/utils.c
OBJS = $(SRCS:.c=.o)

# =======================================
# 4. ATURAN KOMPILASI (COMPILATION RULES)
# =======================================

# Target default saat mengetik perintah 'make'
all: $(TARGET)

# Aturan untuk membuat file eksekutabel utama
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Aturan pembersihan yang perintahnya ditentukan otomatis oleh hasil deteksi OS di atas
clean:
	$(CLEAN_CMD)

.PHONY: all clean