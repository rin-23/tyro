import os
import sys
import numpy as np
import time

#import scipy.io as sio
#import csv
#import maya.cmds as cmds
#import os
#import pandas as pd

import maya.cmds as cmds

def export_bshapes_to_obj(selection_list,  attributes, directory, special_import, special_export):
    for i in range(len(selection_list)):
        if i == 0:
            cmds.select(selection_list[i], r=True)
        else:
            cmds.select(selection_list[i], add=True)

    if not os.path.exists(directory):
        os.makedirs(directory)

    objwritesmth = os.path.join(directory, "objlist.txt")
    print objwritesmth
    lul = open(objwritesmth, "w")
        
    for aIdx in range(len(attributes)):
        attrib_name = attributes[aIdx]
        attrib_value = 1.0
        if aIdx in special_import:
            attrib_value = special_import[aIdx](attrib_value)  # custom behaviour for attribute
        cmds.setAttr(attrib_name, attrib_value)

        filename = os.path.join(directory, str(attrib_name) + ".obj")
        cmds.file(filename, pr=1, 
                  typ="OBJexport", es=1, 
                  op="groups=0; ptgroups=0; materials=0; smoothing=1; normals=0")
        
        attrib_value = 0
        if aIdx in special_export:
            attrib_value = special_export[aIdx](attrib_value)  # custom behaviour for attribute
        cmds.setAttr(attrib_name, attrib_value)
        lul.write(filename + "\n")
        print "\"" + filename + "\","

    lul.close()

def export_all_obj_valley_girl():
    # MAKE SURE ALL ANIMATIONS ARE TURNED OFF
    list_lower = ["head_output"]
    folder = os.path.normpath("/Users/rinat/Desktop")
    obj_path = os.path.join(folder, "ValleyGirlAsiaBlendshape")
    
    BSHAPES =    [  
    "faceMuscles.frontalis",                        
    "faceMuscles.currogator",                       
    "faceMuscles.medFrontalis",                     
    "faceMuscles.orbOculi",                         
    "faceMuscles.orbOculi_L",                       
    "faceMuscles.orbOculi_R",                       
    "faceMuscles.orbOculi_out",                     
    "faceMuscles.latFrontalis",                     
    "faceMuscles.latFrontalis_L",                   
    "faceMuscles.latFrontalis_R",                   
    "faceMuscles.labInf",                           
    "faceMuscles.zygomatic",                        
    "faceMuscles.labSup",                           
    "faceMuscles.labSup_AN",                        
    "faceMuscles.triangularis",                     
    "faceMuscles.incisivus",                        
    "faceMuscles.mentalis",                         
    "faceMuscles.risPlatysma",                 
    "faceMuscles.orbOris_loose_lo",                 
    "faceMuscles.orbOris_loose_hi",                 
    "faceMuscles.orbOris_tight_lo",                 
    "faceMuscles.orbOris_tight_hi",                                        
    "faceMuscles.mouthClose",                       
    "faceMuscles.orbOculi_lo",                      
    "faceMuscles.buccinator",                       
    "faceMuscles.orbOculi_lo_L",                    
    "faceMuscles.orbOculi_lo_R",                    
    "faceMuscles.labSup_L",                         
    "faceMuscles.labSup_R",                         
    "faceMuscles.zygomatic_L",                      
    "faceMuscles.zygomatic_R",                      
    "faceMuscles.risPlatysma_L",               
    "faceMuscles.risPlatysma_R",               
    "faceMuscles.levAnguliOris",                    
    "faceMuscles.dilatorNaris",                     
    "faceMuscles.Zyg_Minor",                        
    "faceMuscles.mentalis_lowerLip",                
    "faceMuscles.triangularis_L",                   
    "faceMuscles.triangularis_R",                   
#    "faceMuscles.orbOris_up_hi",                    
    "faceMuscles.jawOpen_comp",                      
    "faceMuscles.blow"                             
#    "jaw.rotateZ",                                  
    #"jaw.rotateY"                                 
#    "jaw.translateX",                              
    #"blink_ctl.translateY",                       
    #"loBlink_ctl.translateY"                      
#    "blink_ctl.translateY_pos",                     
#    "blink_ctl.translateY_neg",                     
#    "loBlink_ctl.translateY_pos",                   
#    "loBlink_ctl.translateY_neg"                    
    ]

    attributes = BSHAPES
    #special_import = {5:i1, 21:i2, 22:i3}
    #special_export = {5:e1, 21:e2, 22:e3}
    s = {}
    export_bshapes_to_obj(list_lower, attributes, obj_path, s, s)

export_all_obj_valley_girl()
    
'''
 BSHAPES_BABY =    [  
    "faceMuscles.frontalis",                        
    "faceMuscles.currogator",                       
    "faceMuscles.medFrontalis",                     
    "faceMuscles.orbOculi",                         
    "faceMuscles.orbOculi_L",                       
    "faceMuscles.orbOculi_R",                       
#    "faceMuscles.orbOculi_out",                     
    "faceMuscles.latFrontalis",                     
    "faceMuscles.latFrontalis_L",                   
    "faceMuscles.latFrontalis_R",                   
    "faceMuscles.labInf",                           
    "faceMuscles.Zyg_Major",                        
    "faceMuscles.labSup",                           
    "faceMuscles.labSup_AN",                        
    "faceMuscles.triangularis",                     
#    "faceMuscles.incisivus",                        
    "faceMuscles.mentalis",                         
    "faceMuscles.risPlatysma",                 
#    "faceMuscles.orbOris_loose_lo",                 
#    "faceMuscles.orbOris_loose_hi",                 
#    "faceMuscles.orbOris_tight_lo", 
    "faceMuscles.orbOris_down_hi",                
    "faceMuscles.orbOris_tight_hi",                                        
#    "faceMuscles.mouthClose",                       
#    "faceMuscles.orbOculi_lo",                      
#    "faceMuscles.buccinator",                       
#    "faceMuscles.orbOculi_lo_L",                    
#    "faceMuscles.orbOculi_lo_R",                    
    "faceMuscles.labSup_L",                         
    "faceMuscles.labSup_R",                         
#    "faceMuscles.zygomatic_L",                      
#    "faceMuscles.zygomatic_R",                      
    "faceMuscles.risPlatysma_L",               
    "faceMuscles.risPlatysma_R",               
#    "faceMuscles.levAnguliOris",                    
#    "faceMuscles.dilatorNaris",                     
    "faceMuscles.Zyg_Minor",                        
    "faceMuscles.mentalis_lowerLip",                
    "faceMuscles.triangularis_L",                   
    "faceMuscles.triangularis_R",                   
#    "faceMuscles.orbOris_up_hi",                    
#    "faceMuscles.jawOpen_comp",                      
#    "faceMuscles.blow"                             
#    "jaw.rotateZ",                                  
    #"jaw.rotateY"                                 
#    "jaw.translateX",                              
    #"blink_ctl.translateY",                       
    #"loBlink_ctl.translateY"                      
#    "blink_ctl.translateY_pos",                     
#    "blink_ctl.translateY_neg",                     
#    "loBlink_ctl.translateY_pos",                   
#    "loBlink_ctl.translateY_neg"                    
    ]
    '''