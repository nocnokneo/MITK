/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "vtkMitkRenderProp.h"

#include <vtkObjectFactory.h>
#include <vtkLODProp3D.h>
#include <vtkPropAssembly.h>

#include "mitkVtkMapper.h"
#include "mitkGLMapper.h"


vtkStandardNewMacro(vtkMitkRenderProp);

vtkMitkRenderProp::vtkMitkRenderProp()
{
}
vtkMitkRenderProp::~vtkMitkRenderProp()
{
}

double *vtkMitkRenderProp::GetBounds()
{
  return const_cast<double*>(m_VtkPropRenderer->GetBounds());
}

void vtkMitkRenderProp::SetPropRenderer(mitk::VtkPropRenderer::Pointer propRenderer)
{
  this->m_VtkPropRenderer = propRenderer;
}

int vtkMitkRenderProp::RenderOpaqueGeometry(vtkViewport* /*viewport*/)
{
  return m_VtkPropRenderer->Render(mitk::VtkPropRenderer::Opaque);
}

int vtkMitkRenderProp::RenderOverlay(vtkViewport* /*viewport*/)
{
  return m_VtkPropRenderer->Render(mitk::VtkPropRenderer::Overlay);
}

void vtkMitkRenderProp::ReleaseGraphicsResources(vtkWindow* window)
{
  m_VtkPropRenderer->ReleaseGraphicsResources(window);
}

void vtkMitkRenderProp::InitPathTraversal()
{
  m_VtkPropRenderer->InitPathTraversal();
}

vtkAssemblyPath* vtkMitkRenderProp::GetNextPath()
{
  return m_VtkPropRenderer->GetNextPath();
}
//BUG (#1551) added method depth peeling
int vtkMitkRenderProp::HasTranslucentPolygonalGeometry()
{
  // I do not see the vtkLODProp3D issue and disabling this method breaks translucent rendering <Taylor>
  /*return 0;  If I don't deactivate this function, I get lots of output like 

               ERROR: In /home/maleike/build/mitk2/VTK-src/Rendering/vtkLODProp3D.cxx, line 969
               vtkLODProp3D (0x7f810452f000): Index out of range!
            */
  typedef std::map<int,mitk::Mapper*> MappersMapType;
  MappersMapType mappersMap = m_VtkPropRenderer->GetMappersMap();
  for(MappersMapType::iterator it = mappersMap.begin(); it != mappersMap.end(); it++)
  {
    mitk::Mapper * mapper = (*it).second;

    mitk::VtkMapper::Pointer vtkMapper = dynamic_cast<mitk::VtkMapper*>(mapper);
    if(vtkMapper)
    {
      // Due to VTK 5.2 bug, we need to initialize the Paths object in vtkPropAssembly
      // manually (see issue #8186 committed to VTK's Mantis issue tracker)
      // --> VTK bug resolved on 2008-12-01
      vtkPropAssembly *propAssembly = dynamic_cast< vtkPropAssembly * >(
          vtkMapper->GetVtkProp(m_VtkPropRenderer) );
      if ( propAssembly )
      {
        propAssembly->InitPathTraversal();
      }

      if (vtkMapper->GetVtkProp(m_VtkPropRenderer)->HasTranslucentPolygonalGeometry()==1)
        return 1;
    }
  }
  return 0;
}

int vtkMitkRenderProp::RenderTranslucentPolygonalGeometry( vtkViewport * )
{
  return m_VtkPropRenderer->Render(mitk::VtkPropRenderer::Translucent);
}
int vtkMitkRenderProp::RenderVolumetricGeometry( vtkViewport * )
{
  return m_VtkPropRenderer->Render(mitk::VtkPropRenderer::Volumetric);
}

