#include "Unknown.h"

void unTest(){
    UNVirtualMachine vm = unVirtualMachineMake();
    UNCompiler compiler = unCompilerMake();

    printf("compiling\n");
    UNProgram program = unCompilerCompile(
        &compiler,
        "test_script.un"
    );

    printf("running vm\n");
    unVirtualMachineInterpret(&vm, &program);
    unProgramFree(&program);
    unVirtualMachineFree(&vm);
    unCompilerFree(&compiler);
}