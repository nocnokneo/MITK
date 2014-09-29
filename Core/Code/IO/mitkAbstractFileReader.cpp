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

#include <mitkAbstractFileReader.h>

#include <mitkIOUtil.h>
#include <mitkCustomMimeType.h>

#include <Internal/mitkFileReaderWriterBase.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usPrototypeServiceFactory.h>

#include <itksys/SystemTools.hxx>

#include <fstream>

namespace mitk {

class AbstractFileReader::Impl : public FileReaderWriterBase
{
public:

  Impl()
    : FileReaderWriterBase()
    , m_PrototypeFactory(NULL)
  {}

  Impl(const Impl& other)
    : FileReaderWriterBase(other)
    , m_PrototypeFactory(NULL)
  {}

  us::PrototypeServiceFactory* m_PrototypeFactory;
  us::ServiceRegistration<IFileReader> m_Reg;

};


AbstractFileReader::AbstractFileReader()
  : d(new Impl)
{
}

AbstractFileReader::~AbstractFileReader()
{
  UnregisterService();

  delete d->m_PrototypeFactory;
}

AbstractFileReader::AbstractFileReader(const AbstractFileReader& other)
  : d(new Impl(*other.d.get()))
{
}

AbstractFileReader::AbstractFileReader(const CustomMimeType& mimeType, const std::string& description)
  : d(new Impl)
{
  d->SetMimeType(mimeType);
  d->SetDescription(description);
}

AbstractFileReader::AbstractFileReader(const std::string& extension, const std::string& description)
  : d(new Impl)
{
  CustomMimeType customMimeType;
  customMimeType.AddExtension(extension);
  d->SetMimeType(customMimeType);
  d->SetDescription(description);
}

////////////////////// Reading /////////////////////////

std::vector<itk::SmartPointer<BaseData> > AbstractFileReader::Read(const std::string& path)
{
  if (!itksys::SystemTools::FileExists(path.c_str()))
  {
    mitkThrow() << "File '" + path + "' not found.";
  }
  std::ifstream stream;
  stream.open(path.c_str(), std::ios_base::in | std::ios_base::binary);
  try
  {
    return this->Read(stream);
  }
  catch (mitk::Exception& e)
  {
    mitkReThrow(e) << "Error reading file '" << path << "'";
  }
  catch (const std::exception& e)
  {
    mitkThrow() << "Error reading file '" << path << "': " << e.what();
  }
}

std::vector<itk::SmartPointer<BaseData> > AbstractFileReader::Read(std::istream& stream)
{
  // Create a temporary file and copy the data to it
  std::ofstream tmpOutputStream;
  std::string tmpFilePath = IOUtil::CreateTemporaryFile(tmpOutputStream);
  tmpOutputStream << stream.rdbuf();
  tmpOutputStream.close();

  // Now read from the temporary file
  std::vector<itk::SmartPointer<BaseData> > result = this->Read(tmpFilePath);
  std::remove(tmpFilePath.c_str());
  return result;
}

DataStorage::SetOfObjects::Pointer AbstractFileReader::Read(const std::string& path, DataStorage& ds)
{
  DataStorage::SetOfObjects::Pointer result = DataStorage::SetOfObjects::New();
  std::vector<BaseData::Pointer> data = this->Read(path);
  for (std::vector<BaseData::Pointer>::iterator iter = data.begin();
       iter != data.end(); ++iter)
  {
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(*iter);
    this->SetDefaultDataNodeProperties(node, path);
    ds.Add(node);
    result->InsertElement(result->Size(), node);
  }
  return result;
}

DataStorage::SetOfObjects::Pointer AbstractFileReader::Read(std::istream& stream, DataStorage& ds)
{
  DataStorage::SetOfObjects::Pointer result = DataStorage::SetOfObjects::New();
  std::vector<BaseData::Pointer> data = this->Read(stream);
  for (std::vector<BaseData::Pointer>::iterator iter = data.begin();
       iter != data.end(); ++iter)
  {
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(*iter);
    this->SetDefaultDataNodeProperties(node, std::string());
    ds.Add(node);
    result->InsertElement(result->Size(), node);
  }
  return result;
}

IFileReader::ConfidenceLevel AbstractFileReader::GetConfidenceLevel(const std::string& path) const
{
  return itksys::SystemTools::FileExists(path.c_str(), true) ? Supported : Unsupported;
}


//////////// µS Registration & Properties //////////////

us::ServiceRegistration<IFileReader> AbstractFileReader::RegisterService(us::ModuleContext* context)
{
  if (d->m_PrototypeFactory) return us::ServiceRegistration<IFileReader>();

  if(context == NULL)
  {
    context = us::GetModuleContext();
  }

  d->RegisterMimeType(context);

  if (this->GetMimeType().GetName().empty())
  {
    MITK_WARN << "Not registering reader due to empty MIME type.";
    return us::ServiceRegistration<IFileReader>();
  }

  struct PrototypeFactory : public us::PrototypeServiceFactory
  {
    AbstractFileReader* const m_Prototype;

    PrototypeFactory(AbstractFileReader* prototype)
      : m_Prototype(prototype)
    {}

    us::InterfaceMap GetService(us::Module* /*module*/, const us::ServiceRegistrationBase& /*registration*/)
    {
      return us::MakeInterfaceMap<IFileReader>(m_Prototype->Clone());
    }

    void UngetService(us::Module* /*module*/, const us::ServiceRegistrationBase& /*registration*/,
      const us::InterfaceMap& service)
    {
      delete us::ExtractInterface<IFileReader>(service);
    }
  };

  d->m_PrototypeFactory = new PrototypeFactory(this);
  us::ServiceProperties props = this->GetServiceProperties();
  d->m_Reg = context->RegisterService<IFileReader>(d->m_PrototypeFactory, props);
  return d->m_Reg;
}

void AbstractFileReader::UnregisterService()
{
  try
  {
    d->m_Reg.Unregister();
  }
  catch (const std::exception&)
  {}
}

us::ServiceProperties AbstractFileReader::GetServiceProperties() const
{
  us::ServiceProperties result;

  result[IFileReader::PROP_DESCRIPTION()] = this->GetDescription();
  result[IFileReader::PROP_MIMETYPE()] = this->GetMimeType().GetName();
  result[us::ServiceConstants::SERVICE_RANKING()]  = this->GetRanking();
  return result;
}

us::ServiceRegistration<CustomMimeType> AbstractFileReader::RegisterMimeType(us::ModuleContext* context)
{
  return d->RegisterMimeType(context);
}

void AbstractFileReader::SetMimeType(const CustomMimeType& mimeType)
{
  d->SetMimeType(mimeType);
}

void AbstractFileReader::SetDescription(const std::string& description)
{
  d->SetDescription(description);
}

void AbstractFileReader::SetRanking(int ranking)
{
  d->SetRanking(ranking);
}

int AbstractFileReader::GetRanking() const
{
  return d->GetRanking();
}

//////////////////////// Options ///////////////////////

void AbstractFileReader::SetDefaultOptions(const IFileReader::Options& defaultOptions)
{
  d->SetDefaultOptions(defaultOptions);
}

IFileReader::Options AbstractFileReader::GetDefaultOptions() const
{
  return d->GetDefaultOptions();
}

IFileReader::Options AbstractFileReader::GetOptions() const
{
  return d->GetOptions();
}

us::Any AbstractFileReader::GetOption(const std::string& name) const
{
  return d->GetOption(name);
}

void AbstractFileReader::SetOptions(const Options& options)
{
  d->SetOptions(options);
}

void AbstractFileReader::SetOption(const std::string& name, const us::Any& value)
{
  d->SetOption(name, value);
}

////////////////// MISC //////////////////

void AbstractFileReader::AddProgressCallback(const ProgressCallback& callback)
{
  d->AddProgressCallback(callback);
}

void AbstractFileReader::RemoveProgressCallback(const ProgressCallback& callback)
{
  d->RemoveProgressCallback(callback);
}

////////////////// µS related Getters //////////////////


CustomMimeType AbstractFileReader::GetMimeType() const
{
  return d->GetMimeType();
}

std::string AbstractFileReader::GetDescription() const
{
  return d->GetDescription();
}

void AbstractFileReader::SetDefaultDataNodeProperties(DataNode* node, const std::string& filePath)
{
  // path
  if (!filePath.empty())
  {
    mitk::StringProperty::Pointer pathProp = mitk::StringProperty::New( itksys::SystemTools::GetFilenamePath(filePath) );
    node->SetProperty(StringProperty::PATH, pathProp);
  }

  // name already defined?
  mitk::StringProperty::Pointer nameProp = dynamic_cast<mitk::StringProperty*>(node->GetProperty("name"));
  if(nameProp.IsNull() || (strcmp(nameProp->GetValue(),"No Name!")==0))
  {
    // name already defined in BaseData
    mitk::StringProperty::Pointer baseDataNameProp = dynamic_cast<mitk::StringProperty*>(node->GetData()->GetProperty("name").GetPointer() );
    if(baseDataNameProp.IsNull() || (strcmp(baseDataNameProp->GetValue(),"No Name!")==0))
    {
      // name neither defined in node, nor in BaseData -> name = filebasename;
      nameProp = mitk::StringProperty::New(itksys::SystemTools::GetFilenameWithoutExtension(itksys::SystemTools::GetFilenameName(filePath)));
      node->SetProperty("name", nameProp);
    }
    else
    {
      // name defined in BaseData!
      nameProp = mitk::StringProperty::New(baseDataNameProp->GetValue());
      node->SetProperty("name", nameProp);
    }
  }

  // visibility
  if(!node->GetProperty("visible"))
  {
    node->SetVisibility(true);
  }
}

}
