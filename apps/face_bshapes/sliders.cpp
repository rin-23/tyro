#include "sliders.h"
#include "math.h"
#include <algorithm>
#include <assert.h>
#include "common.h"
#include "RALogManager.h"

namespace tyro 
{
const std::vector<std::string> UPPER_STUFF = {
           "faceMuscles.frontalis",
           "faceMuscles.medFrontalis",
           "faceMuscles.orbOculi",
           "faceMuscles.orbOculi_lo",
           "faceMuscles.currogator",
           "blink_ctl.translateY_neg",
           "blink_ctl.translateY_pos",
           "loBlink_ctl.translateY_pos",
           "loBlink_ctl.translateY_neg"};

const std::vector<std::string> LOWER_STUFF = {
            "jaw.rotateZ",
            "jaw.translateX",
            "faceMuscles.zygomatic",
            "faceMuscles.labSup",
            "faceMuscles.labSup_AN",
            "faceMuscles.orbOris_loose_lo",
            "faceMuscles.orbOris_loose_hi",
            "faceMuscles.mouthClose",
            "faceMuscles.risoriusPlatysma",
            "faceMuscles.mentalis",
            "faceMuscles.triangularis",
            "faceMuscles.LabInf",
            "faceMuscles.incisivus",
            "faceMuscles.buccinator"};

            

Sliders::Sliders()
:
face_part(FacePart::NONE_PART)
{}

void Sliders::Init() 
{       
#if USE_ALL_BSHAPES   
    for (int i : upper_face_bshape_index)
    {
        up_bnames.push_back(ALL_BSHAPES[i]);
        up_values.push_back(0.0);
    }

    for (int i : lower_face_bshape_index)
    {
        low_bnames.push_back(ALL_BSHAPES[i]);
        low_values.push_back(0.0);
    }    
#else
    for (auto& a : UPPER_STUFF)
    {
        up_bnames.push_back(a);
        up_values.push_back(0.0);
    }

    for (auto& a : LOWER_STUFF)
    {
        low_bnames.push_back(a);
        low_values.push_back(0.0);
    }
#endif    
}

void Sliders::UpdateFrame(std::map<std::string, double>& axes) 
{   
    int i = 0;
    for (auto& s : low_bnames) 
    {   
        low_values[i++] = axes[s]; 
    }
    
    i = 0;
    for (auto& s : up_bnames) 
    {   
        up_values[i++] = axes[s]; 
    }
}    
}