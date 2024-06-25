#include "Unknown.h"

void unTest(){
    UNVirtualMachine vm = unVirtualMachineMake();
    UNProgram program = unProgramMake();

    size_t litIndex = unProgramPushBackLiteral(
        &program,
        1.2
    );
    unProgramPushBackCode(&program, un_literal, 3);
    unProgramPushBackCode(&program, litIndex, 3);

    litIndex = unProgramPushBackLiteral(
        &program,
        3.4
    );
    unProgramPushBackCode(&program, un_literal, 3);
    unProgramPushBackCode(&program, litIndex, 3);

    unProgramPushBackCode(&program, un_add, 3);

    litIndex = unProgramPushBackLiteral(
        &program,
        5.6
    );
    unProgramPushBackCode(&program, un_literal, 3);
    unProgramPushBackCode(&program, litIndex, 3);

    unProgramPushBackCode(&program, un_divide, 3);
    unProgramPushBackCode(&program, un_negate, 3);
    unProgramPushBackCode(&program, un_return, 3);
    //unProgramDisassemble(&program);
    printf("running vm\n");
    unVirtualMachineInterpret(&vm, &program);
    unProgramFree(&program);
}