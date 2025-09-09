
#include <string>


class Scribe
{
    public:

        void cache(const std::string file_path, const std::string data); //working files that store command outputs
        void save(); //saves working cache to file system with date to be used later

    private:

        

};