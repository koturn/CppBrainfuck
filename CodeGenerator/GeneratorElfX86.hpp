/*!
 * @file GeneratorElfX86.hpp
 * @brief x86 ELF binary generator
 * @author koturn
 */
#ifndef GENERATOR_ELF_X86_HPP
#define GENERATOR_ELF_X86_HPP

#include <iostream>

#include "BinaryGenerator.hpp"
#include "util/elfsubset.h"


/*!
 * @brief x86 ELF binary generator
 */
class GeneratorElfX86 : public BinaryGenerator<GeneratorElfX86>
{
private:
  friend class CodeGenerator<GeneratorElfX86>;
  //! Address of .text section
  static const Elf32_Addr kTextAddr;
  //! Address of .bss section
  static const Elf32_Addr kBssAddr;
  //! Program header size
  static const Elf32_Off kHeaderSize;
  //! Program footer size
  static const Elf32_Off kFooterSize;

public:
  GeneratorElfX86(std::ostream& oStream) :
    BinaryGenerator<GeneratorElfX86>(oStream)
  {}

protected:
  void
  emitHeaderImpl() CODE_GENERATOR_NOEXCEPT
  {
    // skip header
    Elf32_Ehdr ehdr;
    write(ehdr);
    Elf32_Phdr phdr;
    write(phdr);
    write(phdr);

    // - - - - - The start of program body - - - - - //
    // mov ecx, {kBssAddr}
    u8 opcode[] = {0xb9};
    write(opcode);
    write(static_cast<u32>(kBssAddr));
  }

