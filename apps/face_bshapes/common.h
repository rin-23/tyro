#pragma once
 
#include <vector>
#include <string>
#include <map>
#include "animlist.h"

#define USE_ALL_BSHAPES 0

namespace tyro
{

const std::string NEUT = "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/neuteral.obj";

const std::map<std::string, std::string> BSHAPES_MAP = 
{
   { "faceMuscles.frontalis",           "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/frontalis.obj" },             
   { "faceMuscles.currogator",          "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/currogator.obj" },
   { "faceMuscles.medFrontalis",        "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/medFrontalis.obj"},
   { "faceMuscles.orbOculi",            "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOculi.obj"},
   { "faceMuscles.orbOculi_L",          "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOculi_L.obj"},
   { "faceMuscles.orbOculi_R",          "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOculi_R.obj"},
   { "faceMuscles.orbOculi_out",        "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOculi_out.obj"},
   { "faceMuscles.latFrontalis",        "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/latFrontalis.obj"},
   { "faceMuscles.latFrontalis_L",      "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/latFrontalis_L.obj"},
   { "faceMuscles.latFrontalis_R",      "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/latFrontalis_R.obj"},
   { "faceMuscles.LabInf",              "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/LabInf.obj"},
   { "faceMuscles.zygomatic",           "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/zygomatic.obj" },
   { "faceMuscles.labSup",              "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/labSup.obj"},
   { "faceMuscles.labSup_AN",           "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/labSup_AN.obj"},
   { "faceMuscles.triangularis",        "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/triangularis.obj"},
   { "faceMuscles.incisivus",           "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/incisivus.obj"},
   { "faceMuscles.mentalis",            "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/mentalis.obj"},
   { "faceMuscles.risoriusPlatysma",    "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/risoriusPlatysma.obj"},
   { "faceMuscles.orbOris_loose_lo",    "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOris_loose_lo.obj"},
   { "faceMuscles.orbOris_loose_hi",    "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOris_loose_hi.obj"},
   { "faceMuscles.orbOris_tight_lo",    "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOris_tight_lo.obj"},
   { "faceMuscles.orbOris_tight_hi",    "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOris_tight_hi.obj"},
   { "faceMuscles.orbOri0s_tight_hi2",  "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOri0s_tight_hi2.obj"},
   { "faceMuscles.orbOris_tight_lo2",   "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOris_tight_lo2.obj"},
   { "faceMuscles.mouthClose",          "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/mouthClose.obj"},
   { "faceMuscles.orbOculi_lo",         "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOculi_lo.obj" },
   { "faceMuscles.buccinator",          "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/buccinator.obj" },
   { "faceMuscles.orbOculi_lo_L",       "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOculi_lo_L.obj" },
   { "faceMuscles.orbOculi_lo_R",       "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOculi_lo_R.obj" },
   { "faceMuscles.labSup_L",            "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/labSup_L.obj" },
   { "faceMuscles.labSup_R",            "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/labSup_R.obj" },
   { "faceMuscles.zygomatic_L",         "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/zygomatic_L.obj" },
   { "faceMuscles.zygomatic_R",         "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/zygomatic_R.obj" },
   { "faceMuscles.risoriusPlatysma_L",  "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/risoriusPlatysma_L.obj" },
   { "faceMuscles.risoriusPlatysma_R",  "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/risoriusPlatysma_R.obj" },
   { "faceMuscles.levAnguliOris",       "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/levAnguliOris.obj" },
   { "faceMuscles.dilatorNaris",        "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/dilatorNaris.obj" },
   { "faceMuscles.Zyg_Minor",           "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/Zyg_Minor.obj" },
   { "faceMuscles.mentalis_lowerLip",   "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/mentalis_lowerLip.obj" },
   { "faceMuscles.triangularis_L",      "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/triangularis_L.obj" },
   { "faceMuscles.triangularis_R",      "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/triangularis_R.obj" },
   { "faceMuscles.orbOris_up_hi",       "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOris_up_hi.obj" },
   { "faceMuscles.jawOpenComp",         "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/jawOpenComp.obj" },
   { "faceMuscles.blow",                "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/blow.obj"},
   { "jaw.rotateZ",                     "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/jawUpDn.obj"},
    //"jaw.rotateY"
   { "jaw.translateX",                  "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/jawInOut.obj"},
   { "blink_ctl.translateY_pos",        "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/topeyelids_up.obj"},
   { "blink_ctl.translateY_neg",        "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/topeyelids_down.obj"},
   { "loBlink_ctl.translateY_pos",      "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/btmeyelids_up.obj"},
   { "loBlink_ctl.translateY_neg",      "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/btmeyelids_down.obj"}
};

const std::vector<std::string> EYES = 
{
    "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/eye_white.obj",
    "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/eye_brown.obj",
    "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/eye_black.obj",
    "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/eye_left_white.obj",
    "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/eye_left_brown.obj",
    "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/eye_left_black.obj"
};

const std::vector<std::string> ALL_BSHAPES = 
{
    "faceMuscles.frontalis",                        //0
    "faceMuscles.currogator",                       //1
    "faceMuscles.medFrontalis",                     //2
    "faceMuscles.orbOculi",                         //3
    "faceMuscles.orbOculi_L",                       //4
    "faceMuscles.orbOculi_R",                       //5
    "faceMuscles.orbOculi_out",                     //6
    "faceMuscles.latFrontalis",                     //7
    "faceMuscles.latFrontalis_L",                   //8
    "faceMuscles.latFrontalis_R",                   //9
    "faceMuscles.LabInf",                           //10
    "faceMuscles.zygomatic",                        //11
    "faceMuscles.labSup",                           //12
    "faceMuscles.labSup_AN",                        //13
    "faceMuscles.triangularis",                     //14
    "faceMuscles.incisivus",                        //15
    "faceMuscles.mentalis",                         //16
    "faceMuscles.risoriusPlatysma",                 //17
    "faceMuscles.orbOris_loose_lo",                 //18
    "faceMuscles.orbOris_loose_hi",                 //19
    "faceMuscles.orbOris_tight_lo",                 //20
    "faceMuscles.orbOris_tight_hi",                 //21
    "faceMuscles.orbOri0s_tight_hi2",               //22
    "faceMuscles.orbOris_tight_lo2",                //23
    "faceMuscles.mouthClose",                       //24
    "faceMuscles.orbOculi_lo",                      //25
    "faceMuscles.buccinator",                       //26
    "faceMuscles.orbOculi_lo_L",                    //27
    "faceMuscles.orbOculi_lo_R",                    //28
    "faceMuscles.labSup_L",                         //29
    "faceMuscles.labSup_R",                         //30
    "faceMuscles.zygomatic_L",                      //31
    "faceMuscles.zygomatic_R",                      //32
    "faceMuscles.risoriusPlatysma_L",               //33
    "faceMuscles.risoriusPlatysma_R",               //34
    "faceMuscles.levAnguliOris",                    //35
    "faceMuscles.dilatorNaris",                     //36
    "faceMuscles.Zyg_Minor",                        //37
    "faceMuscles.mentalis_lowerLip",                //38
    "faceMuscles.triangularis_L",                   //39
    "faceMuscles.triangularis_R",                   //40
    "faceMuscles.orbOris_up_hi",                    //41
    "faceMuscles.jawOpenComp",                      //42
    "faceMuscles.blow",                             //43
    "jaw.rotateZ",                                  //44
    //"jaw.rotateY"                                 //
    "jaw.translateX",                               //45
    //"blink_ctl.translateY",                       //
    //"loBlink_ctl.translateY"                      //
    "blink_ctl.translateY_pos",                     //46 
    "blink_ctl.translateY_neg",                     //47
    "loBlink_ctl.translateY_pos",                   //48
    "loBlink_ctl.translateY_neg"                    //49
};

//remove index 46 to get rid of "jaw.rotateY"
const std::vector<int> lower_face_bshape_index = {10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,26,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45}; 
const std::vector<int> upper_face_bshape_index = {0,1,2,3,4,5,6,7,8,9,25,27,28,46,47,48,49}; 

  
}