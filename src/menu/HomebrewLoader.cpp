#include <algorithm>
#include <string>
#include <string.h>

#include "HomebrewLoader.h"
#include "HomebrewMemory.h"
#include "fs/CFile.hpp"
#include "utils/logger.h"
#include "utils/StringTools.h"

HomebrewLoader * HomebrewLoader::loadToMemoryAsync(const std::string & file)
{
    HomebrewLoader * loader = new HomebrewLoader(file);
    loader->resumeThread();
    return loader;
}

void HomebrewLoader::executeThread()
{
    int result = loadToMemory();
    asyncLoadFinished(this, filepath, result);
}

int HomebrewLoader::loadToMemory()
{
    if(filepath.empty())
        return INVALID_INPUT;

    DEBUG_FUNCTION_LINE("ファイルの読み込み %s\n", filepath.c_str());

    CFile file(filepath, CFile::ReadOnly);
    if(!file.isOpen())
    {
        progressWindow.setTitle(StringTools::strfmt("ファイルを開くことができませんでした %s", StringTools::FullpathToFilename(filepath.c_str())));
        os_sleep(1);
        return FILE_OPEN_FAILURE;
    }

    u32 bytesRead = 0;
    u32 fileSize = file.size();

    progressWindow.setTitle(StringTools::strfmt("ファイルを読み込んでいます %s", StringTools::FullpathToFilename(filepath.c_str())));

    unsigned char *buffer = (unsigned char*) memalign(0x40, (fileSize + 0x3F) & ~0x3F);
    if(!buffer)
    {
        progressWindow.setTitle("メモリーが充分ではありません");
        os_sleep(1);
        return NOT_ENOUGH_MEMORY;
    }

    // Copy rpl in memory
    while(bytesRead < fileSize)
    {
        progressWindow.setProgress(100.0f * (f32)bytesRead / (f32)fileSize);

        u32 blockSize = 0x8000;
        if(blockSize > (fileSize - bytesRead))
            blockSize = fileSize - bytesRead;

        int ret = file.read(buffer + bytesRead, blockSize);
        if(ret <= 0)
        {
            DEBUG_FUNCTION_LINE("ファイルの読み取りに失敗しました %s\n", filepath.c_str());
            break;
        }

        bytesRead += ret;
    }

    progressWindow.setProgress((f32)bytesRead / (f32)fileSize);

    if(bytesRead != fileSize)
    {
        free(buffer);
        DEBUG_FUNCTION_LINE("ファイルの読み込みが完了していません %s, 終了 %i of %i バイト\n", filepath.c_str(), bytesRead, fileSize);
        progressWindow.setTitle("ファイル読み込み失敗");
        os_sleep(1);
        return FILE_READ_ERROR;
    }

    HomebrewInitMemory();

    int ret = HomebrewCopyMemory(buffer, bytesRead);

    free(buffer);

    if(ret < 0)
    {
        progressWindow.setTitle("メモリ不足");
        os_sleep(1);
        return NOT_ENOUGH_MEMORY;
    }

    return fileSize;
}
