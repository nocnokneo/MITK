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


#ifndef MITKNATIVERENDERWINDOWINTERACTOR_H_HEADER_INCLUDED_C1C53722
#define MITKNATIVERENDERWINDOWINTERACTOR_H_HEADER_INCLUDED_C1C53722

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include "itkObject.h"
#include "itkObjectFactory.h"

class vtkRenderWindow;
class vtkRenderWindowInteractor;

namespace mitk
{

class MitkExt_EXPORT NativeRenderWindowInteractor : public itk::Object
{
public:
  mitkClassMacro(NativeRenderWindowInteractor, itk::Object);

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual void Start();

  void SetMitkRenderWindow(vtkRenderWindow * renderwindow);
  itkGetMacro(MitkRenderWindow, vtkRenderWindow*);

protected:
  NativeRenderWindowInteractor();
  virtual ~NativeRenderWindowInteractor();

  vtkRenderWindow* m_MitkRenderWindow;

  vtkRenderWindowInteractor* m_NativeVtkRenderWindowInteractor;
};

}
#endif /* MITKNATIVERENDERWINDOWINTERACTOR_H_HEADER_INCLUDED_C1C53722 */


