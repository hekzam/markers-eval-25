#include <iostream>
#include <cstdlib>
#include <string.h>





int main(int argc, char const* argv[])
{
    size_t pos;
    for(int i=1; i<argc; i++)
    {

    // Rechercher la position du caractère '=' dans la chaîne
    std::string arg(argv[i]);  // Conversion de char* en std::string

    pos = arg.find("-seed");

    }
    std::cout<<pos<<std::endl;


    return 0;
}