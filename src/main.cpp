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
    if (argc != 2)
    {
        std::cerr << "Add Jschlang File (*.jschlang)" << std::endl;

        return EXIT_FAILURE;
    }

    std::string contents;

    {
        std::stringstream contents_stream;

        std::fstream input(argv[1], std::ios::in);
        contents_stream << input.rdbuf();
        
        contents = contents_stream.str();
    }

    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();

    Parser parser(std::move(tokens));
    std::optional<NodeProg> prog = parser.parse_prog();

    if (!prog.has_value())
    {
        std::cerr << "Invalid Program" << std::endl;

        return EXIT_FAILURE;
    }

    Generator generator(prog.value());

    {
        system("mkdir out");

        std::fstream file("out/out.asm", std::ios::out);
        file << generator.gen_prog();
    }

    system("nasm -f win64 out/out.asm -o out/out.o");
    system("gcc out/out.o -o out/out");

    return EXIT_SUCCESS;
}