#pragma once

#include <vector>
#include <map>
#include <string>

namespace tyro 
{
class Sliders
{
public:
    typedef enum 
    {
        UPPER,
        LOWER,
        MID_JAW,
        LIPS,
        NONE_PART
    } FacePart;
    
    Sliders();
    ~Sliders() {}

    void Init(); 

    void UpdateFrame(std::map<std::string, double>& axes);
    
    FacePart face_part;

    std::vector<std::string> low_bnames;
    std::vector<double> low_values;

    std::vector<std::string> up_bnames;
    std::vector<double> up_values;
};
}