  void
  emitFooterImpl() CODE_GENERATOR_NOEXCEPT
  {
    // Emit newline
    emitAssignImpl('\n');
    emitPutcharImpl();

    // mov eax, 0x01
    u8 opcode1[] = {0xb8};
    write(opcode1);
    write(static_cast<u32>(0x01));
    // mov ebx, 0x00
    u8 opcode2[] = {0xbb};
    write(opcode2);
    write(static_cast<u32>(0x00));
    // int 0x80
    u8 opcode3[] = {0xcd, 0x80};
    write(opcode3);
    // - - - - - The end of program body - - - - - //

    Elf32_Off codeSize = static_cast<Elf32_Off>(oStreamPtr->tellp()) - kHeaderSize;

    // - - - - - Program footer - - - - - //
    // Section string table (22bytes)
    const char kShStrTbl[] = "\0.text\0.shstrtbl\0.bss";
    write(kShStrTbl);

    // First section header
    Elf32_Shdr shdr;
    shdr.sh_name = 0;
    shdr.sh_type = SHT_NULL;
    shdr.sh_flags = 0x00000000;
    shdr.sh_addr = 0x00000000;
    shdr.sh_offset = 0x00000000;
    shdr.sh_size = 0x00000000;
    shdr.sh_link = 0x00000000;
    shdr.sh_info = 0x00000000;
    shdr.sh_addralign = 0x00000000;
    shdr.sh_entsize = 0x00000000;
    write(shdr);

    // Second section header (.shstrtbl)
    shdr.sh_name = 7;
    shdr.sh_type = SHT_STRTAB;
    shdr.sh_flags = 0x00000000;
    shdr.sh_addr = 0x00000000;
    shdr.sh_offset = kHeaderSize + codeSize;
    shdr.sh_size = sizeof(kShStrTbl);
    shdr.sh_link = 0x00000000;
    shdr.sh_info = 0x00000000;
    shdr.sh_addralign = 0x00000001;
    shdr.sh_entsize = 0x00000000;
    write(shdr);

    // Third section header (.text)
    shdr.sh_name = 1;
    shdr.sh_type = SHT_PROGBITS;
    shdr.sh_flags = SHF_EXECINSTR | SHF_ALLOC;
    shdr.sh_addr = kTextAddr + kHeaderSize;
    shdr.sh_offset = kHeaderSize;
    shdr.sh_size = codeSize;
    shdr.sh_link = 0x00000000;
    shdr.sh_info = 0x00000000;
    shdr.sh_addralign = 0x00000004;
    shdr.sh_entsize = 0x00000000;
    write(shdr);

    // Fourth section header (.bss)
    shdr.sh_name = 17;
    shdr.sh_type = SHT_NOBITS;
    shdr.sh_flags = SHF_ALLOC | SHF_WRITE;
    shdr.sh_addr = kBssAddr;
    shdr.sh_offset = 0x00001000;
    shdr.sh_size = 0x00010000;  // 65536 cells
    shdr.sh_link = 0x00000000;
    shdr.sh_info = 0x00000000;
    shdr.sh_addralign = 0x00000010;
    shdr.sh_entsize = 0x00000000;
    write(shdr);

    // - - - - - Program header - - - - - //
    oStreamPtr->seekp(0, std::ios_base::beg);
    // ELF header
    Elf32_Ehdr ehdr;
    ehdr.e_ident[EI_MAG0] = ELFMAG0;
    ehdr.e_ident[EI_MAG1] = ELFMAG1;
    ehdr.e_ident[EI_MAG2] = ELFMAG2;
    ehdr.e_ident[EI_MAG3] = ELFMAG3;
    ehdr.e_ident[EI_CLASS] = ELFCLASS32;
    ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
    ehdr.e_ident[EI_VERSION] = EV_CURRENT;
    ehdr.e_ident[EI_OSABI] = ELFOSABI_LINUX;
    ehdr.e_ident[EI_ABIVERSION] = 0x00;
    ehdr.e_ident[EI_PAD] = 0x00;
    ehdr.e_type = ET_EXEC;
    ehdr.e_machine = EM_386;
    ehdr.e_version = EV_CURRENT;
    ehdr.e_entry = kTextAddr + kHeaderSize;
    ehdr.e_phoff = sizeof(Elf32_Ehdr);
    ehdr.e_shoff = static_cast<Elf32_Off>(kHeaderSize + sizeof(kShStrTbl) + codeSize);
    ehdr.e_flags = 0x00000000;
    ehdr.e_ehsize = sizeof(Elf32_Ehdr);
    ehdr.e_phentsize = sizeof(Elf32_Phdr);
    ehdr.e_phnum = 2;
    ehdr.e_shentsize = sizeof(Elf32_Shdr);
    ehdr.e_shnum = 4;
    ehdr.e_shstrndx = 1;
    write(ehdr);

    // Program header
    Elf32_Phdr phdr;
    phdr.p_type = PT_LOAD;
    phdr.p_flags = PF_R | PF_X;
    phdr.p_offset = 0x00000000;
    phdr.p_vaddr = kTextAddr;
    phdr.p_paddr = kTextAddr;
    phdr.p_filesz = static_cast<Elf32_Word>(kHeaderSize + sizeof(kShStrTbl) + kFooterSize + codeSize);
    phdr.p_memsz = static_cast<Elf32_Word>(kHeaderSize + sizeof(kShStrTbl) + kFooterSize + codeSize);
    phdr.p_align = 0x00000100;
    write(phdr);

    // Program header for .bss (56 bytes)
    phdr.p_type = PT_LOAD;
    phdr.p_flags = PF_R | PF_W;
    phdr.p_offset = 0x00001000;
    phdr.p_vaddr = kBssAddr;
    phdr.p_paddr = kBssAddr;
    phdr.p_filesz = 0x00000000;
    phdr.p_memsz = 0x00010000;
    phdr.p_align = 0x00200000;
    write(phdr);

    oStreamPtr->seekp(0, std::ios_base::end);
  }

  void
  emitMovePointerImpl(int op1) CODE_GENERATOR_NOEXCEPT
  {
    if (op1 > 0) {
      if (op1 == 1) {
        // inc ecx
        u8 opcode[] = {0x41};
        write(opcode);
      } else {
        // add ecx, {op1}
        u8 opcode[] = {0x81, 0xc1};
        write(opcode);
        write(op1);
      }
    } else {
      if (op1 == -1) {
        // dec ecx
        u8 opcode[] = {0x49};
        write(opcode);
      } else {
        // sub ecx, {op1}
        u8 opcode[] = {0x81, 0xe9};
        write(opcode);
        write(-op1);
      }
    }
  }

