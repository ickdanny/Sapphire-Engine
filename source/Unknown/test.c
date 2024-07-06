#include "Unknown.h"

UNValue testNativeFunc(int argc, UNValue *argv){
    return unNumberValue(3.1415);
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
    unObjectFree((UNObject*)programPtr);
    unVirtualMachineFree(&vm);
    unCompilerFree(&compiler);
    unNativeFuncSetFree(&nativeFuncSet);
}