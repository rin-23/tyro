#pragma once

#include <vector>
#include <map>
#include <string>

namespace tyro 
{
class CameraControl
{
public:
    typedef enum 
    {
        UPPER,
        LOWER,
        MID_JAW,
        LIPS,
        NONE_PART
    } FacePart;
    
    CameraControl();
    ~CameraControl() {}

    void Init(); 

    void Update(std::vector<std::string>& names, std::vector<double>& values, bool fromcsv=false);
    
    void Print();
    FacePart face_part;

    std::vector<std::string> low_bnames;
    std::vector<double> low_values;

    std::vector<std::string> up_bnames;
    std::vector<double> up_values;
};
}