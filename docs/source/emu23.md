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