#include <cstdlib>
#include <array>
#include <exception>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include "ArgumentParser.hpp"
#include "Brainfuck.hpp"
#include "version.h"


#if __cplusplus >= 201103L
#  define NOEXCEPT  noexcept
#else
#  define NOEXCEPT  throw()
#endif


static void
showVersion() NOEXCEPT;

static std::string
removeDirectoryPart(const std::string& filepath) NOEXCEPT;

static std::string
removeSuffix(const std::string& filename) NOEXCEPT;


int
main(int argc, const char* argv[])
{
  std::unordered_map<std::string, Brainfuck::Target> targetMap{
    {"c", Brainfuck::Target::kC},
    {"xbyakc", Brainfuck::Target::kXbyakC},
    {"winx86", Brainfuck::Target::kWinX86},
    {"winx64", Brainfuck::Target::kWinX64},
    {"elfx86", Brainfuck::Target::kElfX86},
    {"elfx64", Brainfuck::Target::kElfX64},
    {"elfarmeabi", Brainfuck::Target::kElfArmeabi}
  };

  try {
    ArgumentParser ap(argv[0]);
    ap.add('h', "help", "Show help and exit this program");
    ap.add('m', "minify", "Remove all non-brainfuck characters from source code");
    ap.add('t', "target", ArgumentParser::OptionType::kRequiredArgument,
        "Specify target language" + ap.getNewlineDescription()
        + "- c:      Transpile to C source" + ap.getNewlineDescription()
        + "- xbyakc: Dump xbyak code as C source" + ap.getNewlineDescription()
        + "- winx86: Compile to x86 EXE binary" + ap.getNewlineDescription()
        + "- winx64: Compile to x64 EXE binary" + ap.getNewlineDescription()
        + "- elfx86: Compile to x86 ELF binary" + ap.getNewlineDescription()
        + "- elfx64: Compile to x64 ELF binary" + ap.getNewlineDescription()
        + "- elfarmeabi: Compile to ARM EABI ELF binary",
        "TARGET", "");
    ap.add('v', "version", "Show version");
    ap.add('O', "optimize", ArgumentParser::OptionType::kRequiredArgument,
        "Specify optimization level" + ap.getNewlineDescription()
        + "Default value: 1" + ap.getNewlineDescription()
        + "- 0: Execute directly" + ap.getNewlineDescription()
        + "- 1: Compile to IR code and execute" + ap.getNewlineDescription()
        + "- 2: Compile to native code and execute", "LEVEL", 1);
    ap.add("dump-ir", "Dump IR code");
    ap.add("enable-synchronize-with-stdio", "Disable synchronization between std::cout/std::cin and <cstdio>");
    ap.add("heap-size", ArgumentParser::OptionType::kRequiredArgument,
        "Specify heap memory size" + ap.getNewlineDescription()
        + "Default value: 65536", "HEAP_SIZE", 65536);
    ap.parse(argc, argv);

    if (ap.get<bool>("help")) {
      ap.showUsage();
      return EXIT_SUCCESS;
    }
    if (ap.get<bool>("version")) {
      showVersion();
      return EXIT_SUCCESS;
    }
    if (!ap.get<bool>("enable-synchronize-with-stdio")) {
      std::cin.tie(nullptr);
      std::ios::sync_with_stdio(false);
    }
    std::size_t heapSize = ap.get<std::size_t>("heap-size");
    int optLevel = ap.get<int>("optimize");

    std::vector<std::string> args = ap.getArguments();
    if (args.size() == 0) {
      std::cerr << "Please specify one or more brainfuck source code" << std::endl;
      return EXIT_FAILURE;
    }

    Brainfuck bf;
    if (ap.get<bool>("minify")) {
      for (const auto& filename : args) {
        bf.load(filename);
        bf.trim();
        std::cout << bf.getSource() << std::endl;
        return EXIT_SUCCESS;
      }
    }
    if (ap.get<bool>("dump-ir")) {
      for (const auto& filename : args) {
        bf.load(filename);
        bf.trim();
        bf.compile();
        bf.dumpIR();
      }
      return EXIT_SUCCESS;
    }
    const std::string& target = ap.get("target");
    if (target != "") {
      if (targetMap.find(target) == targetMap.end()) {
        std::cerr << "Option -t, --target: Invalid value: \"" << target << "\" is specified" << std::endl;
        return EXIT_FAILURE;
      }
      bf.load(args[0]);
      bf.trim();
      bf.compile(Brainfuck::CompileType::kJit);
      Brainfuck::Target targetType = targetMap[target];
      std::string basename = removeSuffix(removeDirectoryPart(args[0]));
      switch (targetType) {
        case Brainfuck::Target::kC:
        case Brainfuck::Target::kXbyakC:
          bf.emit(std::cout, targetType);
          break;
        case Brainfuck::Target::kWinX86:
        case Brainfuck::Target::kWinX64:
          {
            std::ofstream ofs(basename + ".exe", std::ios::binary);
            if (!ofs.is_open()) {
              std::cerr << "Failed to open: " << "a.out" << std::endl;
              return EXIT_FAILURE;
            }
            bf.emit(ofs, targetType);
          }
          break;
        case Brainfuck::Target::kElfX86:
        case Brainfuck::Target::kElfX64:
        case Brainfuck::Target::kElfArmeabi:
          {
            std::ofstream ofs(basename + ".out", std::ios::binary);
            if (!ofs.is_open()) {
              std::cerr << "Failed to open: " << "a.out" << std::endl;
              return EXIT_FAILURE;
            }
            bf.emit(ofs, targetType);
          }
          break;
        default:
          assert(false);
      }
      return EXIT_SUCCESS;
    }

    for (const auto& filename : args) {
      if (filename == "-") {
        bf.load(std::cin);
      } else {
        bf.load(filename);
      }
      bf.trim();
      if (optLevel == 1) {
        bf.compile(Brainfuck::CompileType::kIR);
      } else if (optLevel > 1){
        bf.compile(Brainfuck::CompileType::kJit);
      }
      bf.execute(heapSize);
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  return EXIT_SUCCESS;
}


static void
showVersion() NOEXCEPT
{
  const char* username = std::getenv("USER");
  std::cout << "<<< CppBrainfuck >>>\n\n";
  if (username != NULL) {
    std::cout << "Compiled by: " << username << "\n";
  }
  std::cout << "Compiled date: " << __DATE__ << " " << __TIME__ << "\n"
               "Version: " << kVersion
            << std::endl;
}

static std::string
removeDirectoryPart(const std::string& filepath) NOEXCEPT
{
  std::string::size_type pos = filepath.find_last_of('/');
  return pos == std::string::npos ? filepath : filepath.substr(pos + 1);
}


static std::string
removeSuffix(const std::string& filename) NOEXCEPT
{
  std::string::size_type pos = filename.find_last_of('.');
  return pos == std::string::npos ? (filename + ".") : filename.substr(0, pos);
}
