#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))

struct opcode_entry {
    const char *name;
    uint32_t code;
};

static const struct opcode_entry OPCODES[] = {
    {"nop", 0x000000},
    {"halt", 0x7FFFFF},
    {"push", 0x000001},
    {"pop", 0x000002},
    {"peek", 0x000003},
    {"dup", 0x000004},
    {"swap", 0x000005},
    {"popy", 0x000006},
    {"peeky", 0x000007},
    {"dupy", 0x000008},
    {"poke", 0x000009},
    {"drop", 0x00000A},
    {"dropy", 0x00000B},
    {"cycle", 0x00000C},
    {"clear", 0x00000D},
    {"dump", 0x00000E},
    {"count", 0x00000F},
    {"cap", 0x000010},
    {"acc", 0x000011},
    {"base", 0x000012},
    {"counter", 0x000013},
    {"extra", 0x000014},
    {"source", 0x000015},
    {"target", 0x000016},
    {"load", 0x000017},
    {"store", 0x000018},
    {"add", 0x000019},
    {"sub", 0x00001A},
    {"rsub", 0x00001B},
    {"neg", 0x00001C},
    {"mul", 0x00001D},
    {"div", 0x00001E},
    {"rdiv", 0x00001F},
    {"inv", 0x000020},
    {"and", 0x000021},
    {"or", 0x000022},
    {"xor", 0x000023},
    {"not", 0x000024},
    {"shl", 0x000025},
    {"shr", 0x000026},
    {"rol", 0x000027},
    {"ror", 0x000028},
    {"xchg", 0x000029},
    {"cmp", 0x00002A},
    {"rcmp", 0x00002B},
    {"sign", 0x00002C},
    {"pres", 0x00002D},
    {"rest", 0x00002E},
    {"mark", 0x00002F},
    {"reset", 0x000030},
    {"setx", 0x000031},
    {"clearx", 0x000032},
    {"togglex", 0x000033},
    {"sety", 0x000034},
    {"cleary", 0x000035},
    {"toggley", 0x000036},
    {"inc", 0x000037},
    {"dec", 0x000038},
    {"loop", 0x000039},
    {"jmp", 0x00003A},
    {"jz", 0x00003B},
    {"jnz", 0x00003C},
    {"js", 0x00003D},
    {"jns", 0x00003E},
    {"jx", 0x00003F},
    {"jnx", 0x000040},
    {"jy", 0x000041},
    {"jny", 0x000042},
    {"jcz", 0x000043},
    {"jcnz", 0x000044},
    {"jaz", 0x000045},
    {"janz", 0x000046},
    {"jse", 0x000047},
    {"jc", 0x000048},
    {"jnc", 0x000049},
    {"jemoderr", 0x00004A},
    {"call", 0x00004B},
    {"ret", 0x00004C},
    {NULL, 0}
};

struct reg_entry {
    const char *name;
    int code;
};

static const struct reg_entry REGS[] = {
    {"a", 1},
    {"b", 2},
    {"c", 3},
    {"d", 4},
    {"e", 5},
    {"s", 6},
    {"t", 7},
    {NULL, 0}
};

static const int SIZE_CODES[] = {0, 1, 2, 3};

static void trim_in_place(char *s) {
    char *start = s;
    char *end;
    while (*start && isspace((unsigned char)*start)) start++;
    if (start != s) memmove(s, start, strlen(start) + 1);
    end = s + strlen(s);
    while (end > s && isspace((unsigned char)end[-1])) {
        end--;
        *end = '\0';
    }
}

static int string_eq_ci(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++; b++;
    }
    return *a == '\0' && *b == '\0';
}

static int lookup_opcode(const char *name, uint32_t *out) {
    for (int i = 0; OPCODES[i].name != NULL; i++) {
        if (string_eq_ci(OPCODES[i].name, name)) {
            *out = OPCODES[i].code;
            return 1;
        }
    }
    return 0;
}

static int lookup_reg(const char *name, int *out) {
    for (int i = 0; REGS[i].name != NULL; i++) {
        if (string_eq_ci(REGS[i].name, name)) {
            *out = REGS[i].code;
            return 1;
        }
    }
    return 0;
}

static int get_size_bits(const char *token, int *size_bits) {
    if (string_eq_ci(token, "byte") || string_eq_ci(token, "b")) {
        *size_bits = 8; return 1;
    }
    if (string_eq_ci(token, "word") || string_eq_ci(token, "w")) {
        *size_bits = 16; return 1;
    }
    if (string_eq_ci(token, "dword") || string_eq_ci(token, "dw")) {
        *size_bits = 32; return 1;
    }
    if (string_eq_ci(token, "qword") || string_eq_ci(token, "qw")) {
        *size_bits = 64; return 1;
    }
    return 0;
}

