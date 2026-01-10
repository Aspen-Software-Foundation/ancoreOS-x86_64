#ifndef RV_VM_H
#define RV_VM_H
// Copyright (c) 2026 Dcraftbg
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <stdint.h>
#include <stddef.h>

typedef struct RvVm RvVm;
struct RvVm {
    uint32_t regs[32];
    uint32_t ip;
    // faulting address
    uint32_t stval;

    int (*read)(RvVm* vm, uint32_t at, void* data, size_t size_bytes);
    int (*write32)(RvVm* vm, uint32_t at, uint32_t value, size_t size_bytes);

    union {
        struct {
            int (*on_syscall)(RvVm* vm);
            int (*on_breakpoint)(RvVm* vm);
        };
        int (*on_environ[2])(RvVm* vm);
    };
};
enum {
    RV_VM_ERR_NON32_BIT = 1,
    RV_VM_ERR_UNSUP_OPCODE,
    RV_VM_ERR_COUNT
};

int rv_vm_step(RvVm* vm);
#endif // RV_VM_H
#ifdef RV_VM_IMPLEMENTATION

#ifndef rv_vm_panic
# include <stdio.h>
# include <stdlib.h>
# define _RV_STRINGIFY0(x) # x
# define _RV_STRINGIFY(x) _RV_STRINGIFY0(x)
# define rv_vm_panic(...) (fprintf(stderr, __FILE__ ":" _RV_STRINGIFY(__LINE__) " " __VA_ARGS__), abort())
#endif
static inline uint32_t rv_get_inst_len16(uint32_t tag) {
    if((tag & 0x3) != 0x3) return 1;
    else if(((tag >> 2) & 0x7) != 0x7) return 2;
    else if(((tag >> 5) & 0x1) != 0x1) return 4;
    else return 0;
}
#define RV_R32_OP(dword) ((dword) & 0x7f)
#define RV_R32_RD(dword) (((dword) >> 7) & 0x1f)
#define RV_R32_R1(dword) (((dword) >> 15) & 0x1f)
#define RV_R32_R2(dword) (((dword) >> 20) & 0x1f)
#define RV_R32_IMM_U(dword) (((int32_t)(dword)) >> 12)
#define RV_R32_IMM_I(dword) (((int32_t)(dword)) >> 20)
#define RV_R32_IMM_S(dword) \
        (((((int32_t)(dword)) >> 7) & 0b11111) | \
        (((((int32_t)(dword)) >> 25)) << 5))
#define RV_R32_IMM_B(dword) \
        ((( \
            (((((int32_t)(dword)) >> 7 ) & 0b011110)      )| \
            (((((int32_t)(dword)) >> 25) & 0b111111) << 5 )| \
            (((((int32_t)(dword)) >> 7 ) & 0b000001) << 11)| \
            (((((int32_t)(dword)) >> 30) & 0b000001) << 12) \
        ) << 20 ) >> 20)

static int32_t RV_R32_IMM_J(int32_t dword) {
    int32_t imm = dword >> 12;
    // Immediate:
    // iiiiiiiiiiiiiiiiiiii
    // Before:
    // daaaaaaaaaabcccccccc
    // After:
    // dccccccccbaaaaaaaaaa0
    int32_t a = (imm >> 9) & 0x3ff,
        b = (imm >> 8) & 0x1,
        c = (imm >> 0) & 0xff,
        d = (imm >> 19);
    return (d << 20) | (c << 12) | (b << 11) | (a << 1);
}

#define RV_R32_FUNCT3(dword) (((dword) >> 12) & 0x7)
#define RV_R32_FUNCT7(dword) (((dword) >> 25) & 0x7f)

#define RV_IMM_INT_MATH_OP 0x13
#define RV_REG_INT_MATH_OP 0x33
#define RV_STORE_OP        0x23
#define RV_LUI_OP          0x37
#define RV_AUIPC_OP        0x17
#define RV_LOAD_OP         0x03
#define RV_BRANCH_OP       0x63
#define RV_JAL_OP          0x6f
#define RV_JALR_OP         0x67
#define RV_ENVIRONMENT_OP  0x73

