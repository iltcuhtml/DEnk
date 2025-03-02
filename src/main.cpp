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
    std::optional<NodeDox> tree = parser.parse();

    if (!tree.has_value())
    {
        std::cerr << "No 'dox' Statement Found" << std::endl;

        return EXIT_FAILURE;
    }

    Generator generator(tree.value());

    {
        system("mkdir out");

        std::fstream file("out/out.asm", std::ios::out);
        file << generator.generate();
    }

    system("nasm -f win64 out/out.asm -o out/out.o");
    system("gcc out/out.o -o out/out");

    return EXIT_SUCCESS;
}