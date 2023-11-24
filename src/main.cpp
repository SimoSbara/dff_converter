#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>

#include <renderware.h>


#include "ConverterGLTF.h"
#include "util.h"


int main()
{
    ConverterGLTF converter;
    std::string inpDir, outDir, input, output;
    std::string ext = ".dff";

    std::cout << "Input Folder ('.' for current folder): ";
    std::cin >> inpDir;

    std::cout << "Output Folder ('.' for current folder): ";
    std::cin >> outDir;

    std::filesystem::path currentDir = std::filesystem::current_path();

    try
    {
        for (auto& p : std::filesystem::recursive_directory_iterator(inpDir))
        {
            std::string file = p.path().generic_string();

            if (p.path().extension().generic_string() == ext)
            {
                int basePath = p.path().parent_path().generic_string().length();
                int fileLength = file.length();

                std::string baseFile = file.substr(basePath, fileLength - basePath - 4);

                std::string dff = inpDir + baseFile + ".dff";
                std::string txd = inpDir + baseFile + ".txd";

                output = outDir + baseFile + ".gltf";

                if(converter.convert(output, dff, txd))
                    std::cout << baseFile << " converted" << std::endl;
                else
                    std::cout << baseFile << " not converted!" << std::endl;
            }
        }
    }
    catch (std::exception e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}