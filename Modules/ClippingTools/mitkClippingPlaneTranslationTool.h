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

#ifndef mitkClippingPlaneTranslationTool_h_Included
#define mitkClippingPlaneTranslationTool_h_Included

#include <MitkClippingToolsExports.h>

#include "mitkAffineDataInteractor3D.h"
#include "mitkCommon.h"
#include "mitkDataNode.h"
#include "mitkTool.h"


namespace us {
  class Module;
}


namespace mitk
{

  /**
  \brief A tool which allows you to move planes.
  */
  class MitkClippingTools_EXPORT ClippingPlaneTranslationTool : public Tool
  {
  public:

    mitkClassMacro(ClippingPlaneTranslationTool, Tool);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;
    virtual const char* GetGroup() const;


  protected:

    ClippingPlaneTranslationTool(); // purposely hidden
    virtual ~ClippingPlaneTranslationTool();

    virtual void Activated();
    virtual void Deactivated();

    mitk::DataNode::Pointer               m_ClippingPlaneNode;
    mitk::AffineDataInteractor3D::Pointer m_AffineDataInteractor;

  };

} //end namespace mitk

#endif


