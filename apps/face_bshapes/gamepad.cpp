#include "gamepad.h"
#include "common.h"
#include "math.h"
#include <algorithm>
#include <assert>

namespace tyro 
{
const std::map<std::string, std::vector<std::string>> UPPER_MAP
= 
{
    //{,  {"faceMuscles.labSup_AN"}},
    //{,  {"faceMuscles.labSup"}},
    {"L_S_Y",  {"faceMuscles.zygomatic"}},
    {"R_S_X",  {"faceMuscles.buccinator"}},
    //{,  {"faceMuscles.triangularis"}},
    {"R_S_Y",  {"faceMuscles.mentalis"}},
    //{,  {"faceMuscles.risoriusPlatysma"}},
    //{,  {"faceMuscles.incisivus"}},
    //{,  {"faceMuscles.orbOris_loose_lo", "faceMuscles.orbOris_loose_hi"}},
    {"L_S_X",  {"jaw.rotateZ"}}
};

const std::map<std::string, std::vector<std::string>> MIDJAW_MAP
= 
{
    //{,  {"faceMuscles.labSup_AN"}},
    //{,  {"faceMuscles.labSup"}},
    {"L_S_Y",  {"faceMuscles.zygomatic"}},
    {"R_S_X",  {"faceMuscles.buccinator"}},
    //{,  {"faceMuscles.triangularis"}},
    {"R_S_Y",  {"faceMuscles.mentalis"}},
    //{,  {"faceMuscles.risoriusPlatysma"}},
    //{,  {"faceMuscles.incisivus"}},
    //{,  {"faceMuscles.orbOris_loose_lo", "faceMuscles.orbOris_loose_hi"}},
    {"L_S_X",  {"jaw.rotateZ"}}
};

const std::map<std::string, std::vector<std::string>> LIP_MAP
= 
{
    //{,  {"faceMuscles.labSup_AN"}},
    //{,  {"faceMuscles.labSup"}},
    {"L_S_Y",  {"faceMuscles.zygomatic"}},
    {"R_S_X",  {"faceMuscles.buccinator"}},
    //{,  {"faceMuscles.triangularis"}},
    {"R_S_Y",  {"faceMuscles.mentalis"}},
    //{,  {"faceMuscles.risoriusPlatysma"}},
    //{,  {"faceMuscles.incisivus"}},
    //{,  {"faceMuscles.orbOris_loose_lo", "faceMuscles.orbOris_loose_hi"}},
    {"L_S_X",  {"jaw.rotateZ"}}
};

Gamepad::Gamepad() 
{
    for (auto i : upper_face_bshape_index)
    {
        lower_bnames.push_back(ALL_BSHAPES[i]);
        lower_values.push_back(0.0);
    }

    for (auto i : lower_face_bshape_index)
    {
        lower_bnames.push_back(ALL_BSHAPES[i]);
        lower_values.push_back(0.0);
    }
}

void Gamepad::UpdateFrame(std::map<std::string, float>& axes, std::map<std::string, bool>& buttons_map) 
{
    if (buttons_map["triangle"]) 
    {
        face_part = FacePart::UPPER;
    }
    else if (buttons_map["square"]) 
    {
        face_part = FacePart::MID_JAW;
    }
    else if (buttons_map["circle"]) 
    {
        face_part = FacePart::LIPS;
    }
    else if (buttons_map["R1"]) 
    {
        //save current face
    }

    auto compute = [&] (std::map<std::string, std::vector<std::string>>& PART_MAP,
                        std::vector<std::string>& bnames,
                        std::vector<float>& values)
    {
        for (auto const& a : PART_MAP) 
        {   
            auto value = fabs(axes[a.first]);
            for (auto& au : a.second) 
            {   
                auto it = std::find(bnames.begin(), bnames.end(), au);
                assert(it != bnames.end());
                int index = std::distance(bnames.begin(), it);
                values[index] = value;
            }
        } 
    };

    if (face_part == FacePart::UPPER) 
    {
       compute(UPPER_MAP, upper_bnames, upper_values)
    }
    else if (face_part == FacePart::MID_JAW)  
    {
        compute(MIDJAW_MAP, lower_bnames, lower_values)
    }   
}

void Gamepad::isLower()
{   
    bool isLower = face_part == FacePart::UPPER? true:false;
    return isLower;
}

    
}