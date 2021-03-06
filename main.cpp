#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#if __cplusplus >= 201103L || defined(_MSC_VER) && _MSC_VER >= 1700
#  include <array>
#  include <unordered_map>
#else
#  include <map>
#endif  //  __cplusplus >= 201103L || defined(_MSC_VER) && _MSC_VER >= 1700

#include "ArgumentParser.hpp"
#include "Brainfuck.hpp"
#include "version.h"


#if defined(__cplusplus) && __cplusplus >= 201103 \
  || defined(_MSC_VER) && (_MSC_VER > 1800 || (_MSC_VER == 1800 && _MSC_FULL_VER == 180021114))
#  define NOEXCEPT  noexcept
#else
#  define NOEXCEPT  throw()
#endif


static void
showVersion() NOEXCEPT;

static std::string
getDefaultOutputName(const std::string& inputFile, Brainfuck::Target targetType) NOEXCEPT;

static std::string
getSuffix(Brainfuck::Target targetType) NOEXCEPT;

static std::string
removeDirectoryPart(const std::string& filepath) NOEXCEPT;

static std::string
removeSuffix(const std::string& filename) NOEXCEPT;


int
main(int argc, const char* argv[])
{
#if __cplusplus >= 201103L || defined(_MSC_VER) && _MSC_VER >= 1700
  std::unordered_map<std::string, Brainfuck::Target> targetMap{
    {"c", Brainfuck::Target::kC},
    {"xbyakc", Brainfuck::Target::kXbyakC},
    {"winx86", Brainfuck::Target::kWinX86},
    {"winx64", Brainfuck::Target::kWinX64},
    {"elfx86", Brainfuck::Target::kElfX86},
    {"elfx64", Brainfuck::Target::kElfX64},
    {"elfarmeabi", Brainfuck::Target::kElfArmeabi}
  };
#else
  std::map<std::string, Brainfuck::Target> targetMap;
  targetMap.insert(std::make_pair("c", Brainfuck::Target::kC));
  targetMap.insert(std::make_pair("xbyakc", Brainfuck::Target::kXbyakC));
  targetMap.insert(std::make_pair("winx86", Brainfuck::Target::kWinX86));
  targetMap.insert(std::make_pair("winx64", Brainfuck::Target::kWinX64));
  targetMap.insert(std::make_pair("elfx86", Brainfuck::Target::kElfX86));
  targetMap.insert(std::make_pair("elfx64", Brainfuck::Target::kElfX64));
  targetMap.insert(std::make_pair("elfarmeabi", Brainfuck::Target::kElfArmeabi));
#endif  // __cplusplus >= 201103L || defined(_MSC_VER) && _MSC_VER >= 1700

  try {
    ArgumentParser ap(argv[0]);
    ap.add('e', "eval",  ArgumentParser::OptionType::kRequiredArgument,
        "Execute specified brainfuck source", "SRC", "");
    ap.add('h', "help", "Show help and exit this program");
    ap.add('m', "minify", "Remove all non-brainfuck characters from source code");
    ap.add('o', "output",  ArgumentParser::OptionType::kRequiredArgument,
        "Specify output filename", "FILE", "");
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
    ap.add("top-break-point", "Add break point to the top of code");
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
#if __cplusplus >= 201103L || defined(_MSC_VER) && _MSC_VER >= 1600
      std::cin.tie(nullptr);
#else
      std::ostream* osNullptr = NULL;
      std::cin.tie(osNullptr);
#endif  // __cplusplus >= 201103L || defined(_MSC_VER) && _MSC_VER >= 1600
      std::ios::sync_with_stdio(false);
    }
    std::size_t heapSize = ap.get<std::size_t>("heap-size");
    int optLevel = ap.get<int>("optimize");

    const std::vector<std::string>& args = ap.getArguments();
    const std::string& source = ap.get("eval");
    std::string inputFile = "a.b";

    Brainfuck bf;
    if (source != "") {
      bf.loadSource(source);
    } else if (args.size() > 0) {
      if (args[0] == "-") {
        bf.load(std::cin);
      } else {
        bf.load(args[0]);
        inputFile = args[0];
      }
    } else {
      std::cerr << "Please specify one brainfuck source code" << std::endl;
      return EXIT_FAILURE;
    }
    bf.trim();

    if (ap.get<bool>("minify")) {
      std::cout << bf.getSource() << std::endl;
      return EXIT_SUCCESS;
    }

    bool hasTopBreakPoint = ap.get<bool>("top-break-point");

    if (ap.get<bool>("dump-ir")) {
      bf.compile(Brainfuck::CompileType::kIR, hasTopBreakPoint);
      bf.dumpIR();
      return EXIT_SUCCESS;
    }
    const std::string& target = ap.get("target");
    if (target != "") {
      if (targetMap.find(target) == targetMap.end()) {
        std::cerr << "Option -t, --target: Invalid value: \"" << target << "\" is specified" << std::endl;
        return EXIT_FAILURE;
      }
      bf.compile(Brainfuck::CompileType::kJit, hasTopBreakPoint);
      Brainfuck::Target targetType = targetMap[target];
      std::string outputFile = ap.get("output");
      if (outputFile == "") {
        outputFile = getDefaultOutputName(inputFile, targetType);
      }
      switch (targetType) {
        case Brainfuck::Target::kC:
        case Brainfuck::Target::kXbyakC:
          {
            std::ofstream ofs(outputFile.c_str());
            if (!ofs.is_open()) {
              std::cerr << "Failed to open: " << outputFile << std::endl;
              return EXIT_FAILURE;
            }
            bf.emit(ofs, targetType);
          }
          break;
        case Brainfuck::Target::kWinX86:
        case Brainfuck::Target::kWinX64:
        case Brainfuck::Target::kElfX86:
        case Brainfuck::Target::kElfX64:
        case Brainfuck::Target::kElfArmeabi:
          {
            std::ofstream ofs(outputFile.c_str(), std::ios::binary);
            if (!ofs.is_open()) {
              std::cerr << "Failed to open: " << outputFile << std::endl;
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

    if (optLevel == 1) {
      bf.compile(Brainfuck::CompileType::kIR, hasTopBreakPoint);
    } else if (optLevel > 1) {
      bf.compile(Brainfuck::CompileType::kJit, hasTopBreakPoint);
    }
    bf.execute(heapSize);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  return EXIT_SUCCESS;
}


static void
showVersion() NOEXCEPT
{
  std::cout << "<<< CppBrainfuck >>>\n\n"
               "Compiled by: " << kUsername << "\n"
               "Compiled date: " << __DATE__ << " " << __TIME__ << "\n"
               "Version: " << kVersion
            << std::endl;
}

static std::string
getDefaultOutputName(const std::string& inputFile, Brainfuck::Target targetType) NOEXCEPT
{
  return removeSuffix(removeDirectoryPart(inputFile)) + getSuffix(targetType);
}

static std::string
getSuffix(Brainfuck::Target targetType) NOEXCEPT
{
  switch (targetType) {
    case Brainfuck::Target::kC:
    case Brainfuck::Target::kXbyakC:
      return ".c";
      break;
    case Brainfuck::Target::kWinX86:
    case Brainfuck::Target::kWinX64:
      return ".exe";
    case Brainfuck::Target::kElfX86:
    case Brainfuck::Target::kElfX64:
    case Brainfuck::Target::kElfArmeabi:
      return ".out";
      break;
    default:
      assert(false);
      return "";
  }
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
