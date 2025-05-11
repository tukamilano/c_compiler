emu23
========

``` c
enum Register { EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, REGISTERS_COUNT };
char* registers_name[] = {
    "EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI"};
```

`EAX`から`EDI`はx86における汎用レジスタです. 合計で8つのレジスタがあり, `REGISTERS_COUNT`はレジスタの数を表します.

``` c
typedef struct {
    /* 汎用レジスタ */
    uint32_t  registers[REGISTERS_COUNT];

    /* EFLAGSレジスタ */
    uint32_t eflags;

    /* メモリ(バイト列) */
    uint8_t* memory;

    /* プログラムカウンタ */
    uint32_t eip;
} Emulator;
```

x86の汎用レジスタは32ビットの整数を格納することができ, `uint32_t`型で表現されます. EFLAGSレジスタはCPUの現在の状態を示す複数のフラグビットを格納します.

``` c
/* エミュレータを作成する */
static Emulator* create_emu(size_t size, uint32_t eip, uint32_t esp)
{
    Emulator* emu = malloc(sizeof(Emulator));
    emu->memory = malloc(size);

    /* 汎用レジスタの初期値を全て0にする */
    memset(emu->registers, 0, sizeof(emu->registers));

    /* レジスタの初期値を指定されたものにする */
    emu->eip = eip;
    emu->registers[ESP] = esp;

    return emu;
}
```
Emulatorをプログラム開始時から終了まで確保される固定的な領域(静的領域)に確保します. 

``` c
/* 汎用レジスタとプログラムカウンタの値を標準出力に出力する */
static void dump_registers(Emulator* emu)
{
    int i;

    for (i = 0; i < REGISTERS_COUNT; i++) {
        printf("%s = %08x\n", registers_name[i], emu->registers[i]);
    }

    printf("EIP = %08x\n", emu-> eip);
}
```
`%08x`は16進数(4ビット分)で8桁の整数を出力する書式指定子です.

``` c
uint32_t get_code32(Emulator* emu, int index)
{
    int i;
    uint32_t ret = 0;

    /* リトルエンディアンでメモリの値を取得する */
    for (i = 0; i < 4; i++) {
        ret |= get_code8(emu, index + i) << (i * 8);
    }

    return ret;
}
```

メモリ上ではリトルエンディアンで表現されるため, メモリ上に `0x78 0x56 0x34 0x12` というバイト列があった場合、このコードを実行すると`ret`には `0x12345678` という値が格納されます.

``` c
void mov_r32_imm32(Emulator* emu)
{
    uint8_t reg = get_code8(emu, 0) - 0xB8;
    uint32_t value = get_code32(emu, 1);
    emu->registers[reg] = value;
    emu->eip += 5;
}
```
`mov`命令はレジスタに即値を格納する命令です. `0xB8`から始まる命令はレジスタに即値を格納する命令で, `0xB8`から`0xBF`(B8+レジスタの値)までの8種類のレジスタに即値を格納し, `get_code8(emu, 0) - 0xB8`でレジスタの番号を取得します. `get_code32(emu, 1)`では, `0xB8`の次のバイトから4バイト分を取得し, それをレジスタに格納します. `emu->eip += 5`は命令ポインタを5バイト進めることを意味します. `mov`命令は1バイトのオペコードと4バイトの即値で構成されているため, 合計で5バイト進める必要があります.

``` c
void short_jump(Emulator* emu)
{
    int8_t diff = get_sign_code8(emu, 1);
    emu->eip += (diff + 2);
}
```
`short jump`命令は, `jmp`命令の一種で, 1バイトのオペコードと1バイトの即値で構成されており, 即値は相対アドレスを表します. `get_sign_code8(emu, 1)`で即値を取得し, `emu->eip += (diff + 2)`で命令ポインタを進めます. `+2`は命令ポインタを進めるために必要なバイト数です.

``` c
typedef void instruction_func_t(Emulator*);
instruction_func_t* instructions[256];
void init_instructions(void)
{
    int i;
    memset(instructions, 0, sizeof(instructions));
    for (i = 0; i < 8; i++) {
        instructions[0xB8 + i] = mov_r32_imm32;
    }
    instructions[0xEB] = short_jump;
}
```
`typedef void instruction_func_t(Emulator*)`でエミュレータに命令を実行させる関数ポインタの型を定義します. そして`instruction_func_t* instructions[256]`でその関数ポインタの配列を作成します.  
instructionsは256個の命令を格納する配列です. x86の命令オペコードは8ビットで表現されるため, 2^8=256となります. そのためinstruction[256]という配列は全てのオペコードに関する命令を格納することができます. `mov`命令は`0xB8`から始まるため, `0xB8`から`0xBF`までの8個の命令をmov_r32_imm32に設定します.
