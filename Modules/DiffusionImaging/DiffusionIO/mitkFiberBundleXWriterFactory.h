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

#ifndef FIBERBUNDLEX_WRITERFACTORY_H_HEADER_INCLUDED
#define FIBERBUNDLEX_WRITERFACTORY_H_HEADER_INCLUDED

#include "itkObjectFactoryBase.h"
#include "mitkBaseData.h"

namespace mitk
{

class FiberBundleXWriterFactory : public itk::ObjectFactoryBase
{
public:

  mitkClassMacro( mitk::FiberBundleXWriterFactory, itk::ObjectFactoryBase )

  /** Class methods used to interface with the registered factories. */
  virtual const char* GetITKSourceVersion(void) const;
  virtual const char* GetDescription(void) const;

  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);

protected:
  FiberBundleXWriterFactory();
  ~FiberBundleXWriterFactory();

private:
  FiberBundleXWriterFactory(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};

} // end namespace mitk

#endif // FiberBundleX_WRITERFACTORY_H_HEADER_INCLUDED



