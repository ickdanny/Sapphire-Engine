#include "Unknown.h"

void unTest(){
    UNVirtualMachine vm = unVirtualMachineMake();
    UNCompiler compiler = unCompilerMake();

    printf("compiling\n");
    UNObjectFunc *programPtr = unCompilerCompile(
        &compiler,
        "test_script.un"
    );

    printf("running vm\n");
    unVirtualMachineInterpret(
        &vm,
        programPtr
    );
    unObjectFree((UNObject*)programPtr);
    unVirtualMachineFree(&vm);
    unCompilerFree(&compiler);
}