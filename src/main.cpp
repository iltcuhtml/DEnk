#include <iostream>

#include <sstream>
#include <vector>
#include <optional>
#include <fstream>
#include <filesystem>

#include "tokenizer.hpp"
#include "parser.hpp"
#include "generator.hpp"

int main(int argc, char* argv[])
{
    system("chcp 65001 > nul");

    if (argc != 2)
    {
        std::cerr << "Fehler: Eine DEnk-Datei (*.DEnk) wird benötigt" << std::endl;

        return EXIT_FAILURE;
    }

    const std::string filename = argv[1];

    std::ifstream input(filename, std::ios::binary);
    
    if (!input)
    {
        std::cerr << "Fehler: Die Datei konnte nicht geöffnet werden" << std::endl;
        
        return EXIT_FAILURE;
    }

    std::string contents(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>()
    );

    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();

    Parser parser(std::move(tokens));
    std::optional<NodeProg> prog = parser.parse_prog();

    if (!prog.has_value())
    {
        std::cerr << "Fehler: Ungültiges Programm" << std::endl;

        return EXIT_FAILURE;
    }

    std::error_code ec;

    if (!std::filesystem::exists("out") && !std::filesystem::create_directory("out", ec))
    {
        std::cerr << "Fehler: Ausgabeverzeichnis konnte nicht erstellt werden: " << ec.message() << std::endl;
        
        return EXIT_FAILURE;
    }

    Generator generator(prog.value());

    std::ofstream file("out/out.asm", std::ios::out | std::ios::binary);
    
    if (!file)
    {
        std::cerr << "Fehler: Die Ausgabedatei konnte nicht erstellt werden" << std::endl;
        
        return EXIT_FAILURE;
    }

    file << generator.gen_prog();

    int ret = system("nasm -f win64 out/out.asm -o out/out.o");
    
    if (ret != 0)
    {
        std::cerr << "Fehler: NASM-Assembler konnte nicht erfolgreich ausgeführt werden" << std::endl;
        
        return EXIT_FAILURE;
    }

    ret = system("gcc out/out.o -o out/out");
    
    if (ret != 0)
    {
        std::cerr << "Fehler: GCC-Linker konnte nicht erfolgreich ausgeführt werden" << std::endl;
        
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}