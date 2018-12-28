#include "RAES2StandardMesh.h"
#include <vector>
#include "Wm5Vector3.h"
#include "GLStructures.h"
#include "RAFileManager.h"
#include "RACamera.h"
#include "RAVertexHardwareBuffer.h"
#include "RAES2CoreVisualEffects.h"
#include "RAVertexBufferAccessor.h"
#include "Wm5Float2.h"
#include "RAMath.h"
#include "RAAxisAlignedBBox.h"
#include "TyroShaderQuad.h"

namespace tyro
{

void ShaderBox::Init ()
{
    ES2TriMesh::Init();

    ES2ShaderProgram* shader = new ES2ShaderProgram();
    shader->LoadProgram(GetFilePath("Shadertoy", "vsh"), GetFilePath("Shadertoy", "frag"));
    
    ES2VertexFormat* vertexFormat = new ES2VertexFormat(1);
    vertexFormat->SetAttribute(0, shader->GetAttributeLocation("aPosition"), 0, ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_POSITION);
    
    ES2ShaderUniforms* uniforms = nullptr;//new ES2ShaderUniforms(3);
    //uniforms->SetUniform(0, shader->GetUniformLocation("uMVPMatrix"), 1, "uMVPMatrix", ES2ShaderUniforms::UniformMatrix4fv);
    //uniforms->SetUniform(1, shader->GetUniformLocation("uNMatrix"), 1, "uNMatrix", ES2ShaderUniforms::UniformMatrix3fv);
    //uniforms->SetUniform(2, shader->GetUniformLocation("uColor"), 1, "uColor", ES2ShaderUniforms::Uniform4fv);
    
    SetVisualEffect(std::make_shared<ES2VisualEffect>(shader, vertexFormat, uniforms));

    int numVertices = 6;
    int stride = sizeof(Wm5::Vector3f);

    // Create a vertex buffer.
    float points[] = {
        -1.0f,  -1.0f,  0.0f,
        1.0f,  -1.0f,  0.0f,
        1.0f, 1.0f,  0.0f,
        -1.0f,  -1.0f,  0.0f,
        1.0f,  1.0f,  0.0f,
        -1.0f, 1.0f,  0.0f
    };
    
    ES2VertexHardwareBufferSPtr vbuffer = std::make_shared<ES2VertexHardwareBuffer>(stride, numVertices, points, HardwareBuffer::BU_STATIC);
    LocalBoundBox = AxisAlignedBBox(Wm5::Vector3f(-1, -1, 0), Wm5::Vector3f(1, 1, 0));

    SetVertexBuffer(vbuffer);
    ES2VertexArraySPtr varray = std::make_shared<ES2VertexArray>(this->GetVisualEffect(), vbuffer);
	SetVertexArray(varray);
}
    
ShaderBoxSPtr ShaderBox::Create()
{
    ShaderBoxSPtr ptr = std::make_shared<ShaderBox>();
    ptr->Init();
    return ptr;
}

void ShaderBox::UpdateUniformsWithCamera(const Camera* camera) 
{}

 
}