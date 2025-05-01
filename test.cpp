#include <iostream>
#include <cstdlib>
#include <string.h>
#include <random>
#include <opencv2/core.hpp>  





int main(int argc, char const* argv[])
{
    size_t pos;
    for(int i=1; i<argc; i++)
    {

    // Rechercher la position du caractère '=' dans la chaîne
    std::string arg(argv[i]);  // Conversion de char* en std::string

    pos = arg.find("-seed");

    }
    int seed=4;
    

    cv::RNG rng = cv::RNG(seed);
    int max_pepper = rng.uniform(1,5);                      
    //rng.next();
    int max_salt = rng.uniform(1,5);

    std::cout<<"maxperpper = "<<max_pepper<<"  max_salt ="<<max_salt<<std::endl;


    return 0;
}