#include <gctypes.h>
#include "elf_abi.h"
#include "../../src/common/common.h"
#include "../../src/common/os_defs.h"

#define CODE_RW_BASE_OFFSET                             0
#define DATA_RW_BASE_OFFSET                             0

#define EXPORT_DECL(res, func, ...)                     res (* func)(__VA_ARGS__);

#define OS_FIND_EXPORT(handle, funcName, func)                    OSDynLoad_FindExport(handle, 0, funcName, &func)

typedef struct _private_data_t
{
    EXPORT_DECL(void *, MEMAllocFromDefaultHeapEx,int size, int align);
    EXPORT_DECL(void, MEMFreeToDefaultHeap,void *ptr);

    EXPORT_DECL(void*, memcpy, void *p1, const void *p2, unsigned int s);
    EXPORT_DECL(void*, memset, void *p1, int val, unsigned int s);
    EXPORT_DECL(void, OSFatal, const char* msg);
    EXPORT_DECL(void, DCFlushRange, const void *addr, u32 length);
    EXPORT_DECL(void, ICInvalidateRange, const void *addr, u32 length);
    EXPORT_DECL(int, __os_snprintf, char* s, int n, const char * format, ...);
    EXPORT_DECL(void, exit, int);

    EXPORT_DECL(int, SYSRelaunchTitle, int argc, char** argv);
} private_data_t;

static unsigned int load_elf_image (private_data_t *private_data, unsigned char *elfstart)
{
	Elf32_Ehdr *ehdr;
	Elf32_Phdr *phdrs;
	unsigned char *image;
	int i;

	ehdr = (Elf32_Ehdr *) elfstart;

	if(ehdr->e_phoff == 0 || ehdr->e_phnum == 0)
		return 0;

	if(ehdr->e_phentsize != sizeof(Elf32_Phdr))
		return 0;

	phdrs = (Elf32_Phdr*)(elfstart + ehdr->e_phoff);

	for(i = 0; i < ehdr->e_phnum; i++)
    {
		if(phdrs[i].p_type != PT_LOAD)
			continue;

		if(phdrs[i].p_filesz > phdrs[i].p_memsz)
			return 0;

		if(!phdrs[i].p_filesz)
			continue;

        unsigned int p_paddr = phdrs[i].p_paddr;

        // use correct offset address for executables and data access
		if(phdrs[i].p_flags & PF_X)
			p_paddr += CODE_RW_BASE_OFFSET;
        else
			p_paddr += DATA_RW_BASE_OFFSET;

		image = (unsigned char *) (elfstart + phdrs[i].p_offset);
		private_data->memcpy ((void *) p_paddr, image, phdrs[i].p_filesz);
        private_data->DCFlushRange((void*)p_paddr, phdrs[i].p_filesz);

		if(phdrs[i].p_flags & PF_X)
			private_data->ICInvalidateRange ((void *) phdrs[i].p_paddr, phdrs[i].p_memsz);
	}

    //! clear BSS
    Elf32_Shdr *shdr = (Elf32_Shdr *) (elfstart + ehdr->e_shoff);
    for(i = 0; i < ehdr->e_shnum; i++)
    {
        const char *section_name = ((const char*)elfstart) + shdr[ehdr->e_shstrndx].sh_offset + shdr[i].sh_name;
        if(section_name[0] == '.' && section_name[1] == 'b' && section_name[2] == 's' && section_name[3] == 's')
        {
            private_data->memset((void*)shdr[i].sh_addr, 0, shdr[i].sh_size);
            private_data->DCFlushRange((void*)shdr[i].sh_addr, shdr[i].sh_size);
        }
        else if(section_name[0] == '.' && section_name[1] == 's' && section_name[2] == 'b' && section_name[3] == 's' && section_name[4] == 's')
        {
            private_data->memset((void*)shdr[i].sh_addr, 0, shdr[i].sh_size);
            private_data->DCFlushRange((void*)shdr[i].sh_addr, shdr[i].sh_size);
        }
    }

	return ehdr->e_entry;
}

