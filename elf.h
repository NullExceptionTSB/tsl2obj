#pragma once
#include <stdint.h>

#define ELF_MAGIC       0x464C457F        
#define ELF_STINFO(t,b) ((((uint8_t)b)<<4)|(((uint8_t)t)&0xf))

#pragma pack(push, 1)
typedef enum _ELF_ST_TYPE{
    STT_NOTYPE,
    STT_OBJECT,
    STT_FUNC,
    STT_SECTION,
    STT_FILE,
    STT_COMMON,
    STT_TLS,
    STT_NUM,
    STT_LOOS = 0x10,
    STT_HIOS = 0x12,
    STT_LOPROC = 0x13,
    STT_HIPROC = 0x15
}elfsttype;

typedef enum _ELF_ST_BIND{
    STB_LOCAL,
    STB_GLOBAL,
    STB_WEAK,
    STB_NUM,
    STB_LOOS = 0x10,
    STB_HIOS = 0x12,
    STB_LOPROC = 0x13,
    STB_HIPROC = 0x15
}elfstbind;

typedef enum _ELF_ST_VISIBILITY{
    STV_DEFAULT,
    STV_INTERNAL,
    STV_HIDDEN,
    STV_PROTECTED
}elfstvisibility;

typedef enum _ELF_ETYPE{
    ET_NONE,
    ET_REL,
    ET_EXEC,
    ET_DYN,
    ET_CORE,
    ET_LOOS = 0xFE00,
    ET_HIOS = 0xFEFF,
    ET_LOPROC = 0xFF00,
    ET_HIPROC = 0xFFFF
}elfetype;

typedef enum _ELF_SHTYPE{
    SHT_NULL,
    SHT_PROGBITS,
    SHT_SYMTAB,
    SHT_STRTAB,
    SHT_RELA,
    SHT_HASH,
    SHT_DYNAMIC,
    SHT_NOTE,
    SHT_NOBITS,
    SHT_REL,
    SHT_SHLIB,
    SHT_DYNSYM,
    SHT_INIT_ARRAY,
    SHT_FINI_ARRAY,
    SHT_PREINIT_ARRAY,
    SHT_GROUP,
    SHT_SYMTAB_SHNDX,
    SHT_NUM,
    SHT_LOOS = 0x60000000
}elfshtype;
//incomplete but i didn't feel like writing
//the entire machine enum for no reason
typedef enum _ELF_EMACHINE{
    EM_UNSPECIFIED,
    EM_X86      = 0x0003,
    EM_AMD64    = 0x003E
}elfemachine;

typedef struct _ELF_EINDENT {
    uint32_t    ei_mag;
    uint8_t     ei_class;       //bit size, 1 = 32, 2 = 64
    uint8_t     ei_data;        //endianness, 1 = little, 2 = big
    uint8_t     ei_version;     //elf std version, currently 1
    uint8_t     ei_osabi;       //abi, 0 for relocatables
    uint8_t     ei_abiversion;  //abi version, 0 for relocatables
    uint8_t     ei_pad[7];      //padding
} elfeindent;

//placeholder, unnused for this project
typedef struct _ELF_PROGRAMHEADER{
    uint32_t    p_type;
} elfprghdr;

typedef struct _ELF_SECTIONHEADER {
    uint32_t    sh_name;        //offset off .shstrtab for sec name
    uint32_t    sh_type;        //see _ELF_SHTYPE enum
    size_t      sh_flags;       //see _ELF_SHFLAGS enum, bitflags
    uint8_t*    sh_addr;        //virtual address where sec should be loaded
    size_t      sh_offset;      //section offset in image
    size_t      sh_size;        //size of section in bytes
    uint32_t    sh_link;        //associated section index
    uint32_t    sh_info;        //extra section info
    size_t      sh_addralign;   //sec alignment, must be power of 2
    size_t      sh_entsize;     //entry size in bytes
} elfsechdr;

typedef struct _ELF_HDR {
    elfeindent  e_indent;
    uint16_t    e_type;         //see _ELF_ETYPE enum
    uint16_t    e_machine;      //see _ELF_EMACHINE enum
    uint32_t    e_version;      //same as ei_version
    uint8_t*    e_entry;        //entry point
    elfprghdr*  e_phoff;        //offset to program header table, 
                                //0x34 for 32bit, 0x40 for x64, 0 for obj
    elfsechdr*  e_shoff;        //offset to section header table
    uint32_t    e_flags;
    uint16_t    e_ehsize;       //size of self
    uint16_t    e_phentsize;    //program header table entry size
    uint16_t    e_phnum;        //program header table entry count
    uint16_t    e_shentsize;    //section header table entry size
    uint16_t    e_shnum;        //section header table entry count
    uint16_t    e_shstrndx;     //section header index containing section names
} elfheader;

//ok so for some mother fucking reason some genius decided to change the
//structure layout between 32 bit and 64 bit symbols for absolutely
//no fucking reason
//not only that, the elf typedefs are so fucking backasswards that i 
//wanted to strangle myself when i found out that ELF64_WORD IS ACTUALLY DWORD
//AND THAT A WORD IS CALLED ELF64_HALF
//WHAT THE FUCK IS WRONG WITH YOU, JUST USE THE MOST COMMON STANDARD
//LOOK, IT'S SIMPLE, A WORD IS 16 BITS AND A DWORD IS 32 BITS, HENCE THE NAME,
//DOUBLEWORD. WHY MAKE THE FORMAT CONFUSING FOR ZERO FUCKING REASON YOU CUNTS

//btw i somehow managed to keep this entire header 80col formatted :-]

typedef struct _ELF_SYMBOL64 {
    uint32_t    st_name;
    uint8_t     st_info;
    uint8_t     st_other;
    uint16_t    st_shndx;
    uint8_t*    st_value;
    size_t      st_size;
} elfsymbol64;
//i couldn't find the fucking typedef anywhere
typedef struct _ELF_SYMBOL32 {
    uint32_t    st_name;
    uint8_t*    st_value;
    uint32_t    st_size;
    uint8_t     st_info;
    uint8_t     st_other;
    uint16_t    st_shndx;
} elfsymbol32;
#pragma pack(pop)
