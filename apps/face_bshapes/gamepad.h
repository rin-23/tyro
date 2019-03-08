#pragma once

#include "TyroWindow.h"
#include <vector>
#include <map>

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

    void UpdateFrame(std::map<std::string, float> axes, std::map<std::string, bool> buttons_map);
    
    FacePart face_part;
    
    std::vector<std::string> lower_bnames;
    std::vector<float> lower_values;

    std::vector<std::string> upper_bnames;
    std::vector<float> upper_values;
};
}