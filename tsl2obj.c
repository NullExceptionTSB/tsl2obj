#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "elf.h"

//only 64 bit for now, but will be fixed for 32 bit later   

void fail(char* msg) {
    puts(msg);
    exit(-1);
}

int is_64bit() {
    return (sizeof(size_t)==8);
}

elfheader* generate_elf_headers_obj() {
    elfheader* elf_return = calloc(1, sizeof(elfheader));
    elf_return->e_indent.ei_mag =       ELF_MAGIC;
    elf_return->e_indent.ei_class =     is_64bit()+1;
    elf_return->e_indent.ei_data =      1;
    elf_return->e_indent.ei_version =   1;
    
    elf_return->e_type =    ET_REL;
    elf_return->e_machine = is_64bit() ? EM_AMD64 : EM_X86;
    elf_return->e_version = elf_return->e_indent.ei_version;
    elf_return->e_ehsize = sizeof(elfheader);
    elf_return->e_shentsize = sizeof(elfsechdr);
    
    return elf_return;
}

uint32_t shstrtab_get_offset(char* string, uint8_t* shstrtab, size_t shstrs_size) {
    for (int i = 1; i < shstrs_size; i++) {
        if (shstrtab[i] == '.') {
            if (!strcmp(string, shstrtab+i)) 
                return i;
        }
    }
    return 0;
}

uint8_t* generate_shstrtab(const char* shstrs[], size_t shstrs_count, size_t* shstrs_size) {
    size_t shstrtab_size = 1;

    for (int i = 0; i < shstrs_count; i++) {
        shstrtab_size += strlen(shstrs[i])+1;
    }

    uint8_t* shstrtab_data = calloc(1, shstrtab_size);
    size_t buf_offset = 1;

    for (int i = 0; i < shstrs_count; i++) {
        memcpy(shstrtab_data+buf_offset, shstrs[i], strlen(shstrs[i])+1);
        buf_offset += strlen(shstrs[i])+1;
    }
    if (shstrs_size) *shstrs_size = shstrtab_size;
    return shstrtab_data;
}

