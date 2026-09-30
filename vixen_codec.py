#!/usr/bin/env python3

import sys
import re

OPCODES = {
    "nop": 0x000000,
    "halt": 0x7FFFFF,
    "push": 0x000001,
    "pop": 0x000002,
    "peek": 0x000003,
    "dup": 0x000004,
    "swap": 0x000005,
    "popy": 0x000006,
    "peeky": 0x000007,
    "dupy": 0x000008,
    "poke": 0x000009,
    "drop": 0x00000A,
    "dropy": 0x00000B,
    "cycle": 0x00000C,
    "clear": 0x00000D,
    "dump": 0x00000E,
    "count": 0x00000F,
    "cap": 0x000010,
    "acc": 0x000011,
    "base": 0x000012,
    "counter": 0x000013,
    "extra": 0x000014,
    "source": 0x000015,
    "target": 0x000016,
    "load": 0x000017,
    "store": 0x000018,
    "add": 0x000019,
    "sub": 0x00001A,
    "rsub": 0x00001B,
    "neg": 0x00001C,
    "mul": 0x00001D,
    "div": 0x00001E,
    "rdiv": 0x00001F,
    "inv": 0x000020,
    "and": 0x000021,
    "or": 0x000022,
    "xor": 0x000023,
    "not": 0x000024,
    "shl": 0x000025,
    "shr": 0x000026,
    "rol": 0x000027,
    "ror": 0x000028,
    "xchg": 0x000029,
    "cmp": 0x00002A,
    "rcmp": 0x00002B,
    "sign": 0x00002C,
    "pres": 0x00002D,
    "rest": 0x00002E,
    "mark": 0x00002F,
    "reset": 0x000030,
    "setx": 0x000031,
    "clearx": 0x000032,
    "togglex": 0x000033,
    "sety": 0x000034,
    "cleary": 0x000035,
    "toggley": 0x000036,
    "inc": 0x000037,
    "dec": 0x000038,
    "loop": 0x000039,
    "jmp": 0x00003A,
    "jz": 0x00003B,
    "jnz": 0x00003C,
    "js": 0x00003D,
    "jns": 0x00003E,
    "jx": 0x00003F,
    "jnx": 0x000040,
    "jy": 0x000041,
    "jny": 0x000042,
    "jcz": 0x000043,
    "jcnz": 0x000044,
    "jaz": 0x000045,
    "janz": 0x000046,
    "jse": 0x000047,
    "jc": 0x000048,
    "jnc": 0x000049,
    "jemoderr": 0x00004A,
    "call": 0x00004B,
    "ret": 0x00004C,
}

REGS = {
    "a": 1,
    "b": 2,
    "c": 3,
    "d": 4,
    "e": 5,
    "s": 6,
    "t": 7,
}

REV_REGS = {v: k.upper() for k, v in REGS.items()}

SIZE_CODES = {8: 0, 16: 1, 32: 2, 64: 3}
SIZE_NAMES = {
    "byte": 8,
    "word": 16,
    "dword": 32,
    "qword": 64,
    "b": 8,
    "w": 16,
    "dw": 32,
    "qw": 64,
}


def parse_operand(token):
    token = token.strip()
    if not token:
        return None, None, None, None
    if "," in token:
        raise ValueError("Invalid operand syntax: Vixen uses a single operand only, e.g. '@0x1000' or '%8'")

    # Explicit width keyword before the value, e.g. word 42 or byte @C
    if re.match(r"^(byte|word|dword|qword|b|w|dw|qw)\s+", token, re.I):
        m = re.match(r"^(byte|word|dword|qword|b|w|dw|qw)\s+(.*)$", token, re.I)
        width = SIZE_NAMES[m.group(1).lower()]
        inner = m.group(2).strip()
        kind, value, is_memref_flag, _ = parse_operand(inner)
        return kind, value, is_memref_flag, width

    if token.startswith("@"):
        inner = token[1:]
        addr_mode = 1  # absolute address
        if inner.upper() in REGS:
            return "reg", REGS[inner.lower()], 1, addr_mode
        try:
            value = int(inner, 0)
            return "imm", value, 1, addr_mode
        except ValueError:
            raise ValueError(f"Unsupported absolute memory operand: {token!r}")

    if token.startswith("%"):
        inner = token[1:]
        addr_mode = 0  # base-relative offset
        if inner.upper() in REGS:
            return "reg", REGS[inner.lower()], 1, addr_mode
        try:
            value = int(inner, 0)
            return "imm", value, 1, addr_mode
        except ValueError:
            raise ValueError(f"Unsupported relative memory operand: {token!r}")

    if token.upper() in REGS:
        return "reg", REGS[token.lower()], 0, 0

    try:
        value = int(token, 0)
        return "imm", value, 0, 0
    except ValueError:
        raise ValueError(f"Unsupported operand: {token!r}")


def encode_word(op, operand=None, size_bits=None, is_memref=False, addr_mode=0):
    if op not in OPCODES:
        raise ValueError(f"Unknown opcode: {op}")

    regsel = 0
    memsize = 0
    memref = 1 if is_memref else 0
    addr_mode_code = addr_mode & 1

    if operand is None:
        size_bits = 0
    else:
        kind, value, is_memref_flag, _ = parse_operand(operand)
        if kind == "reg":
            regsel = value
            size_bits = 64 if size_bits is None else size_bits
        else:
            size_bits = 64 if size_bits is None else size_bits
            regsel = 0
            if is_memref:
                memref = 1

    if size_bits != 0 and size_bits not in SIZE_CODES:
        raise ValueError(f"Unsupported operand size: {size_bits}")

    size_code = SIZE_CODES.get(size_bits, 0)
    word = (regsel << 29) | (size_code << 27) | (memsize << 25) | (memref << 24) | (addr_mode_code << 23) | OPCODES[op]
    return word


