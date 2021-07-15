#include "pat8.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

struct pat8 p8;

//utils
static byte2 get_pix_index(byte x, byte y)
{
    return x + y * width;
}

static byte2 merge_bytes(byte a, byte b)
{
    return (a << 8) | b;
}

static byte get_stack(byte i)
{
    return p8.stack[p8.s - i];
}
static byte2 get_stack_merge(byte p)
{
    return merge_bytes(get_stack(2 + p - 1), get_stack(1 + p - 1));
}
static void set_stack(byte i, byte v)
{
    p8.stack[p8.s - i] = v;
}

static byte get_prog(byte i)
{
    return p8.memory[p8.p + i - 1];
}
static byte2 get_prog_merge(byte p)
{
    return merge_bytes(get_prog(1 + p - 1), get_stack(2 + p - 1));
}

static void pop()
{
    p8.s--;
}
static void popn(byte2 n)
{
    p8.s -= n;
}
static void push(byte v)
{
    p8.stack[p8.s] = v;
    p8.s++;
}
static void swap()
{
    byte a = get_stack(1);
    byte b = get_stack(2);
    set_stack(1, b);
    set_stack(2, a);
}
static void swap_pop()
{
    swap();
    pop();
}
static void swap_popn(byte n)
{
    for (byte i = 0; i < n; i++)
        swap_pop();
}

// opcodes =========================================================

//// data ==========================================================
static void op_return()
{
    p8.r--;
    p8.p = p8.rstack[p8.r];
}

static void op_push()
{
    push(get_prog(1));
    p8.p++;
}

static void op_pop()
{
    pop();
}

//// logic ==========================================================
// jump
static void op_impl_jump(byte2 addr)
{
    p8.p = addr;
}
static void op_keep_jump()
{
    op_impl_jump(get_stack_merge(1));
}
static void op_jump()
{
    op_keep_jump();
    popn(2);
}

// jump when
static void op_impl_jump_when(byte b, byte2 addr)
{
    if (b)
        op_impl_jump(addr);
}
static void op_keep_jump_when()
{
    op_impl_jump_when(get_stack(3), get_stack_merge(1));
}
static void op_jump_when()
{
    op_keep_jump_when();
    popn(3);
}

// jump if
static void op_impl_jump_if(byte b, byte2 true_addr, byte2 false_addr)
{
    if (b)
        op_impl_jump(true_addr);
    else
        op_impl_jump(false_addr);
}
static void op_keep_jump_if()
{
    op_impl_jump_if(get_stack(5), get_stack(3), get_stack(1));
}
static void op_jump_if()
{
    op_keep_jump_if();
    popn(5);
}

//// maths ==========================================================
#define macro_def_op_binop(name, binop)                \
    static void op_##name()                            \
    {                                                  \
        set_stack(2, get_stack(2) binop get_stack(1)); \
        pop();                                         \
    }                                                  \
    static void op_keep_##name()                       \
    {                                                  \
        push(get_stack(2) binop get_stack(1));         \
    }

macro_def_op_binop(add, +);
macro_def_op_binop(sub, -);
macro_def_op_binop(mul, *);
macro_def_op_binop(pow, ^);
macro_def_op_binop(lshift, <<);
macro_def_op_binop(rshift, >>);
macro_def_op_binop(eq, ==);
macro_def_op_binop(neq, !=);
macro_def_op_binop(le, <);
macro_def_op_binop(nle, >=);
macro_def_op_binop(leq, <=);
macro_def_op_binop(nleq, >);

#define macro_def_op_unop(name, unop)    \
    static void op_##name()              \
    {                                    \
        set_stack(1, unop get_stack(1)); \
    }                                    \
    static void op_keep_##name()         \
    {                                    \
        push(unop get_stack(1));         \
    }

macro_def_op_unop(neg, !);

static void op_rand()
{
    push(rand() % 255);
}

//// video ==========================================================
static void op_impl_set_pix(byte c, byte x, byte y)
{
    printf("c %x, x %x, y %x, i %x\n", c, x, y, y * width + x);
    p8.video[(y * width + x) % screen_size] = c;
    p8.draw = 1;
}
static void op_keep_set_pix()
{
    op_impl_set_pix(get_stack(3), get_stack(2), get_stack(1));
}
static void op_set_pix() // c x y : sets the pixel at coord (x,y) to c
{
    op_keep_set_pix();
    popn(3);
}

//// input
static void op_impl_get_key(byte k)
{
    push(p8.key[k]);
}
static void op_keep_get_key()
{
    op_impl_get_key(get_stack(1));
}
static void op_get_key()
{
    op_keep_get_key();
    pop();
}

#define macro_op_name(name) \
    op_##name,              \
        op_keep_##name

static void (*ops[])(void) = {
    NULL,
    // data
    op_return,
    op_push,
    op_pop,
    op_rand,
    // logic
    macro_op_name(jump),
    macro_op_name(jump_when),
    macro_op_name(jump_if),
    // maths
    macro_op_name(add),
    macro_op_name(sub),
    macro_op_name(mul),
    macro_op_name(pow),
    macro_op_name(lshift),
    macro_op_name(eq),
    macro_op_name(neq),
    macro_op_name(le),
    macro_op_name(nle),
    macro_op_name(leq),
    macro_op_name(nleq),
    macro_op_name(neg),
    // input
    macro_op_name(get_key),
    //video
    macro_op_name(set_pix)};

byte p8_init()
{
    printf("[p8 log] num ops %i\n", num_ops);

    srand(time(NULL));

    int i;

    for (i = 0; i < mem_size; i++)
        p8.memory[i] = 0;

    for (i = 0; i < width * height; i++)
        p8.video[i] = 0xff;
    // p8.video[i] = rand() % 256;
    // p8.video[i] = 0x0;

    for (i = 0; i < rstack_size; i++)
        p8.rstack[i] = 0;

    for (i = 0; i < stack_size; i++)
        p8.stack[i] = 0;

    for (i = 0; i < key_size; i++)
        p8.key[i] = 0;

    p8.p = 0;
    p8.s = 0;
    p8.r = 0;
    p8.draw = 1;
    p8.error = 0;

    return 0;
}

static void p8_dump()
{
    printf("[p8 log] dump:\n p: %x, r: %x, s: %x\n", p8.p, p8.r, p8.s);
}

void p8_eval()
{
    byte b = p8.memory[p8.p];
    byte op = b % 128;

    p8_dump();
    printf("[p8 log] op: %x\n", op);

    p8.p++;

    if (op == 0)
        return;

    if (op < num_prog_ops + (num_ops - num_prog_ops) * 2)
    {
        (*ops[op])();
    }
    else
    {
        printf("[p8 error] unknown opcode: 0x%x\n", op);
        p8.error = 1;
    }
}