#include <iostream>

#include <cstdint>
#include <sstream>
#include <vector>

#include <optional>

#include <fstream>

#include "tokenization.hpp"
#include "parser.hpp"
#include "generation.hpp"

int main(int argc, char* argv[])
{
    system("chcp 65001 > nul");

    if (argc != 2)
    {
        std::cerr << "Fehler: Eine DEnk-Datei (*.DEnk) wird benötigt" << std::endl;

        return EXIT_FAILURE;
    }

    std::string contents;

    {
        std::ifstream input(argv[1], std::ios::binary);

        if (!input.is_open())
        {
            std::cerr << "Fehler: Die Datei konnte nicht geöffnet werden" << std::endl;
            return EXIT_FAILURE;
        }

        std::ostringstream contents_stream;
        contents_stream << input.rdbuf();
        
        contents = contents_stream.str();
    }

    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();

    Parser parser(std::move(tokens));
    std::optional<NodeProg> prog = parser.parse_prog();

    if (!prog.has_value())
    {
        std::cerr << "Fehler: Ungültiges Programm" << std::endl;

        return EXIT_FAILURE;
    }

    Generator generator(prog.value());

    {
        system("mkdir out > nul");

        std::ofstream file("out/out.asm", std::ios::out | std::ios::binary);

        if (!file)
        {
            std::cerr << "Fehler: Die Ausgabedatei konnte nicht erstellt werden" << std::endl;
            return EXIT_FAILURE;
        }

        file << generator.gen_prog();
    }

    system("nasm -f win64 out/out.asm -o out/out.o");
    system("gcc out/out.o -o out/out");

    return EXIT_SUCCESS;
}