#include "Unknown.h"
#include <unistd.h>

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

    UNUserFuncSet userFuncSet
        = unUserFuncSetMake();

    UNCompiler compiler = unCompilerMake();

    printf("compiling script\n");
    UNObjectFunc *programPtr = unCompilerCompileScript(
        &compiler,
        "test_script.un"
    );

    printf("compiling and adding func\n");
    unUserFuncSetAdd(
        &userFuncSet,
        "test_func",
        unCompilerCompileFuncFile(
            &compiler,
            "test_func.unf"
        )
    );

    printf("\nfreeing compiler\n");
    unCompilerFree(&compiler);

    UNVirtualMachine vm = unVirtualMachineMake(
        &nativeFuncSet,
        &userFuncSet
    );

    printf("running vm\n");
    UNInterpretResult result = 0;
    result = unVirtualMachineInterpret(
        &vm,
        programPtr
    );

    while(result == un_yielded){
        printf("yielded; sleeping for 1 sec\n");
        sleep(1);
        printf("resuming vm after yield\n");
        result = unVirtualMachineResume(&vm);
    }

    printf("\nfreeing program\n");
    unObjectFree((UNObject*)programPtr);

    printf("\nfreeing vm\n");
    unVirtualMachineFree(&vm);

    printf("\nfreeing native funcs\n");
    unNativeFuncSetFree(&nativeFuncSet);

    printf("\nfreeing user funcs\n");
    unUserFuncSetFree(&userFuncSet);
}