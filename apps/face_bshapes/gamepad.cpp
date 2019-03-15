#include "gamepad.h"
#include "math.h"
#include <algorithm>
#include <assert.h>
#include "common.h"
#include "RALogManager.h"

namespace tyro 
{
const std::vector<std::string> UPPER_STUFF = {
           "faceMuscles.frontalis",
           "faceMuscles.orbOculi",
           "faceMuscles.currogator",
           "blink_ctl_pos.translateY_neg",                      
           "blink_ctl_neg.translateY_pos",                     
           "loBlink_ctl_pos.translateY_pos",                  
           "loBlink_ctl_neg.translateY_neg"};

const std::map<std::string, std::vector<std::string>> UPPER_MAP
= 
{
    {"L_T",          {"faceMuscles.frontalis"}},
    {"L_S_Y_UP",     {"faceMuscles.orbOculi"}},
    {"L_S_X_LEFT",   {"faceMuscles.currogator"}},
   
    {"R_S_X_LEFT",   {"blink_ctl_pos.translateY_neg"}},                     
    {"R_S_X_RIGHT",  {"blink_ctl_neg.translateY_pos"}},                     
    {"R_S_Y_UP",     {"loBlink_ctl_pos.translateY_pos"}},                  
    {"R_S_Y_DOWN",   {"loBlink_ctl_neg.translateY_neg" }}                 

    //todo: add eyelids bshapes
    //{"R_S_X",  {"faceMuscles.buccinator"}},
    //{,  {"faceMuscles.triangularis"}},
    //{"R_S_Y",  {"faceMuscles.mentalis"}},
    //{,  {"faceMuscles.risoriusPlatysma"}},
    //{,  {"faceMuscles.incisivus"}},
    //{,  {"faceMuscles.orbOris_loose_lo", "faceMuscles.orbOris_loose_hi"}},
};

const std::vector<std::string> LOWER_STUFF = {
 "jaw.rotateZ",
 "jaw.translateX",
 "faceMuscles.zygomatic",
 "faceMuscles.labSup",
 "faceMuscles.labSup_AN",
 "faceMuscles.orbOris_loose_lo",
 "faceMuscles.risoriusPlatysma",
 "faceMuscles.mentalis",
 "faceMuscles.triangularis",
 "faceMuscles.LabInf",
 "faceMuscles.incisivus"};

const std::map<std::string, std::vector<std::string>> MIDJAW_MAP
= 
{
    {"L_T",          {"jaw.rotateZ"}},
    {"L_S_Y_UP",     {"jaw.translateX"}},
    //{"L_S_X_LEFT",   {"jaw.rotateY"}},  
    //{"L_S_X_RIGHT",  {"jaw.rotateY"}},  

    {"R_T",          {"faceMuscles.zygomatic"}},
    {"R_S_X_RIGHT",  {"faceMuscles.labSup"}},
    {"R_S_Y_UP",     {"faceMuscles.labSup_AN"}},
};

const std::map<std::string, std::vector<std::string>> LIP_MAP
= 
{
    {"R_T",         {"faceMuscles.orbOris_loose_lo"}},
    {"R_S_X_LEFT",  {"faceMuscles.risoriusPlatysma"}},
    {"R_S_Y_UP",    {"faceMuscles.mentalis"}},

    {"L_T",         {"faceMuscles.triangularis"}},
    {"L_S_X_LEFT",  {"faceMuscles.LabInf"}},
    {"L_S_Y_UP",    {"faceMuscles.incisivus"}}
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

    for (auto& a : LOWER_STUFF)
    {
        low_bnames.push_back(a);
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