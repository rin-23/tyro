#include "cameracontrol.h"
#include "math.h"
#include <algorithm>
#include <assert.h>
#include "common.h"
#include "RALogManager.h"
#include <stdio.h>
#include <functional>
#include <iostream>
#include <stdio.h>
#include "common.h"

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


const std::map<std::string, std::vector<std::string>> OPENFACE_TO_BSHAPES_MAP = 
{
    {"AU01", {"faceMuscles.medFrontalis"}},
    {"AU02", {"faceMuscles.frontalis"}},
    {"AU04", {"faceMuscles.currogator"}},
    {"AU05", {"blink_ctl.translateY_pos"}},
    {"AU06", {"faceMuscles.orbOculi"}},
    //{"AU07_r",
    {"AU09", {"faceMuscles.labSup_AN"}},
    {"AU10", {"faceMuscles.labSup"}},
    {"AU12", {"faceMuscles.zygomatic"}},
    {"AU14", {"faceMuscles.buccinator"}},
    {"AU15", {"faceMuscles.triangularis"}},
    {"AU17", {"faceMuscles.mentalis"}},
    {"AU20", {"faceMuscles.risoriusPlatysma"}},
    {"AU23", {"faceMuscles.incisivus"}},
    {"AU25", {"faceMuscles.orbOris_loose_lo", "faceMuscles.orbOris_loose_hi"}},
    {"AU26", {"jaw.rotateZ"}}
    //{"AU45_r",
};

const std::map<std::string, std::vector<std::string>> OPENFACE_TO_BSHAPES_MAP_R = 
{
    {"AU01_r",  {"faceMuscles.medFrontalis"}},
    {"AU02_r",  {"faceMuscles.frontalis"}},
    {"AU04_r",  {"faceMuscles.currogator"}},
    {"AU05_r",  {"blink_ctl.translateY_pos"}},
    {"AU06_r",  {"faceMuscles.orbOculi"}},
    //{"AU07_r",
    {"AU09_r",  {"faceMuscles.labSup_AN"}},
    {"AU10_r",  {"faceMuscles.labSup"}},
    {"AU12_r",  {"faceMuscles.zygomatic"}},
    {"AU14_r",  {"faceMuscles.buccinator"}},
    {"AU15_r",  {"faceMuscles.triangularis"}},
    {"AU17_r",  {"faceMuscles.mentalis"}},
    {"AU20_r",  {"faceMuscles.risoriusPlatysma"}},
    {"AU23_r",  {"faceMuscles.incisivus"}},
    {"AU25_r",  {"faceMuscles.orbOris_loose_lo", "faceMuscles.orbOris_loose_hi"}},
    {"AU26_r",  {"jaw.rotateZ"}}
    //{"AU45_r",
};

CameraControl::CameraControl()
:
face_part(FacePart::NONE_PART)
{}

void CameraControl::Init() 
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

void CameraControl::Update(std::vector<std::string>& names, std::vector<double>& values, bool fromcsv ) 
{   
    //iterate over available values for OpenFace
    for (int i =0;i<names.size(); ++i) 
    {
        //RA_LOG_INFO("Updating\n %s %f", names[i].c_str(), values[i]);
    } 

    for (int i=0; i<names.size();++i) 
    {
        auto k = names[i];
        if (OPENFACE_TO_BSHAPES_MAP_R.count(k) || OPENFACE_TO_BSHAPES_MAP.count(k)) //check if we support those attributes 
        {
            auto v = fromcsv ? OPENFACE_TO_BSHAPES_MAP_R.at(k) : OPENFACE_TO_BSHAPES_MAP.at(k);
            
            // iterate though the list of mapped values, usually one but sometimes 2
            for (auto& au : v) 
            {
                // get and scale the value
                double scaled = values[i]/5.0;
                if (scaled <= 0.1) scaled = 0.0; //reject noise
                assert(scaled <= 1.0 && scaled >=0);

                // construct lower values vector to feed to NN
                auto it = std::find(low_bnames.begin(), low_bnames.end(), au);
                if(it != low_bnames.end()) 
                {
                    int index = std::distance(low_bnames.begin(), it);
                    low_values[index] = scaled;
                }
                
                auto it2 = std::find(up_bnames.begin(), up_bnames.end(), au);
                if(it2 != up_bnames.end()) 
                {
                    int index = std::distance(up_bnames.begin(), it2);
                    up_values[index] = scaled;
                }

                assert(it != low_bnames.end() || it2 != up_bnames.end());                
            }
        }
    }
}

void CameraControl::Print()
{   
    RA_LOG_INFO("-------------------");
    for (int i=0;i<low_values.size();++i) 
    {
        RA_LOG_INFO("%s %f", low_bnames[i].c_str(), low_values[i]);
    }
    for (int i=0;i<up_values.size();++i) 
    {
        RA_LOG_INFO("%s %f", up_bnames[i].c_str(), up_values[i]);
    }
}   

}