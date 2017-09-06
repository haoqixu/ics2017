#include "cpu/exec.h"

make_EHelper(mov);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);

make_EHelper(call);
make_EHelper(call_rm);
make_EHelper(ret);
make_EHelper(push);
make_EHelper(pop);
make_EHelper(sub);
make_EHelper(xor);

make_EHelper(movsx);
make_EHelper(movzx);
make_EHelper(lea);

make_EHelper(adc);
make_EHelper(sbb);
make_EHelper(mul);
make_EHelper(imul);
make_EHelper(div);
make_EHelper(idiv);

make_EHelper(setcc);

make_EHelper(jcc);
make_EHelper(jmp);
make_EHelper(jmp_rm);

make_EHelper(nop);
