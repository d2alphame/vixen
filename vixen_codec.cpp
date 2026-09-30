#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <cctype>
#include <cstdlib>
#include <map>
#include <sstream>
#include <iomanip>

using namespace std;

static const map<string, uint32_t> OPCODES = {
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
    {"ret", 0x00004C}
};

static const map<string, int> REGS = {
    {"a", 1},
    {"b", 2},
    {"c", 3},
    {"d", 4},
    {"e", 5},
    {"s", 6},
    {"t", 7}
};

static const map<int, int> SIZE_CODES = {{8, 0}, {16, 1}, {32, 2}, {64, 3}};
static const map<string, int> SIZE_NAMES = {{"byte", 8}, {"word", 16}, {"dword", 32}, {"qword", 64}, {"b", 8}, {"w", 16}, {"dw", 32}, {"qw", 64}};

static string trim(const string &s) {
    size_t a = 0;
    while (a < s.size() && isspace((unsigned char)s[a])) a++;
    size_t b = s.size();
    while (b > a && isspace((unsigned char)s[b - 1])) b--;
    return s.substr(a, b - a);
}

static string to_lower(string s) {
    for (char &c : s) c = static_cast<char>(tolower((unsigned char)c));
    return s;
}

static void parse_operand(const string &token, int &kind, long long &value, bool &memref, int &addr_mode, int &size_bits) {
    string t = trim(token);
    if (t.empty()) {
        kind = 0; value = 0; memref = false; addr_mode = 0; size_bits = 64; return;
    }
    if (t.find(',') != string::npos) {
        throw runtime_error("Invalid operand syntax: Vixen uses a single operand only, e.g. @0x1000 or %8");
    }

    string inner = t;
    size_bits = 64;
    size_t pos = 0;
    while (pos < inner.size() && !isspace((unsigned char)inner[pos])) pos++;
    string head = inner.substr(0, pos);
    if (SIZE_NAMES.count(to_lower(head)) != 0) {
        size_bits = SIZE_NAMES.at(to_lower(head));
        string rest = trim(inner.substr(pos));
        if (rest.empty()) throw runtime_error("Missing operand after size keyword");
        inner = rest;
    }

    if (!inner.empty() && inner[0] == '@') {
        memref = true; addr_mode = 1; inner.erase(0, 1);
    } else if (!inner.empty() && inner[0] == '%') {
        memref = true; addr_mode = 0; inner.erase(0, 1);
    }

    if (inner.empty()) {
        kind = 0; value = 0; return;
    }

    if (REGS.count(to_lower(inner)) != 0) {
        kind = 1; value = REGS.at(to_lower(inner)); return;
    }

    char *end = nullptr;
    long long v = strtoll(inner.c_str(), &end, 0);
    if (end == inner.c_str() || *end != '\0') {
        throw runtime_error("Unsupported operand: " + token);
    }
    kind = 2; value = v;
}

static vector<uint32_t> encode_immediate(long long value, int bits) {
    vector<uint32_t> out;
    if (bits == 8) out.push_back((uint32_t)(value & 0xFFu));
    else if (bits == 16) out.push_back((uint32_t)(value & 0xFFFFu));
    else if (bits == 32) out.push_back((uint32_t)(value & 0xFFFFFFFFu));
    else if (bits == 64) {
        out.push_back((uint32_t)(value & 0xFFFFFFFFu));
        out.push_back((uint32_t)((value >> 32) & 0xFFFFFFFFu));
    } else {
        throw runtime_error("Unsupported immediate width");
    }
    return out;
}

static vector<uint32_t> encode_instruction(const string &text) {
    string t = trim(text);
    if (t.empty()) throw runtime_error("Empty instruction");

    size_t sp = t.find_first_of(" \t");
    string op = trim(t.substr(0, sp));
    string operand = sp == string::npos ? "" : trim(t.substr(sp + 1));

    auto it = OPCODES.find(to_lower(op));
    if (it == OPCODES.end()) throw runtime_error("Unknown opcode: " + op);

    uint32_t opcode = it->second;
    if (operand.empty()) {
        return {opcode};
    }
    if (operand.find(',') != string::npos) {
        throw runtime_error("Invalid operand syntax: Vixen uses a single operand only, e.g. @0x1000 or %8");
    }

    int kind = 0; long long value = 0; bool memref = false; int addr_mode = 0; int size_bits = 64;
    parse_operand(operand, kind, value, memref, addr_mode, size_bits);

    uint32_t size_code = SIZE_CODES.at(size_bits);
    if (kind == 1) {
        uint32_t regsel = static_cast<uint32_t>(value);
        return {((regsel & 0b111u) << 29) |
                ((size_code & 0b11u) << 27) |
                ((memref ? 1u : 0u) << 24) |
                ((addr_mode & 1u) << 23) |
                opcode};
    }

    vector<uint32_t> out;
    uint32_t word = (0u << 29) |
                    ((size_code & 0b11u) << 27) |
                    ((memref ? 1u : 0u) << 24) |
                    ((addr_mode & 1u) << 23) |
                    opcode;
    out.push_back(word);
    auto imm = encode_immediate(value, size_bits);
    out.insert(out.end(), imm.begin(), imm.end());
    return out;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        cout << "Usage:\n";
        cout << "  vixen_codec_cpp encode \"add %8\"\n";
        cout << "  vixen_codec_cpp encode \"add @0x1000\"\n";
        return 1;
    }

    string mode = argv[1];
    if (mode == "encode") {
        if (argc < 3) {
            cout << "Missing instruction text\n";
            return 1;
        }
        try {
            auto words = encode_instruction(argv[2]);
            for (size_t i = 0; i < words.size(); ++i) {
                cout << "0x" << hex << uppercase << setw(8) << setfill('0') << words[i] << (i + 1 == words.size() ? "\n" : " ");
            }
            return 0;
        } catch (const exception &e) {
            cerr << "Error: " << e.what() << endl;
            return 2;
        }
    }

    cout << "Usage:\n";
    cout << "  vixen_codec_cpp encode \"add %8\"\n";
    cout << "  vixen_codec_cpp encode \"add @0x1000\"\n";
    return 1;
}