  void
  emitAddImpl(int op1) CODE_GENERATOR_NOEXCEPT
  {
    if (op1 > 0) {
      if (op1 == 1) {
        // inc byte ptr [ecx]
        u8 opcode[] = {0xfe, 0x01};
        write(opcode);
      } else {
        // add byte ptr [ecx], {op1}
        u8 opcode[] = {0x80, 0x01};
        write(opcode);
        write(static_cast<u8>(op1));
      }
    } else {
      if (op1 == -1) {
        // dec byte ptr [ecx]
        u8 opcode[] = {0xfe, 0x09};
        write(opcode);
      } else {
        // sub byte ptr [ecx], {op1}
        u8 opcode[] = {0x80, 0x29};
        write(opcode);
        write(static_cast<u8>(-op1));
      }
    }
  }

  void
  emitPutcharImpl() CODE_GENERATOR_NOEXCEPT
  {
    // mov eax, 0x01
    u8 opcode1[] = {0xb8};
    write(opcode1);
    write(static_cast<u32>(0x04));
    // mov edx, 0x01
    u8 opcode2[] = {0xba};
    write(opcode2);
    write(static_cast<u32>(0x01));
    // mov ebx, 0x01
    u8 opcode3[] = {0xbb};
    write(opcode3);
    write(static_cast<u32>(0x01));
    // int 0x80
    u8 opcode4[] = {0xcd, 0x80};
    write(opcode4);
  }

  void
  emitGetcharImpl() CODE_GENERATOR_NOEXCEPT
  {
    // mov eax, 0x01
    u8 opcode1[] = {0xb8};
    write(opcode1);
    write(static_cast<u32>(0x03));
    // mov edx, 0x01
    u8 opcode2[] = {0xba};
    write(opcode2);
    write(static_cast<u32>(0x01));
    // mov ebx, 0x01
    u8 opcode3[] = {0xbb};
    write(opcode3);
    write(static_cast<u32>(0x00));
    // int 0x80
    u8 opcode4[] = {0xcd, 0x80};
    write(opcode4);
  }

  void
  emitLoopStartImpl() CODE_GENERATOR_NOEXCEPT
  {
    loopStack.push(oStreamPtr->tellp());
    // cmp byte ptr [ecx], 0x00
    u8 opcode1[] = {0x80, 0x39};
    write(opcode1);
    write(static_cast<u8>(0x00));
    // je 0x********
    u8 opcode2[] = {0x0f, 0x84};
    write(opcode2);
    write(static_cast<u32>(0x00000000));
  }

  void
  emitLoopEndImpl() CODE_GENERATOR_NOEXCEPT
  {
    std::ostream::pos_type pos = loopStack.top();
    int offset = static_cast<int>(pos - oStreamPtr->tellp()) - 1;
    if (offset - static_cast<int>(sizeof(u8)) < -128) {
      // jmp {offset} (near jump)
      u8 opcode = {0xe9};
      write(opcode);
      write(offset - sizeof(u32));
    } else {
      // jmp {offset} (short jump)
      u8 opcode = {0xeb};
      write(opcode);
      write(static_cast<u8>(offset - sizeof(u8)));
    }
    // fill loop start
    std::ostream::pos_type curPos = oStreamPtr->tellp();
    oStreamPtr->seekp(pos + static_cast<std::ostream::pos_type>(5), std::ios_base::beg);
    write(static_cast<u32>(curPos - oStreamPtr->tellp() - sizeof(u32)));
    oStreamPtr->seekp(0, std::ios_base::end);
    loopStack.pop();
  }

