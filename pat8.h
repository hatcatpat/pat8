#define width 128
#define height 96
#define screen_size (width * height)
#define mem_size 65536
#define stack_size 256
#define rstack_size 256
#define key_size 16

typedef unsigned char byte;
typedef unsigned short byte2;

struct pat8
{
    byte memory[mem_size];
    byte video[screen_size];
    byte rstack[rstack_size];
    byte stack[stack_size];
    byte key[key_size];
    byte2 p;
    byte s;
    byte r;
    byte draw;
    byte error;
};
extern struct pat8 p8;

static int num_prog_ops = 5;
enum
{
    // prog ops
    op_enum_null,
    op_enum_return,
    op_enum_push,
    op_enum_pop,
    op_enum_rand,
    // normal ops
    op_enum_jump,
    op_enum_jump_when,
    op_enum_jump_if,
    op_enum_add,
    op_enum_sub,
    op_enum_mul,
    op_enum_pow,
    op_enum_lshift,
    op_enum_eq,
    op_enum_neq,
    op_enum_le,
    op_enum_nle,
    op_enum_leq,
    op_enum_nleq,
    op_enum_neg,
    op_enum_set_pix,
    op_enum_get_key,
    num_ops
};

byte p8_init();
void p8_eval();