static void loadFunctionPointers(private_data_t * private_data)
{
    unsigned int coreinit_handle;

    EXPORT_DECL(int, OSDynLoad_Acquire, const char* rpl, u32 *handle);
    EXPORT_DECL(int, OSDynLoad_FindExport, u32 handle, int isdata, const char *symbol, void *address);

    OSDynLoad_Acquire = (int (*)(const char*, u32 *))OS_SPECIFICS->addr_OSDynLoad_Acquire;
    OSDynLoad_FindExport = (int (*)(u32, int, const char *, void *))OS_SPECIFICS->addr_OSDynLoad_FindExport;

    OSDynLoad_Acquire("coreinit", &coreinit_handle);

    unsigned int *functionPtr = 0;

    OSDynLoad_FindExport(coreinit_handle, 1, "MEMAllocFromDefaultHeapEx", &functionPtr);
    private_data->MEMAllocFromDefaultHeapEx = (void * (*)(int, int))*functionPtr;
    OSDynLoad_FindExport(coreinit_handle, 1, "MEMFreeToDefaultHeap", &functionPtr);
    private_data->MEMFreeToDefaultHeap = (void (*)(void *))*functionPtr;

    OS_FIND_EXPORT(coreinit_handle, "memcpy", private_data->memcpy);
    OS_FIND_EXPORT(coreinit_handle, "memset", private_data->memset);
    OS_FIND_EXPORT(coreinit_handle, "OSFatal", private_data->OSFatal);
    OS_FIND_EXPORT(coreinit_handle, "DCFlushRange", private_data->DCFlushRange);
    OS_FIND_EXPORT(coreinit_handle, "ICInvalidateRange", private_data->ICInvalidateRange);
    OS_FIND_EXPORT(coreinit_handle, "__os_snprintf", private_data->__os_snprintf);
    OS_FIND_EXPORT(coreinit_handle, "exit", private_data->exit);

    unsigned int sysapp_handle;
    OSDynLoad_Acquire("sysapp.rpl", &sysapp_handle);
    OS_FIND_EXPORT(sysapp_handle, "SYSRelaunchTitle", private_data->SYSRelaunchTitle);
}

int _start(int argc, char **argv)
{
    {
        private_data_t private_data;
        loadFunctionPointers(&private_data);

        if(ELF_DATA_ADDR != 0xDEADC0DE && ELF_DATA_SIZE > 0)
        {
            //! copy data to safe area before processing it
            unsigned char * pElfBuffer = (unsigned char *)private_data.MEMAllocFromDefaultHeapEx(ELF_DATA_SIZE, 4);
            if(pElfBuffer)
            {
                private_data.memcpy(pElfBuffer, (unsigned char*)ELF_DATA_ADDR, ELF_DATA_SIZE);
                MAIN_ENTRY_ADDR = load_elf_image(&private_data, pElfBuffer);
                private_data.MEMFreeToDefaultHeap(pElfBuffer);
            }
            ELF_DATA_ADDR = 0xDEADC0DE;
            ELF_DATA_SIZE = 0;
        }

        if((MAIN_ENTRY_ADDR != 0xDEADC0DE) && (MAIN_ENTRY_ADDR != 0))
        {
            int returnVal = ((int (*)(int, char **))MAIN_ENTRY_ADDR)(argc, argv);

            //! exit to miimaker and restart application on re-enter of another application
            if(returnVal != (int)EXIT_RELAUNCH_ON_LOAD)
            {
                MAIN_ENTRY_ADDR = 0xDEADC0DE;
                private_data.SYSRelaunchTitle(0, 0);
                private_data.exit(0);
            }
        }
    }

    return ( (int (*)(int, char **))(*(unsigned int*)OS_SPECIFICS->addr_OSTitle_main_entry) )(argc, argv);
}
