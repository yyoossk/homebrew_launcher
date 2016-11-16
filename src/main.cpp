#include "Application.h"
#include "system/memory.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include "dynamic_libs/padscore_functions.h"
#include "common/common.h"
#include "kernel/gx2sploit.h"
#include "menu/HomebrewMemory.h"

/* Entry point */
extern "C" int Menu_Main(void)
{
    //!*******************************************************************
    //!                   Initialize function pointers                   *
    //!*******************************************************************
    //! do OS (for acquire) and sockets first so we got logging
    log_init("192.168.178.3");

    //! *******************************************************************
    //! *    Check if our application needs to run the kexploit started   *
    //! *******************************************************************
    if(CheckKernelExploit() == 0)
    {
        return 0;
    }

    log_printf("Welcome to the Homebrew Launcher %s\n", HBL_VERSION);

    InitPadScoreFunctionPointers();
    log_printf("Function exports loaded\n");

    //! initialize homebrew memory layout
    HomebrewInitMemory();

    //!*******************************************************************
    //!                    Enter main application                        *
    //!*******************************************************************
    log_printf("Start main application\n");
    int returnCode = Application::instance()->exec();
    log_printf("Main application stopped\n");

    Application::destroyInstance();

    log_printf("HBL exit\n");
    log_deinit();

    return returnCode;
}