int rv_vm_step(RvVm* vm) {
    int e;
    vm->regs[0] = 0;
    uint32_t ipbuf[1];
    // TODO: don't copy over every time
    e = vm->read(vm, vm->ip, ipbuf, sizeof(ipbuf));
    if(e < 0) return e;

    uint32_t inst = ipbuf[0];
    uint32_t len = rv_get_inst_len16(inst);
    if(len != 2) return -RV_VM_ERR_NON32_BIT;
    switch(RV_R32_OP(inst)) {
    case RV_IMM_INT_MATH_OP: {
        int32_t a = vm->regs[RV_R32_R1(inst)],
                b = RV_R32_IMM_I(inst),
                c;
        switch(RV_R32_FUNCT3(inst)) {
        case 0x0:
            c = a + b;
            break;
        // TODO: some weird imm[5:11] = 0x00 Right now I do not support nor see the reason for this to even be specified explicitly.
        case 0x1:
            c = a << (b & 0x1f);
            break;
        case 0x2:
            c = a < b ? 1 : 0;
            break;
        case 0x3:
            c = (uint32_t)a < (uint32_t)b ? 1 : 0;
            break;
        case 0x4:
            c = a ^ b;
            break;
        case 0x5:
            if((b >> 5) == 0x20) {
                b &= 0x1f;
                c = a >> b;
            } else {
                b &= 0x1f;
                c = (uint32_t)a >> (uint32_t)b;
            }
            break;
        case 0x6:
            c = a | b;
            break;
        case 0x7:
            c = a & b;
            break;
        }
        vm->regs[RV_R32_RD(inst)] = c;
    } break;
    case RV_REG_INT_MATH_OP: {
        int32_t a = vm->regs[RV_R32_R1(inst)],
                b = vm->regs[RV_R32_R2(inst)],
                c;
        switch((RV_R32_FUNCT7(inst) << 4) | RV_R32_FUNCT3(inst)) {
        case 0x000:
            c = a + b;
            break;
        case 0x001:
            c = a << b;
            break;
        case 0x002:
            c = a < b ? 1 : 0;
            break;
        case 0x003:
            c = (uint32_t) a < (uint32_t) b ? 1 : 0;
            break;
        case 0x004:
            c = a ^ b;
            break;
        case 0x005:
            c = (uint32_t)a >> (uint32_t)b;
            break;
        case 0x006:
            c = a | b;
            break;
        case 0x007:
            c = a & b;
            break;
        case 0x010:
            c = a * b;
            break;
        case 0x015:
            c = (uint32_t)a / (uint32_t)b;
            break;
        case 0x017:
            c = (uint32_t)a % (uint32_t)b;
            break;
        case 0x200:
            c = a - b;
            break;
        case 0x205:
            c = a >> b;
            break;
        default:
            rv_vm_panic("Unsupported funct3: 0x%x | funct7: 0x%02x\n", RV_R32_FUNCT3(inst), RV_R32_FUNCT7(inst));
            return -RV_VM_ERR_UNSUP_OPCODE;
        }
        vm->regs[RV_R32_RD(inst)] = c;
    } break;
    case RV_STORE_OP: {
        int32_t shift = RV_R32_FUNCT3(inst) % 4;
        int32_t byte_size = (1 << shift);
        const int32_t masks[3] = {
            0xFF,
            0xFFFF,
            0xFFFFFFFF
        };
        e = vm->write32(vm, vm->regs[RV_R32_R1(inst)] + RV_R32_IMM_S(inst), vm->regs[RV_R32_R2(inst)] & masks[shift], byte_size);
        if(e < 0) return e;
    } break;
    case RV_LOAD_OP: {
        int32_t shift = RV_R32_FUNCT3(inst) % 4;
        int32_t byte_size = (1 << shift);
        int32_t sign_extend = ((4 - byte_size) * 8) & 
                           ((((RV_R32_FUNCT3(inst) & 0x4) >> 2) << 31) >> 31);
        int32_t data = 0;
        // TODO: support big endian
        e = vm->read(vm, vm->regs[RV_R32_R1(inst)] + RV_R32_IMM_I(inst), &data, byte_size);
        if(e < 0) return e;
        data = (data << (sign_extend*8)) >> (sign_extend*8);
        vm->regs[RV_R32_RD(inst)] = data;
    } break;
    case RV_LUI_OP:
        vm->regs[RV_R32_RD(inst)] = RV_R32_IMM_U(inst) << 12;
        break;
    case RV_AUIPC_OP:
        vm->regs[RV_R32_RD(inst)] = vm->ip + (RV_R32_IMM_U(inst) << 12);
        break;
    case RV_BRANCH_OP:
        int32_t res = 0;
        int32_t rs1 = vm->regs[RV_R32_R1(inst)],
                rs2 = vm->regs[RV_R32_R2(inst)];
        switch(RV_R32_FUNCT3(inst)/2) {
        case 0x00:
            res = rs1 == rs2;
            break;
        case 0x01:
            res = 0;
            break;
        case 0x02:
            res = rs1 < rs2;
            break;
        case 0x03:
            res = (uint32_t)rs1 < (uint32_t)rs2;
            break;
        }
        if(RV_R32_FUNCT3(inst) & 0x1) res = !res;
        if(res) vm->ip += RV_R32_IMM_B(inst) - 4;
        break;
    case RV_JAL_OP:
        // rd = PC+4; PC += imm
        vm->regs[RV_R32_RD(inst)] = vm->ip + 4;
        vm->ip += RV_R32_IMM_J(inst) - 4; 
        break;
    case RV_JALR_OP:
        // rd = PC+4; PC = rs1 + imm
        vm->regs[RV_R32_RD(inst)] = vm->ip + 4;
        vm->ip = vm->regs[RV_R32_R1(inst)] + RV_R32_IMM_I(inst) - 4; 
        break;
    case RV_ENVIRONMENT_OP:
        if(RV_R32_FUNCT3(inst) == 0x0 && (uint32_t)RV_R32_IMM_I(inst) < 2) {
            vm->ip += 4;
            return vm->on_environ[RV_R32_IMM_I(inst)](vm);
        }
        // fallthrough
    default:
        return -RV_VM_ERR_UNSUP_OPCODE;
    }
    vm->ip += 4;
    return 0;
}
#endif
