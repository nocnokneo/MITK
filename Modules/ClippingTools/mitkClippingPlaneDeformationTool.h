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

#ifndef mitkClippingPlaneDeformationTool_h_Included
#define mitkClippingPlaneDeformationTool_h_Included

#include <MitkClippingToolsExports.h>

#include "mitkCommon.h"
#include "mitkDataNode.h"
#include "mitkSurfaceDeformationDataInteractor3D.h"
#include "mitkTool.h"

namespace mitk
{
  /**
  \brief A tool with whom you can deform planes.
  */
  class MitkClippingTools_EXPORT ClippingPlaneDeformationTool : public Tool
  {
  public:

    mitkClassMacro(ClippingPlaneDeformationTool, Tool);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;
    virtual const char* GetGroup() const;


  protected:

    ClippingPlaneDeformationTool(); // purposely hidden
    virtual ~ClippingPlaneDeformationTool();

    virtual void Activated();
    virtual void Deactivated();

    mitk::DataNode::Pointer                           m_ClippingPlaneNode;
    mitk::SurfaceDeformationDataInteractor3D::Pointer m_SurfaceInteractor;
  };

} //end namespace mitk

#endif


