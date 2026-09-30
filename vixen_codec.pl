#!/usr/bin/env perl
use strict;
use warnings;

my %OPCODES = (
    nop => 0x000000,
    halt => 0x7FFFFF,
    push => 0x000001,
    pop => 0x000002,
    peek => 0x000003,
    dup => 0x000004,
    swap => 0x000005,
    popy => 0x000006,
    peeky => 0x000007,
    dupy => 0x000008,
    poke => 0x000009,
    drop => 0x00000A,
    dropy => 0x00000B,
    cycle => 0x00000C,
    clear => 0x00000D,
    dump => 0x00000E,
    count => 0x00000F,
    cap => 0x000010,
    acc => 0x000011,
    base => 0x000012,
    counter => 0x000013,
    extra => 0x000014,
    source => 0x000015,
    target => 0x000016,
    load => 0x000017,
    store => 0x000018,
    add => 0x000019,
    sub => 0x00001A,
    rsub => 0x00001B,
    neg => 0x00001C,
    mul => 0x00001D,
    div => 0x00001E,
    rdiv => 0x00001F,
    inv => 0x000020,
    and => 0x000021,
    or => 0x000022,
    xor => 0x000023,
    not => 0x000024,
    shl => 0x000025,
    shr => 0x000026,
    rol => 0x000027,
    ror => 0x000028,
    xchg => 0x000029,
    cmp => 0x00002A,
    rcmp => 0x00002B,
    sign => 0x00002C,
    pres => 0x00002D,
    rest => 0x00002E,
    mark => 0x00002F,
    reset => 0x000030,
    setx => 0x000031,
    clearx => 0x000032,
    togglex => 0x000033,
    sety => 0x000034,
    cleary => 0x000035,
    toggley => 0x000036,
    inc => 0x000037,
    dec => 0x000038,
    loop => 0x000039,
    jmp => 0x00003A,
    jz => 0x00003B,
    jnz => 0x00003C,
    js => 0x00003D,
    jns => 0x00003E,
    jx => 0x00003F,
    jnx => 0x000040,
    jy => 0x000041,
    jny => 0x000042,
    jcz => 0x000043,
    jcnz => 0x000044,
    jaz => 0x000045,
    janz => 0x000046,
    jse => 0x000047,
    jc => 0x000048,
    jnc => 0x000049,
    jemoderr => 0x00004A,
    call => 0x00004B,
    ret => 0x00004C,
);

my %REGS = (
    a => 1,
    b => 2,
    c => 3,
    d => 4,
    e => 5,
    s => 6,
    t => 7,
);

my %REV_REGS = reverse %REGS;
my %SIZE_CODES = (8 => 0, 16 => 1, 32 => 2, 64 => 3);
my %SIZE_NAMES = (
    byte => 8,
    word => 16,
    dword => 32,
    qword => 64,
    b => 8,
    w => 16,
    dw => 32,
    qw => 64,
);

sub usage {
    print "Usage:\n";
    print "  perl vixen_codec.pl encode \"acc 42\"\n";
    print "  perl vixen_codec.pl decode 0x00000000 0x0000002A\n";
    print "  perl vixen_codec.pl decode 0x00000000\n";
}

sub parse_operand {
    my ($token) = @_;
    $token = '' unless defined $token;
    $token =~ s/^\s+|\s+$//g;
    return (undef, undef, undef) if $token eq '';
    die "Invalid operand syntax: Vixen uses a single operand only, e.g. \@0x1000 or %8\n" if $token =~ /,/;

    if ($token =~ /^(byte|word|dword|qword|b|w|dw|qw)\s+(.*)$/i) {
        my $name = lc $1;
        my $inner = $2;
        my $width = $SIZE_NAMES{$name};
        my ($kind, $value, $mem_flag) = parse_operand($inner);
        return ($kind, $value, $mem_flag, $width);
    }

    if ($token =~ /^@(.*)$/) {
        my $inner = $1;
        if (exists $REGS{lc $inner}) {
            return ('reg', $REGS{lc $inner}, 1, 0);
        }
        my $value = eval "0 + $inner";
        return ('imm', $value, 1, 0);
    }

    if (exists $REGS{lc $token}) {
        return ('reg', $REGS{lc $token}, 0, 0);
    }

    my $value = eval "0 + $token";
    return ('imm', $value, 0, 0);
}

sub encode_immediate {
    my ($value, $bits) = @_;
    if ($bits == 8) { return [$value & 0xFF]; }
    if ($bits == 16) { return [$value & 0xFFFF]; }
    if ($bits == 32) { return [$value & 0xFFFFFFFF]; }
    if ($bits == 64) {
        my $lo = $value & 0xFFFFFFFF;
        my $hi = ($value >> 32) & 0xFFFFFFFF;
        return [$lo, $hi];
    }
    die "Unsupported immediate width: $bits\n";
}

sub encode_word {
    my ($op, $operand, $size_bits, $memref, $addr_mode) = @_;
    die "Unknown opcode: $op\n" unless exists $OPCODES{$op};
    $size_bits = 0 unless defined $size_bits;
    $memref = 0 unless defined $memref;
    $addr_mode = 0 unless defined $addr_mode;

    my $regsel = 0;
    my $size_code = exists $SIZE_CODES{$size_bits} ? $SIZE_CODES{$size_bits} : 0;
    my $word = (($regsel & 0b111) << 29) |
               (($size_code & 0b11) << 27) |
               (0 << 25) |
               (($memref ? 1 : 0) << 24) |
               (($addr_mode & 1) << 23) |
               $OPCODES{$op};

    if (defined $operand) {
        my ($kind, $value, $mem_flag, $junk) = parse_operand($operand);
        if ($kind eq 'reg') {
            $regsel = $value;
            $size_bits = 64 unless defined $size_bits;
            $size_code = $SIZE_CODES{$size_bits};
            $word = (($regsel & 0b111) << 29) |
                    (($size_code & 0b11) << 27) |
                    (0 << 25) |
                    (($memref ? 1 : 0) << 24) |
                    (($addr_mode & 1) << 23) |
                    $OPCODES{$op};
        }
    }

    return $word;
}

