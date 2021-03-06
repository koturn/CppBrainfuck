/*!
 * @file GeneratorC.hpp
 * @brief C-source generator
 * @author koturn
 */
#ifndef GENERATOR_C_HPP
#define GENERATOR_C_HPP

#include <iostream>

#include "SourceGenerator.hpp"


/*!
 * @brief C-source generator
 */
class GeneratorC : public SourceGenerator<GeneratorC>
{
private:
  friend class CodeGenerator<GeneratorC>;

public:
  explicit GeneratorC(std::ostream& oStream, const std::string& indent_="  ") CODE_GENERATOR_NOEXCEPT :
    SourceGenerator<GeneratorC>(oStream, indent_)
  {}

protected:
  void
  emitHeaderImpl() CODE_GENERATOR_NOEXCEPT
  {
    oStream << "#include <signal.h>\n"
               "#include <stdio.h>\n"
               "#include <stdlib.h>\n"
               "#include <string.h>\n\n"
               "#define MEMORY_SIZE 65536\n\n"
               "#ifdef _MSC_VER\n"
               "#  define debugbreak __debugbreak\n"
               "#else\n"
               "__attribute__((gnu_inline, always_inline))\n"
               "__inline__ static void\n"
               "debugbreak(void)\n"
               "{\n"
               "#  if defined(__i386__) || defined(__x86_64__)\n"
               "  __asm__ volatile(\"int $0x03\");\n"
               "#  elif defined(__thumb__)\n"
               "  __asm__ volatile(\".inst 0xde01\");\n"
               "#  elif defined(__arm__) && !defined(__thumb__)\n"
               "  __asm__ volatile(\".inst 0xe7f001f0\");\n"
               "#  elif defined(__aarch64__) && defined(__APPLE__)\n"
               "  __builtin_trap();\n"
               "#  elif defined(__aarch64__)\n"
               "  __asm__ volatile(\".inst 0xd4200000\");\n"
               "#  elif defined(_WIN32)\n"
               "  __builtin_trap();\n"
               "#  else\n"
               "  raise(SIGTRAP);\n"
               "#  endif\n"
               "}\n"
               "#endif\n\n"
               "int\n"
               "main(void)\n"
               "{\n"
            << indent << "unsigned char memory[MEMORY_SIZE] = {0};\n"
            << indent << "unsigned char *p = memory;\n\n";
    indentLevel++;
  }

  void
  emitFooterImpl() CODE_GENERATOR_NOEXCEPT
  {
    oStream << indent << "putchar('\\n');\n\n"
            << indent << "return EXIT_SUCCESS;\n"
               "}"
            << std::endl;
  }

  void
  emitMovePointerImpl(int op1) CODE_GENERATOR_NOEXCEPT
  {
    emitIndent();
    if (op1 > 0) {
      if (op1 == 1) {
        oStream << "p++;\n";
      } else {
        oStream << "p += " << op1 << ";\n";
      }
    } else {
      if (op1 == -1) {
        oStream << "p--;\n";
      } else {
        oStream << "p -= " << -op1 << ";\n";
      }
    }
  }

  void
  emitAddImpl(int op1) CODE_GENERATOR_NOEXCEPT
  {
    emitIndent();
    if (op1 > 0) {
      if (op1 == 1) {
        oStream << "(*p)++;\n";
      } else {
        oStream << "*p += " << op1 << ";\n";
      }
    } else {
      if (op1 == -1) {
        oStream << "(*p)--;\n";
      } else {
        oStream << "*p -= " << -op1 << ";\n";
      }
    }
  }

  void
  emitPutcharImpl() CODE_GENERATOR_NOEXCEPT
  {
    emitIndent();
    oStream << "putchar(*p);\n";
  }

  void
  emitGetcharImpl() CODE_GENERATOR_NOEXCEPT
  {
    emitIndent();
    oStream << "*p = (unsigned char) getchar();\n";
  }

  void
  emitLoopStartImpl() CODE_GENERATOR_NOEXCEPT
  {
    emitIndent();
    oStream << "while (*p) {\n";
    indentLevel++;
  }

  void
  emitLoopEndImpl() CODE_GENERATOR_NOEXCEPT
  {
    indentLevel--;
    emitIndent();
    oStream << "}\n";
  }

  void
  emitIfImpl() CODE_GENERATOR_NOEXCEPT
  {
    emitIndent();
    oStream << "if (*p) {\n";
    indentLevel++;
  }

  void
  emitEndIfImpl() CODE_GENERATOR_NOEXCEPT
  {
    emitLoopEndImpl();
  }

  void
  emitAssignImpl(int op1) CODE_GENERATOR_NOEXCEPT
  {
    emitIndent();
    oStream << "*p = " << op1 << ";\n";
  }

  void
  emitSearchZeroImpl(int op1) CODE_GENERATOR_NOEXCEPT
  {
    emitIndent();
    if (op1 > 0) {
      if (op1 == 1) {
        oStream << "p = memchr(p, 0, sizeof(memory));\n";
      } else {
        oStream << "for (; *p; p += " << op1 << ");\n";
      }
    } else {
      if (op1 == -1) {
        oStream << "for (; *p; p--);\n";
      } else {
        oStream << "for (; *p; p -= " << -op1 << ");\n";
      }
    }
  }

  void
  emitAddVarImpl(int op1) CODE_GENERATOR_NOEXCEPT
  {
    emitIndent();
    if (op1 > 0) {
      oStream << "*(p + " << op1;
    } else {
      oStream << "*(p - " << -op1;
    }
    oStream << ") += *p;\n";
  }

  void
  emitSubVarImpl(int op1) CODE_GENERATOR_NOEXCEPT
  {
    emitIndent();
    if (op1 > 0) {
      oStream << "*(p + " << op1;
    } else {
      oStream << "*(p - " << -op1;
    }
    oStream << ") -= *p;\n";
  }

  void
  emitAddCMulVarImpl(int op1, int op2) CODE_GENERATOR_NOEXCEPT
  {
    emitIndent();
    if (op1 > 0) {
      oStream << "*(p + " << op1;
    } else {
      oStream << "*(p - " << -op1;
    }
    oStream << ") += *p * " << op2 << ";\n";
  }

  void
  emitInfLoopImpl() CODE_GENERATOR_NOEXCEPT
  {
    emitIndent();
    oStream << "if (*p) {\n";
    emitIndent();
    oStream << indent << "for (;;);\n";
    emitIndent();
    oStream << "}\n";
  }

  void
  emitBreakPointImpl() CODE_GENERATOR_NOEXCEPT
  {
    emitIndent();
    oStream << "debugbreak();\n";
  }
};  // class GeneratorC

#endif  // GENERATOR_C_HPP
