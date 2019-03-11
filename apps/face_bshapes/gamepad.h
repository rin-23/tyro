#pragma once

#include "TyroWindow.h"
#include <vector>
#include <map>
#include <string>

namespace tyro 
{
class Gamepad 
{
public:
    typedef enum 
    {
        UPPER,
        MID_JAW,
        LIPS
    } FacePart;
    
    Gamepad();
    ~Gamepad() {}

    void Init(); 

    void UpdateFrame(std::map<std::string, double>& axes, 
                     std::map<std::string, bool>& buttons_map);
    
    bool isLower();

    FacePart face_part;
    
    std::vector<std::string> low_bnames;
    std::vector<double> low_values;

    std::vector<std::string> up_bnames;
    std::vector<double> up_values;
};
}