static void parse_operand(const char *token, int *kind, long long *value, int *memref, int *addr_mode, int *size_bits) {
    char tmp[256];
    char *p;
    char *inner;
    int local_size = 64;

    *kind = 0;
    *value = 0;
    *memref = 0;
    *addr_mode = 0;
    *size_bits = 64;

    if (token == NULL || *token == '\0') {
        return;
    }

    snprintf(tmp, sizeof(tmp), "%s", token);
    trim_in_place(tmp);
    if (strchr(tmp, ',') != NULL) {
        fprintf(stderr, "Invalid operand syntax: Vixen uses a single operand only, e.g. @0x1000 or %%8\n");
        exit(2);
    }

    p = tmp;
    while (*p && isspace((unsigned char)*p)) p++;
    if (strncmp(p, "byte", 4) == 0 || strncmp(p, "word", 4) == 0 || strncmp(p, "dword", 5) == 0 || strncmp(p, "qword", 5) == 0 ||
        strncmp(p, "b", 1) == 0 || strncmp(p, "w", 1) == 0 || strncmp(p, "dw", 2) == 0 || strncmp(p, "qw", 2) == 0) {
        char word[32];
        char *q = p;
        while (*q && !isspace((unsigned char)*q)) q++;
        if (*q == '\0') {
            fprintf(stderr, "Invalid size keyword syntax\n");
            exit(2);
        }
        snprintf(word, sizeof(word), "%.*s", (int)(q - p), p);
        get_size_bits(word, &local_size);
        while (*q && isspace((unsigned char)*q)) q++;
        inner = q;
        *size_bits = local_size;
    } else {
        inner = p;
    }

    if (inner == NULL || *inner == '\0') {
        return;
    }

    if (*inner == '@') {
        *memref = 1;
        *addr_mode = 1;
        inner++;
    } else if (*inner == '%') {
        *memref = 1;
        *addr_mode = 0;
        inner++;
    }

    char regname[32];
    if (sscanf(inner, "%31[A-Za-z]", regname) == 1 && strlen(regname) > 0) {
        int reg = 0;
        if (lookup_reg(regname, &reg)) {
            *kind = 1;   // reg
            *value = reg;
            return;
        }
    }

    errno = 0;
    char *end = NULL;
    long long v = strtoll(inner, &end, 0);
    if (errno != 0 || end == inner || *end != '\0') {
        fprintf(stderr, "Unsupported operand: %s\n", token);
        exit(2);
    }

    *kind = 2;  // imm
    *value = v;
}

static void encode_immediate(uint64_t value, int bits, uint32_t *words, int *word_count) {
    if (bits == 8) {
        words[0] = (uint32_t)(value & 0xFFu);
        *word_count = 1;
        return;
    }
    if (bits == 16) {
        words[0] = (uint32_t)(value & 0xFFFFu);
        *word_count = 1;
        return;
    }
    if (bits == 32) {
        words[0] = (uint32_t)(value & 0xFFFFFFFFu);
        *word_count = 1;
        return;
    }
    if (bits == 64) {
        words[0] = (uint32_t)(value & 0xFFFFFFFFu);
        words[1] = (uint32_t)((value >> 32) & 0xFFFFFFFFu);
        *word_count = 2;
        return;
    }
    fprintf(stderr, "Unsupported immediate width: %d\n", bits);
    exit(2);
}

static int encode_instruction(const char *text, uint32_t *out_words, int *out_count) {
    char buf[256];
    char *op_text;
    char *operand = NULL;
    uint32_t opcode = 0;
    int size_bits = 64;
    int kind = 0;
    long long value = 0;
    int memref = 0;
    int addr_mode = 0;
    uint32_t word = 0;
    uint32_t imm_words[2] = {0, 0};
    int imm_count = 0;
    int regsel = 0;

    if (text == NULL || *text == '\0') {
        fprintf(stderr, "Empty instruction\n");
        return 0;
    }

    snprintf(buf, sizeof(buf), "%s", text);
    trim_in_place(buf);

    op_text = strtok(buf, " \t");
    if (op_text == NULL) {
        fprintf(stderr, "Bad instruction syntax\n");
        return 0;
    }
    operand = strtok(NULL, "");
    if (operand != NULL) {
        trim_in_place(operand);
    }

    if (!lookup_opcode(op_text, &opcode)) {
        fprintf(stderr, "Unknown opcode: %s\n", op_text);
        return 0;
    }

    if (operand == NULL || *operand == '\0') {
        out_words[0] = opcode;
        *out_count = 1;
        return 1;
    }

    parse_operand(operand, &kind, &value, &memref, &addr_mode, &size_bits);

    if (kind == 1) {  // reg operand
        regsel = (int)value;
        word = ((uint32_t)regsel << 29) |
               ((uint32_t)SIZE_CODES[(size_bits == 8 ? 0 : size_bits == 16 ? 1 : size_bits == 32 ? 2 : 3)] << 27) |
               (0u << 25) |
               ((memref ? 1u : 0u) << 24) |
               ((addr_mode & 1u) << 23) |
               opcode;
        out_words[0] = word;
        *out_count = 1;
        return 1;
    }

    encode_immediate((uint64_t)value, size_bits, imm_words, &imm_count);
    word = (0u << 29) |
           ((uint32_t)SIZE_CODES[(size_bits == 8 ? 0 : size_bits == 16 ? 1 : size_bits == 32 ? 2 : 3)] << 27) |
           (0u << 25) |
           ((memref ? 1u : 0u) << 24) |
           ((addr_mode & 1u) << 23) |
           opcode;

    out_words[0] = word;
    for (int i = 0; i < imm_count; i++) out_words[i + 1] = imm_words[i];
    *out_count = 1 + imm_count;
    return 1;
}

static void usage(const char *prog) {
    printf("Usage:\n");
    printf("  %s encode \"add %%8\"\n", prog);
    printf("  %s encode \"add @0x1000\"\n", prog);
    printf("  %s decode 0x19000019 0x00000008\n", prog);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "encode") == 0) {
        if (argc < 3) {
            usage(argv[0]);
            return 1;
        }
        uint32_t words[4] = {0, 0, 0, 0};
        int count = 0;
        if (!encode_instruction(argv[2], words, &count)) {
            return 2;
        }
        for (int i = 0; i < count; i++) {
            printf("0x%08X%c", words[i], i + 1 == count ? '\n' : ' ');
        }
        return 0;
    }

    if (strcmp(argv[1], "decode") == 0) {
        fprintf(stderr, "decode not implemented in this C version yet\n");
        return 1;
    }

    usage(argv[0]);
    return 1;
}
