#include "Unknown.h"

void unTest(){
    UNVirtualMachine vm = unVirtualMachineMake();
    
    UNProgram program = unProgramMake();
    size_t litIndex = unProgramPushBackLiteral(
        &program,
        1.2
    );
    unProgramPushBackCode(&program, UN_LITERAL, 3);
    unProgramPushBackCode(&program, litIndex, 3);
    unProgramPushBackCode(&program, UN_RETURN, 3);
    unProgramDisassemble(&program);
    unProgramFree(&program);
}