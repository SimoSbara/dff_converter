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

                std::string d = inpDir + baseFile + ".dff";
                std::string t = inpDir + baseFile + ".txd";

                std::ifstream dff(inpDir + baseFile + ".dff", std::ios::binary);
                std::ifstream txd(inpDir + baseFile + ".txd", std::ios::binary);

                rw::Clump dffStruct;
                rw::TextureDictionary txdStruct;

                dffStruct.read(dff);
                txdStruct.read(txd);

                output = outDir + baseFile + ".gltf";

                if(converter.convert((char*)output.c_str(), dffStruct, txdStruct))
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