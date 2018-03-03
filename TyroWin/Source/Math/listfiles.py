import os
import os.path
from natsort import natsorted

def run(FOLDER_NAME, filecmake):
    #FOLDER_NAME = "Approximation"
    mypath = "./"+FOLDER_NAME
    onlyfiles = [f for f in os.listdir(mypath) if os.path.isfile(os.path.join(mypath, f))]

    src_files = []
    inc_files = []

    for f in onlyfiles:
        ext = os.path.splitext(f)[1]
        if ext == ".h" or ext == ".inl":
            inc_files.append(f)
        elif ext == ".cpp":
            src_files.append(f)

    inc_files = natsorted(inc_files)
    src_files = natsorted(src_files)

    source_str = "set(TYRO_SRC"+"_"+FOLDER_NAME.upper() + " \n" 
    i = 0
    for f in src_files:
        path = os.path.join(FOLDER_NAME, f)
        source_str += "\"" + path + "\" \n"
        
        i+=1

    source_str += ")\n\n"
    
    inc_str = "set(TYRO_INC"+"_"+FOLDER_NAME.upper() + " \n" 
    i = 0
    for f in inc_files:
        path = os.path.join(FOLDER_NAME, f)
        inc_str += "\"" + path + "\" \n"
        

        i+=1

    inc_str += ")\n\n"


    if len(inc_files) > 0:
        filecmake.write(inc_str)
    
    if len(src_files) > 0:
        filecmake.write(source_str)
    
  

list_of_folder = [
"Algebra",
"Approximation",
"Assert",
"Base",
"ComputationalGeometry",
"Containment",
"CurvesSurfacesVolumes",
"DataTypes",
"Distance",
"InputOutput",
"Interpolation",
"Intersection",
"Memory",
"Meshes",
"Miscellaneous",
"NumericalAnalysis",
"Objects2D",
"Objects3D",
"ObjectSystems",
"Query",
"Rational",
"Threading",
"Time",
"Tools",
"Transformation"
]

filecmake = open(os.path.join(".", "CMakeSources.cmake"), "w")
for FOLDER_NAME in list_of_folder:
    run(FOLDER_NAME, filecmake)

total_folder = "Math"
total_src_str = "set(TYRO_"+total_folder.upper()+"_SRC\n"
i=0
for FOLDER_NAME in list_of_folder:
    n = "TYRO_SRC"+"_"+FOLDER_NAME.upper()
    total_src_str+="${" + n + "}\n"
    
    i+=1

total_src_str += ")"
filecmake.write(total_src_str)
filecmake.close()