def encode_immediate(value, bits):
    if bits == 8:
        return [value & 0xFF]
    if bits == 16:
        return [value & 0xFFFF]
    if bits == 32:
        return [value & 0xFFFFFFFF]
    if bits == 64:
        lo = value & 0xFFFFFFFF
        hi = (value >> 32) & 0xFFFFFFFF
        return [lo, hi]
    raise ValueError(f"Unsupported immediate width: {bits}")


def encode_instruction(text):
    text = text.strip()
    if not text:
        raise ValueError("Empty instruction")

    m = re.match(r"^([A-Za-z][A-Za-z0-9_]*)\s*(.*)$", text)
    if not m:
        raise ValueError(f"Bad instruction syntax: {text!r}")

    op = m.group(1).lower()
    rest = m.group(2).strip()

    if op not in OPCODES:
        raise ValueError(f"Unknown opcode: {op}")

    if not rest:
        word = encode_word(op, operand=None)
        return [word]

    operand = rest
    if "," in operand:
        raise ValueError("Invalid operand syntax: Vixen uses a single operand only, e.g. '@0x1000' or '%8'")

    size_bits = 64
    memref = False
    addr_mode = 0

    if re.match(r"^(byte|word|dword|qword|b|w|dw|qw)\s+", operand, re.I):
        m2 = re.match(r"^(byte|word|dword|qword|b|w|dw|qw)\s+(.*)$", operand, re.I)
        size_bits = SIZE_NAMES[m2.group(1).lower()]
        operand = m2.group(2).strip()

    if operand.startswith("@"):
        memref = True
        addr_mode = 1
        operand = operand[1:]
    elif operand.startswith("%"):
        memref = True
        addr_mode = 0
        operand = operand[1:]

    if operand.upper() in REGS:
        regsel = REGS[operand.lower()]
        word = (regsel << 29) | (SIZE_CODES[size_bits] << 27) | (0 << 25) | ((1 if memref else 0) << 24) | (addr_mode << 23) | OPCODES[op]
        return [word]

    value = int(operand, 0)
    imm_words = encode_immediate(value, size_bits)
    word = (0 << 29) | (SIZE_CODES[size_bits] << 27) | (0 << 25) | ((1 if memref else 0) << 24) | (addr_mode << 23) | OPCODES[op]
    return [word] + imm_words


def decode_reg(code):
    if code == 0:
        return None
    return REV_REGS.get(code, f"r{code}")


def decode_immediate(words, bits):
    if bits == 8:
        return words[0] & 0xFF
    if bits == 16:
        return words[0] & 0xFFFF
    if bits == 32:
        return words[0] & 0xFFFFFFFF
    if bits == 64:
        low = words[0] & 0xFFFFFFFF
        high = words[1] & 0xFFFFFFFF if len(words) > 1 else 0
        return (high << 32) | low
    raise ValueError(f"Unsupported size in decode: {bits}")


def decode_instruction(words):
    if not words:
        raise ValueError("No instruction words provided")

    word = words[0]
    op_bits = word & 0x7FFFFF
    opcode_name = None
    for name, code in OPCODES.items():
        if code == op_bits:
            opcode_name = name
            break

    if opcode_name is None:
        raise ValueError(f"Unknown opcode bits: 0x{op_bits:06X}")

    regsel = (word >> 29) & 0b111
    size_code = (word >> 27) & 0b11
    memsize = (word >> 25) & 0b11
    ref = (word >> 24) & 1
    addr_mode = (word >> 23) & 1

    size_bits = {0: 8, 1: 16, 2: 32, 3: 64}[size_code]

    if regsel != 0:
        operand = decode_reg(regsel)
        op_text = f"{opcode_name} {operand}"
        return op_text

    operand = None
    remaining = words[1:]
    if ref:
        # memory reference: if a register follows in the operand selector, it is represented by regsel, but here regsel=0 means immediate form.
        if remaining:
            value = decode_immediate(remaining, size_bits)
            prefix = "@" if addr_mode == 1 else "%"
            operand = f"{prefix}{value}"
    else:
        if remaining:
            value = decode_immediate(remaining, size_bits)
            operand = str(value)

    if operand is None:
        return opcode_name
    return f"{opcode_name} {operand}"


def usage():
    print("Usage:")
    print("  python3 vixen_codec.py encode \"acc 42\"")
    print("  python3 vixen_codec.py decode 0x00000000 0x0000002A")
    print("  python3 vixen_codec.py decode 0x00000000")


def main(argv):
    if len(argv) < 2:
        usage()
        return 1

    mode = argv[1].lower()

    try:
        if mode == "encode":
            if len(argv) < 3:
                raise ValueError("Missing instruction text")
            text = " ".join(argv[2:])
            result = encode_instruction(text)
            print(" ".join(f"0x{v:08X}" for v in result))
            return 0

        if mode == "decode":
            if len(argv) < 3:
                raise ValueError("Missing instruction words")
            words = []
            for token in argv[2:]:
                token = token.strip()
                if token.startswith("0x") or token.startswith("0X"):
                    words.append(int(token, 16))
                else:
                    words.append(int(token, 10))
            result = decode_instruction(words)
            print(result)
            return 0

        usage()
        return 1

    except Exception as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
