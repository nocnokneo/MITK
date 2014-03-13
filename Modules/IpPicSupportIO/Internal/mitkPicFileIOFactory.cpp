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

#include "mitkPicFileIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkPicFileReader.h"
#include "mitkCoreObjectFactory.h"

#include "itkVersion.h"


namespace mitk
{
PicFileIOFactory::PicFileIOFactory()
{
  this->RegisterOverride("mitkIOAdapter",
                         "mitkPicFileReader",
                         "mitk Pic Image IO",
                         1,
                         itk::CreateObjectFunction<IOAdapter<PicFileReader> >::New());
}

PicFileIOFactory::~PicFileIOFactory()
{
}

const char* PicFileIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* PicFileIOFactory::GetDescription() const
{
  return "PicFile IO Factory, allows the loading of DKFZ Pic images";
}

} // end namespace mitk

struct RegisterIpPicIOFactory{
  RegisterIpPicIOFactory()
    : m_Factory( mitk::PicFileIOFactory::New() )
  {
    MITK_DEBUG << "Registering PicFileIOFactory ";
    itk::ObjectFactoryBase::RegisterFactory( m_Factory );
  }

  ~RegisterIpPicIOFactory()
  {
    itk::ObjectFactoryBase::UnRegisterFactory( m_Factory );
  }

  mitk::PicFileIOFactory::Pointer m_Factory;
};

static RegisterIpPicIOFactory registerIpPicIOFactory;