sub encode_instruction {
    my ($text) = @_;
    $text =~ s/^\s+|\s+$//g;
    die "Empty instruction\n" if $text eq '';

    my ($op, $rest) = $text =~ /^([A-Za-z][A-Za-z0-9_]*)\s*(.*)$/;
    die "Bad instruction syntax: $text\n" unless defined $op;
    $op = lc $op;
    die "Unknown opcode: $op\n" unless exists $OPCODES{$op};

    my $operand = $rest;
    $operand =~ s/^\s+|\s+$//g;
    die "Invalid operand syntax: Vixen uses a single operand only, e.g. \@0x1000 or %8\n" if $operand =~ /,/;

    if ($operand eq '') {
        return [encode_word($op, undef, 0)];
    }

    my $size_bits = 64;
    if ($operand =~ /^(byte|word|dword|qword|b|w|dw|qw)\s+/i) {
        my $name = lc $1;
        $size_bits = $SIZE_NAMES{$name};
        $operand =~ s/^(byte|word|dword|qword|b|w|dw|qw)\s+//i;
    }

    my $memref = 0;
    my $addr_mode = 0;
    if ($operand =~ /^@/) {
        $memref = 1;
        $addr_mode = 1;
        $operand =~ s/^@//;
    } elsif ($operand =~ /^%/) {
        $memref = 1;
        $addr_mode = 0;
        $operand =~ s/^%//;
    }

    my ($kind, $value, $mflag, $unused) = parse_operand($operand);

    if (defined $kind && $kind eq 'reg') {
        my $word = (($value & 0b111) << 29) |
                   (($SIZE_CODES{$size_bits} & 0b11) << 27) |
                   (0 << 25) |
                   (($memref ? 1 : 0) << 24) |
                   (0 << 23) |
                   $OPCODES{$op};
        return [$word];
    }

    my @imm = @{ encode_immediate($value, $size_bits) };
    my $word = (0 << 29) |
               (($SIZE_CODES{$size_bits} & 0b11) << 27) |
               (0 << 25) |
               (($memref ? 1 : 0) << 24) |
               (($addr_mode & 1) << 23) |
               $OPCODES{$op};

    return [$word, @imm];
}

sub decode_reg {
    my ($code) = @_;
    return undef if !defined $code || $code == 0;
    return $REV_REGS{$code} // "r$code";
}

sub decode_immediate {
    my ($words, $bits) = @_;
    if ($bits == 8) {
        return $words->[0] & 0xFF;
    }
    if ($bits == 16) {
        return $words->[0] & 0xFFFF;
    }
    if ($bits == 32) {
        return $words->[0] & 0xFFFFFFFF;
    }
    if ($bits == 64) {
        my $low = $words->[0] & 0xFFFFFFFF;
        my $high = defined $words->[1] ? ($words->[1] & 0xFFFFFFFF) : 0;
        return (($high << 32) | $low);
    }
    die "Unsupported immediate width: $bits\n";
}

sub decode_instruction {
    my (@words) = @_;
    die "No instruction words provided\n" if !@words;

    my $word = $words[0] & 0xFFFFFFFF;
    my $op_bits = $word & 0x7FFFFF;
    my $name = '';
    for my $k (keys %OPCODES) {
        if ($OPCODES{$k} == $op_bits) {
            $name = $k;
            last;
        }
    }
    die "Unknown opcode bits: 0x" . sprintf("%06X", $op_bits) . "\n" if $name eq '';

    my $regsel = ($word >> 29) & 0b111;
    my $size_code = ($word >> 27) & 0b11;
    my $memsize = ($word >> 25) & 0b11;
    my $ref = ($word >> 24) & 1;
    my $addr_mode = ($word >> 23) & 1;
    my $size_bits = {0 => 8, 1 => 16, 2 => 32, 3 => 64}->{$size_code};

    if ($regsel != 0) {
        my $reg = decode_reg($regsel);
        return "$name $reg";
    }

    my @tail = @words[1 .. $#words];
    my $value = undef;
    if (@tail) {
        $value = decode_immediate(\@tail, $size_bits);
    }

    if (!$ref && defined $value) {
        return "$name $value";
    }
    if ($ref && defined $value) {
        my $prefix = $addr_mode == 1 ? '@' : '%';
        return "$name ${prefix}${value}";
    }

    return $name;
}

my $mode = shift @ARGV // '';
if (!defined $mode || $mode eq '' || $mode =~ /^-h|^--help$/) {
    usage();
    exit 0;
}

if ($mode eq 'encode') {
    my $text = join(' ', @ARGV);
    my @words = @{ encode_instruction($text) };
    my @out = map { sprintf("0x%08X", $_) } @words;
    print join(' ', @out), "\n";
    exit 0;
}

if ($mode eq 'decode') {
    my @tokens = @ARGV;
    my @words = map {
        my $t = $_;
        $t =~ s/^0x/0x/i;
        if ($t =~ /^0x/i) { hex($t) } else { int($t) }
    } @tokens;
    print decode_instruction(@words), "\n";
    exit 0;
}

usage();
exit 1;