int main(int argc, char* argv[]) {
    if (argc != 5) 
        fail("Usage: tsl2obj [binary file] [object file] [data symbol name] [size symbol name]");
    
    //load binary file into memory
    uint8_t* binfile_data = NULL;
    FILE* binfile = fopen(argv[1], "rb");
    size_t binfile_sz = 0;

    if (!binfile)
        fail("F: failed to open binary file");

    fseek(binfile, 0, SEEK_END);
    binfile_sz = ftell(binfile);
    fseek(binfile, 0, SEEK_SET);

    if (!binfile_sz)
        fail("F: binary file is empty");

    binfile_data = calloc(1, binfile_sz);
    if (!binfile_data)
        fail("F: failed to allocate binary file buffer, file is probably too large");
    
    size_t binfile_actual_size = fread(binfile_data, 1, binfile_sz, binfile);
    if (binfile_actual_size != binfile_sz) {
        puts("W: binary file not read entirely, chance of data corruption");
        binfile_sz = binfile_actual_size;
    }
    fclose(binfile);
    
    //generate ELF file structures
    int data_len = binfile_sz + sizeof(int);
    elfheader* elf_header = generate_elf_headers_obj();
    elfsechdr* elf_data_section_header = calloc(1, sizeof(elfsechdr));
    elfsechdr* elf_null_section_header = calloc(1, sizeof(elfsechdr));
    elfsechdr* elf_shstrtab_section_header = calloc(1, sizeof(elfsechdr));
    elfsechdr* elf_strtab_section_header = calloc(1, sizeof(elfsechdr));
    elfsechdr* elf_symtab_section_header = calloc(1, sizeof(elfsechdr));

    //generate .shstrtab segment
    size_t shstrtab_len = 0;
    const char* sections[] = { ".symtab", ".strtab", ".shstrtab", ".data" };
    uint8_t* shstrtab = generate_shstrtab(sections, 4, &shstrtab_len);

    //generate .strtab segment
    size_t strtab_len = strlen(argv[3]) + strlen(argv[4]) + 3;
    uint8_t* strtab = calloc(1, strtab_len);
    memcpy(strtab + 1, argv[3], strlen(argv[3])+1);
    memcpy(strtab + strlen(argv[3]) + 2, argv[4], strlen(argv[4]) + 1);

    //generate .symtab segment
    size_t symtab_len = sizeof(elfsymbol64) * 3;
    elfsymbol64* symtab = calloc(3, sizeof(elfsymbol64)); //index 1 is data, index 2 is length

    symtab[1].st_name = 1;
    symtab[1].st_info = ELF_STINFO(STT_NOTYPE, STB_GLOBAL);
    symtab[1].st_shndx = 1;
    symtab[1].st_value = 0; //0 from start of *section*, not image
    symtab[1].st_size = binfile_sz;

    symtab[2].st_name = strlen(argv[3])+2;
    symtab[2].st_info = ELF_STINFO(STT_NOTYPE, STB_GLOBAL);
    symtab[2].st_shndx = 1;
    symtab[2].st_value = (uint8_t*)binfile_sz; //right after the last thing
    symtab[2].st_size = sizeof(int);

    //init sh_name
    elf_data_section_header->sh_name = shstrtab_get_offset(".data", shstrtab, shstrtab_len);
    elf_symtab_section_header->sh_name = shstrtab_get_offset(".symtab", shstrtab, shstrtab_len);
    elf_strtab_section_header->sh_name = shstrtab_get_offset(".strtab", shstrtab, shstrtab_len);
    elf_shstrtab_section_header->sh_name = shstrtab_get_offset(".shstrtab", shstrtab, shstrtab_len);

    //init sh_type
    elf_data_section_header->sh_type = SHT_PROGBITS;
    elf_symtab_section_header->sh_type = SHT_SYMTAB;
    elf_strtab_section_header->sh_type = SHT_STRTAB;
    elf_shstrtab_section_header->sh_type = SHT_STRTAB;
    //init sh_offset
    elf_data_section_header->sh_offset = sizeof(elfheader);
    elf_symtab_section_header->sh_offset = elf_data_section_header->sh_offset + data_len;
    elf_strtab_section_header->sh_offset = elf_symtab_section_header->sh_offset + symtab_len;
    elf_shstrtab_section_header->sh_offset = elf_strtab_section_header->sh_offset + strtab_len;
    elf_header->e_shoff = (elfsechdr*)(elf_shstrtab_section_header->sh_offset + shstrtab_len);

    //init sh_addralign
    elf_data_section_header->sh_addralign = 1;
    elf_symtab_section_header->sh_addralign = 8;
    elf_strtab_section_header->sh_addralign = 1;
    elf_shstrtab_section_header->sh_addralign = 1;

    //init sh_size
    elf_data_section_header->sh_size = data_len;
    elf_symtab_section_header->sh_size = symtab_len;
    elf_strtab_section_header->sh_size = strtab_len;
    elf_shstrtab_section_header->sh_size = shstrtab_len;

    //init rest of headers
    elf_header->e_shnum = 5;
    elf_header->e_shstrndx = 4;
    elf_symtab_section_header->sh_link = 3;
    elf_symtab_section_header->sh_info = 1;
    elf_symtab_section_header->sh_entsize = sizeof(elfsymbol64);
    elf_data_section_header->sh_flags = 0x3; //funny number

    //open output file
    FILE* output = fopen(argv[2], "wb");
    fwrite(elf_header, 1, sizeof(elfheader), output);
    fwrite(binfile_data, 1, binfile_sz, output);
    fwrite(&binfile_sz, sizeof(int), 1, output);
    fwrite(symtab, 1, symtab_len, output);
    fwrite(strtab, 1, strtab_len, output);
    fwrite(shstrtab, 1, shstrtab_len, output);

    fwrite(elf_null_section_header, 1, sizeof(elfsechdr), output);
    fwrite(elf_data_section_header, 1, sizeof(elfsechdr), output);
    fwrite(elf_symtab_section_header, 1, sizeof(elfsechdr), output);
    fwrite(elf_strtab_section_header, 1, sizeof(elfsechdr), output);
    fwrite(elf_shstrtab_section_header, 1, sizeof(elfsechdr), output);

    fclose(output);
}