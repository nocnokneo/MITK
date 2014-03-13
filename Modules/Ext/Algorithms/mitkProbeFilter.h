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

#ifndef MITKPROBEFILTER_H_HEADER_INCLUDED_C10B22CD
#define MITKPROBEFILTER_H_HEADER_INCLUDED_C10B22CD

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include "mitkSurfaceSource.h"

class vtkPlaneSource;
class vtkTransformPolyDataFilter;
class vtkDataSetToPolyDataFilter;

namespace mitk {

class Surface;
class Image;

//##Documentation
//## @brief Adapter for vtkProbeFilter, making it a 3D+t filter
//##
//## @ingroup Process
class MitkExt_EXPORT ProbeFilter : public SurfaceSource
{
public:
  mitkClassMacro(ProbeFilter, SurfaceSource);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual void GenerateOutputInformation();
  virtual void GenerateInputRequestedRegion();
  virtual void GenerateData();

  const mitk::Surface *GetInput(void);
  const mitk::Image *GetSource(void);

  virtual void SetInput(const mitk::Surface *input);
  virtual void SetSource(const mitk::Image *source);

protected:
  ProbeFilter();

  virtual ~ProbeFilter();
};

} // namespace mitk

#endif /* MITKPROBEFILTER_H_HEADER_INCLUDED_C10B22CD */
