#include "Scribe.h"
#include <fstream>
#include <iostream>


void cache(const std::string filePath, const std::string data){

    std::ofstream file(filePath);    
    if (!file.is_open()) std::cout << "Cannot open file: " << filePath << std::endl;   
    file << data; 
    if (file.fail()) std::cout << "Failed to write to file: " << filePath << std::endl;  
    
}