#include "Application.h"
#include "system/memory.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include "dynamic_libs/padscore_functions.h"
#include "common/common.h"

/* Entry point */
extern "C" int Menu_Main(void)
{
    //!*******************************************************************
    //!                   Initialize function pointers                   *
    //!*******************************************************************
    //! do OS (for acquire) and sockets first so we got logging
//    InitOSFunctionPointers();
//    InitSocketFunctionPointers();

    log_init("192.168.178.3");
    log_print("Starting launcher\n");

/*
    InitFSFunctionPointers();
    InitGX2FunctionPointers();
    InitSysFunctionPointers();
    InitVPadFunctionPointers();
    InitPadScoreFunctionPointers();
    InitAXFunctionPointers();
*/
    InitPadScoreFunctionPointers();
    log_print("Function exports loaded\n");

    //!*******************************************************************
    //!                    Enter main application                        *
    //!*******************************************************************
    log_printf("Start main application\n");
    int returnCode = Application::instance()->exec();
    log_printf("Main application stopped\n");

    Application::destroyInstance();

    log_deinit();

    return returnCode;
}