  void
  emitEndIfImpl() CODE_GENERATOR_NOEXCEPT
  {
    // fill loop start
    std::ostream::pos_type pos = loopStack.top();
    std::ostream::pos_type curPos = oStreamPtr->tellp();
    oStreamPtr->seekp(pos + static_cast<std::ostream::pos_type>(5), std::ios_base::beg);
    write(static_cast<u32>(curPos - oStreamPtr->tellp() - sizeof(u32)));
    oStreamPtr->seekp(0, std::ios_base::end);
    loopStack.pop();
  }

  void
  emitAssignImpl(int op1) CODE_GENERATOR_NOEXCEPT
  {
    // mov byte ptr [ecx], {op1}
    u8 opcode[] = {0xc6, 0x01};
    write(opcode);
    write(static_cast<u8>(op1));
  }

  void
  emitAddVarImpl(int op1) CODE_GENERATOR_NOEXCEPT
  {
    // mov al, byte ptr [ecx]
    u8 opcode1[] = {0x8a, 0x01};
    write(opcode1);
    // add byte ptr [ecx + {op1}], al
    if (op1 < -128 || 127 < op1) {
      u8 opcode2[] = {0x00, 0x81};
      write(opcode2);
      write(static_cast<u32>(op1));
    } else {
      u8 opcode2[] = {0x00, 0x41};
      write(opcode2);
      write(static_cast<u8>(op1));
    }
  }

  void
  emitSubVarImpl(int op1) CODE_GENERATOR_NOEXCEPT
  {
    // mov al, byte ptr [ecx]
    u8 opcode1[] = {0x8a, 0x01};
    write(opcode1);
    // sub byte ptr [rbx + {op1}], al
    if (op1 < -128 || 127 < op1) {
      u8 opcode2[] = {0x28, 0x81};
      write(opcode2);
      write(static_cast<u32>(op1));
    } else {
      u8 opcode2[] = {0x28, 0x41};
      write(opcode2);
      write(static_cast<u8>(op1));
    }
  }

  void
  emitAddCMulVarImpl(int op1, int op2) CODE_GENERATOR_NOEXCEPT
  {
    if (op2 > 0) {
      // mov al, {op2}
      u8 opcode1[] = {0xb0};
      write(opcode1);
      write(static_cast<u8>(op2));
      // mul byte ptr [ecx]
      u8 opcode2[] = {0xf6, 0x21};
      write(opcode2);
      // add byte ptr [ecx + {op1}], al
      if (op1 < -128 || 127 < op1) {
        u8 opcode2[] = {0x00, 0x81};
        write(opcode2);
        write(static_cast<u32>(op1));
      } else {
        u8 opcode2[] = {0x00, 0x41};
        write(opcode2);
        write(static_cast<u8>(op1));
      }
    } else {
      // mov al, {-op2}
      u8 opcode1[] = {0xb0};
      write(opcode1);
      write(static_cast<u8>(-op2));
      // mul byte ptr [ecx]
      u8 opcode2[] = {0xf6, 0x21};
      write(opcode2);
      // sub byte ptr [ecx], al
      if (op1 < -128 || 127 < op1) {
        u8 opcode2[] = {0x28, 0x81};
        write(opcode2);
        write(static_cast<u32>(op1));
      } else {
        u8 opcode2[] = {0x28, 0x41};
        write(opcode2);
        write(static_cast<u8>(op1));
      }
    }
  }

  void
  emitInfLoopImpl() CODE_GENERATOR_NOEXCEPT
  {
    emitIfImpl();
    // jmp {offset} (near jump)
    u8 opcode = {0xeb};
    write(opcode);
    write(static_cast<u8>(-2));
    emitEndIfImpl();
  }
};  // class GeneratorElfX86


const Elf32_Addr GeneratorElfX86::kTextAddr = 0x04048000;
const Elf32_Addr GeneratorElfX86::kBssAddr = 0x04248000;
const Elf32_Off GeneratorElfX86::kHeaderSize = sizeof(Elf32_Ehdr) + sizeof(Elf32_Phdr) * 2;
const Elf32_Off GeneratorElfX86::kFooterSize = sizeof(Elf32_Shdr) * 4;



#endif  // GENERATOR_ELF_X86_HPP
