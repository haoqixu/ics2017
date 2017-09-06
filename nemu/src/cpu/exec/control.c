#include "cpu/exec.h"

make_EHelper(jmp) {
  // the target address is calculated at the decode stage
  decoding.is_jmp = 1;

  print_asm("jmp %x", decoding.jmp_eip);
}

make_EHelper(jcc) {
  // the target address is calculated at the decode stage
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  decoding.is_jmp = t2;

  print_asm("j%s %x", get_cc_name(subcode), decoding.jmp_eip);
}

make_EHelper(jmp_rm) {
  decoding.jmp_eip = id_dest->val;
  decoding.is_jmp = 1;

  print_asm("jmp *%s", id_dest->str);
}

make_EHelper(call) {
  // the target address is calculated at the decode stage
  decoding.is_jmp = 1;
  if (decoding.is_operand_size_16) {
    decoding.jmp_eip = (decoding.seq_eip + id_src->val) & 0xffff;
    rtl_push(&decoding.seq_eip, 2);
  }
  else {
    decoding.jmp_eip = decoding.seq_eip + id_src->val;
    rtl_push(&decoding.seq_eip, 4);
  }

  print_asm("call %x", decoding.jmp_eip);
}

make_EHelper(ret) {
  decoding.is_jmp = 1;
  if (decoding.is_operand_size_16) {
    id_dest->val = 0;
    rtl_pop(&id_dest->val, 2);
  } else {
    rtl_pop(&id_dest->val, 4);
  }
  decoding.jmp_eip = id_dest->val;

  print_asm("ret");
}

make_EHelper(call_rm) {
  TODO();

  print_asm("call *%s", id_dest->str);
}
