#include "Unknown.h"

UNValue testNativeFunc(int argc, UNValue *argv){
    return unFloatValue(3.1415);
}

void unTest(){
    UNNativeFuncSet nativeFuncSet
        = unNativeFuncSetMake();
    unNativeFuncSetAdd(
        &nativeFuncSet,
        "pi",
        testNativeFunc
    );

    UNVirtualMachine vm = unVirtualMachineMake(
        &nativeFuncSet
    );
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

    printf("\nfreeing program\n");
    unObjectFree((UNObject*)programPtr);

    printf("\nfreeing vm\n");
    unVirtualMachineFree(&vm);

    printf("\nfreeing compiler\n");
    unCompilerFree(&compiler);

    printf("\nfreeing native funcs\n");
    unNativeFuncSetFree(&nativeFuncSet);
}