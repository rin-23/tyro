#include "gamepad.h"
#include "math.h"
#include <algorithm>
#include <assert.h>
#include "common.h"
#include "RALogManager.h"

namespace tyro 
{
const std::map<std::string, std::vector<std::string>> UPPER_MAP
= 
{
    {"L_T",         {"faceMuscles.frontalis"}},
    {"L_S_Y_UP",    {"faceMuscles.orbOculi"}},
    {"L_S_X_LEFT",  {"faceMuscles.currogator"}}
    //todo: add eyelids bshapes

    //{"R_S_X",  {"faceMuscles.buccinator"}},
    //{,  {"faceMuscles.triangularis"}},
    //{"R_S_Y",  {"faceMuscles.mentalis"}},
    //{,  {"faceMuscles.risoriusPlatysma"}},
    //{,  {"faceMuscles.incisivus"}},
    //{,  {"faceMuscles.orbOris_loose_lo", "faceMuscles.orbOris_loose_hi"}},
};

const std::map<std::string, std::vector<std::string>> MIDJAW_MAP
= 
{
    {"L_T",          {"jaw.rotateZ"}},
    {"R_T",          {"faceMuscles.zygomatic"}},

    {"L_S_Y_UP",     {"jaw.translateX"}},
//    {"L_S_X",        {"faceMuscles.zygomatic"}}, jaw left right 

    {"R_S_X_RIGHT",  {"faceMuscles.labSup"}},
    {"R_S_Y_UP",     {"faceMuscles.labSup_AN"}},
};

const std::map<std::string, std::vector<std::string>> LIP_MAP
= 
{
    {"L_T",         {"faceMuscles.triangularis"}},
    {"R_T",         {"faceMuscles.blow"}},
    
    {"R_S_X_LEFT",  {"faceMuscles.risoriusPlatysma"}},
    {"R_S_Y_UP",    {"faceMuscles.mentalis"}},
    
    {"L_S_X_LEFT",  {"faceMuscles.incisivus"}},
    {"L_S_Y_UP",    {"faceMuscles.LabInf"}}
};

Gamepad::Gamepad()
:
face_part(FacePart::NONE_PART),
mState(State::NONE_STATE) 
{}

void Gamepad::Init() 
{   
    for (const auto& i : upper_face_bshape_index)
    {
        up_bnames.push_back(ALL_BSHAPES[i]);
        up_values.push_back(0.0);
    }

    for (const auto& i : lower_face_bshape_index)
    {
        low_bnames.push_back(ALL_BSHAPES[i]);
        low_values.push_back(0.0);
    }
}

void Gamepad::UpdateFrame(std::map<std::string, double>& axes, 
                          std::map<std::string, bool>&   buttons_map) 
{   
    RA_LOG_INFO("Current state %i", mState);
    RA_LOG_INFO("Current part %i", face_part);
    if (buttons_map["triangle"]) 
    {   
        RA_LOG_INFO("switched to upper");
        face_part = FacePart::UPPER;
        mState = State::MOD_UPPER;
    }
    else if (buttons_map["square"]) 
    {
        RA_LOG_INFO("switched to midjaw");
        face_part = FacePart::MID_JAW;
        mState = State::MOD_MID;
    }
    else if (buttons_map["circle"]) 
    {
        RA_LOG_INFO("switched to lips");
        face_part = FacePart::LIPS;
        mState = State::MOD_LIPS;
    }
    else if (buttons_map["R1"]) // save current face
    {           
        mState = State::SAVED;
    }

    auto computeValues = [&] (const std::map<std::string, std::vector<std::string>>& PART_MAP,
                              std::vector<std::string>& bnames,
                              std::vector<double>& values)
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

    if (mState == State::SAVED || 
        mState == State::NONE_STATE) 
    {

    }
    else if (mState == State::MOD_LIPS ||
             mState == State::MOD_UPPER ||
             mState == State::MOD_MID) 
    {
        if (face_part == FacePart::UPPER) 
        {
            computeValues(UPPER_MAP, up_bnames, up_values);
        }
        else if (face_part == FacePart::MID_JAW)  
        {
            computeValues(MIDJAW_MAP, low_bnames, low_values);
        } 
        else if (face_part == FacePart::LIPS)  
        {
            computeValues(LIP_MAP, low_bnames, low_values);
        }
    }
      
}

bool Gamepad::isLower()
{   
    bool isLower = face_part == FacePart::UPPER ? true : false;
    return isLower;
}

    
}