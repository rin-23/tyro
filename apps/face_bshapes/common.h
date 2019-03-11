#pragma once
 
#include <vector>
#include <string>
#include <map>

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
   { "jaw.translateX",                  "/home/rinat/Workspace/tyro/apps/face_bshapes/resources/bshapes/jawInOut.obj"}
    //"blink_ctl.translateY",
    //"loBlink_ctl.translateY"
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
};

//remove index 46 to get rid of "jaw.rotateY"
const std::vector<int> lower_face_bshape_index = {10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,26,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45}; 
const std::vector<int> upper_face_bshape_index = {0,1,2,3,4,5,6,7,8,9,25,27,28}; 

const std::vector<std::string> OpengFaceAUs = 
{
    "faceMuscles.labSup_AN",
    "faceMuscles.labSup",
    "faceMuscles.zygomatic",
    "faceMuscles.buccinator",
    "faceMuscles.triangularis",
    "faceMuscles.mentalis",
    "faceMuscles.risoriusPlatysma",
    "faceMuscles.incisivus",
    "faceMuscles.orbOris_loose_lo", 
    "faceMuscles.orbOris_loose_hi",
    "jaw.rotateZ"
};

const std::map<std::string, std::vector<std::string>> OPENFACE_TO_BSHAPES_MAP = 
{
    //{"AU01_r",
    //{"AU02_r",
    //{"AU04_r",
    //{"AU05_r",
    //{"AU06_r",
    //{"AU07_r",
    {"AU09",  {"faceMuscles.labSup_AN"}},
    {"AU10",  {"faceMuscles.labSup"}},
    {"AU12",  {"faceMuscles.zygomatic"}},
    {"AU14",  {"faceMuscles.buccinator"}},
    {"AU15",  {"faceMuscles.triangularis"}},
    {"AU17",  {"faceMuscles.mentalis"}},
    {"AU20",  {"faceMuscles.risoriusPlatysma"}},
    {"AU23",  {"faceMuscles.incisivus"}},
    {"AU25",  {"faceMuscles.orbOris_loose_lo", "faceMuscles.orbOris_loose_hi"}},
    {"AU26",  {"jaw.rotateZ"}}
    //{"AU45_r",
};
  

//const std::string RENDER_VIDEOS_PATH = "/home/rinat/Workspace/FacialManifoldSource/data_anim/videos";

const std::vector<std::string> ANIM_LIST =
{
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/jali_exports/tyro/anim_weight.txt"
 
 /*
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/results/test/2019-01-17_17-26-08/clean/0000.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/results/test/2019-01-17_17-26-08/noisy/0000.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/results/test/2019-01-17_17-26-08/denoised/0000.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/results/test/2019-01-17_17-26-08/clean/0001.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/results/test/2019-01-17_17-26-08/noisy/0001.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/results/test/2019-01-17_17-26-08/denoised/0001.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/results/test/2019-01-17_17-26-08/clean/0002.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/results/test/2019-01-17_17-26-08/noisy/0002.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/results/test/2019-01-17_17-26-08/denoised/0002.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/results/test/2019-01-17_17-26-08/clean/0003.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/results/test/2019-01-17_17-26-08/noisy/0003.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/results/test/2019-01-17_17-26-08/denoised/0003.txt"
 */
 /*
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00101.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00102.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00103.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00104.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00105.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00106.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00107.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00108.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00109.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00110.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00111.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00112.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00113.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00114.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00115.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00116.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00117.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00118.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00119.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00120.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00121.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00122.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00123.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00124.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00125.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00126.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00127.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00128.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00129.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00130.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00131.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00132.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00133.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00134.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00135.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00136.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00137.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00138.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00139.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00140.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00141.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00142.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00143.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00144.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00145.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00146.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00147.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00148.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00149.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00150.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00151.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00152.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00153.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00154.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00155.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00156.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00157.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00158.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00159.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00160.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00161.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00162.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00163.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00164.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00165.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00166.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00167.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00168.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00169.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00170.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00171.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00172.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00173.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00174.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00175.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00176.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00177.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00178.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00179.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00180.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00181.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00182.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00183.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00184.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00185.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00186.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00187.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00188.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00189.txt",
 "/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/windowed/lower/5_00190.txt"
 */
};

/*{
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0010_02_animation_workshop_0058_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0030_02_animation_workshop_0059_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0040_02_animation_workshop_0050_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0040_02_animation_workshop_0050_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0050_02_animation_workshop_0046_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0060_02_animation_workshop_0139_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0060_02_animation_workshop_0139_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0065_02_animation_workshop_0005_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0065_02_animation_workshop_0005_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0070_02_animation_workshop_0021_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0090_02_animation_workshop_0070_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0110_02_animation_workshop_0106_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0140_02_animation_workshop_0047_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0145_02_animation_workshop_0072_Charles1.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0145_02_animation_workshop_0072_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0170_02_animation_workshop_0052_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0175_02_animation_workshop_0038_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0178_02_animation_workshop_0037_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0180_02_animation_workshop_0058_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0180_02_animation_workshop_0058_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0185_02_animation_workshop_0035_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0210_02_animation_workshop_0046_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/03_0230_02_animation_workshop_0034_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0010_02_animation_workshop_0013_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0020_02_animation_workshop_0036_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0020_02_animation_workshop_0036_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0035_02_animation_workshop_0007_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0038_02_animation_workshop_0206_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0040_02_animation_workshop_0118_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0050_02_animation_workshop_0018_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0060_02_animation_workshop_0007_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0060_02_animation_workshop_0007_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0080_02_animation_workshop_0015_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0090_02_animation_workshop_0071_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0100_02_animation_workshop_0023_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0120_02_animation_workshop_0015_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0130_02_animation_workshop_0023_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0150_02_animation_workshop_0024_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0150_02_animation_workshop_0024_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0170_02_animation_workshop_0020_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0180_02_animation_workshop_0034_Charles1.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0210_02_animation_workshop_0063_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0230_02_animation_workshop_0015_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0260_02_animation_workshop_0026_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/06_0270_02_animation_workshop_0013_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/08_0010_02_animation_workshop_0051_Host.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/08_0050_02_animation_workshop_0025_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/08_0058_02_animation_workshop_0041_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/08_0110_02_animation_workshop_0020_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/08_0120_02_animation_workshop_0071_Charles.txt",
"/home/rinat/Workspace/FacialManifoldSource/data_anim/maya_exports2/processed/14_0010_02_animation_workshop_0032_Charles.txt"
};*/

}