#pragma once
 
#include <vector>
#include <string>

const std::string NEUT = "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/neuteral.obj";

const std::map<std::string, std::string> BSHAPES_MAP = 
{
   {"faceMuscles.frontalis", "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/frontalis.obj" }                            ,             
   {"faceMuscles.currogator", "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/currogator.obj"     },
   { "faceMuscles.medFrontalis", "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/medFrontalis.obj" },
   {"faceMuscles.orbOculi", "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOculi.obj" },
   { "faceMuscles.orbOculi_L","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOculi_L.obj" },
   { "faceMuscles.orbOculi_R","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOculi_R.obj" },
   { "faceMuscles.orbOculi_out","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOculi_out.obj" },
   {"faceMuscles.latFrontalis", "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/latFrontalis.obj" },
   { "faceMuscles.latFrontalis_L", "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/latFrontalis_L.obj" },
   { "faceMuscles.latFrontalis_R","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/latFrontalis_R.obj"},
   { "faceMuscles.LabInf","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/LabInf.obj"},
   { "faceMuscles.zygomatic","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/zygomatic.obj" },
   { "faceMuscles.labSup","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/labSup.obj" },
   { "faceMuscles.labSup_AN","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/labSup_AN.obj" },
   { "faceMuscles.triangularis","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/triangularis.obj" },
   { "faceMuscles.incisivus","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/incisivus.obj" },
   { "faceMuscles.mentalis","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/mentalis.obj" },
   { "faceMuscles.risoriusPlatysma","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/risoriusPlatysma.obj" },
   { "faceMuscles.orbOris_loose_lo","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOris_loose_lo.obj" },
   { "faceMuscles.orbOris_loose_hi","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOris_loose_hi.obj" },
   { "faceMuscles.orbOris_tight_lo","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOris_tight_lo.obj" },
   { "faceMuscles.orbOris_tight_hi","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOris_tight_hi.obj" },
   { "faceMuscles.orbOri0s_tight_hi2","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOri0s_tight_hi2.obj" },
   { "faceMuscles.orbOris_tight_lo2","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOris_tight_lo2.obj" },
   { "faceMuscles.mouthClose", "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/mouthClose.obj" },
   { "faceMuscles.orbOculi_lo","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOculi_lo.obj" },
   { "faceMuscles.buccinator","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/buccinator.obj" },
   { "faceMuscles.orbOculi_lo_L","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOculi_lo_L.obj" },
   { "faceMuscles.orbOculi_lo_R", "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOculi_lo_R.obj" },
   { "faceMuscles.labSup_L", "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/labSup_L.obj" },
   { "faceMuscles.labSup_R", "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/labSup_R.obj" },
   { "faceMuscles.zygomatic_L", "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/zygomatic_L.obj" },
   { "faceMuscles.zygomatic_R", "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/zygomatic_R.obj" },
   { "faceMuscles.risoriusPlatysma_L", "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/risoriusPlatysma_L.obj" },
   { "faceMuscles.risoriusPlatysma_R", "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/risoriusPlatysma_R.obj" },
   { "faceMuscles.levAnguliOris", "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/levAnguliOris.obj" },
   { "faceMuscles.dilatorNaris", "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/dilatorNaris.obj" },
   { "faceMuscles.Zyg_Minor", "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/Zyg_Minor.obj" },
   { "faceMuscles.mentalis_lowerLip","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/mentalis_lowerLip.obj" },
   { "faceMuscles.triangularis_L", "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/triangularis_L.obj" },
   { "faceMuscles.triangularis_R","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/triangularis_R.obj" },
   { "faceMuscles.orbOris_up_hi","/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/orbOris_up_hi.obj" },
   //"faceMuscles.jawOpenComp",
   { "faceMuscles.blow", "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/blow.obj"},
   { "jaw.rotateZ", "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/jawUpDn.obj"},
    //"jaw.rotateY"
   { "jaw.translateX", "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/jawInOut.obj"}
    //"blink_ctl.translateY",
    //"loBlink_ctl.translateY"
};

const std::string RENDER_IMGS_PATH = "/home/rinat/Workspace/FacialManifoldSource/data_anim/images";
const std::string RENDER_VIDEOS_PATH = "/home/rinat/Workspace/FacialManifoldSource/data_anim/videos";

const std::vector<std::string> ANIM_LIST =
{
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/08_0010_02_animation_workshop_0051_Host.txt"
};