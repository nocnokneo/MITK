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

#ifndef MITKFILEREADERSELECTOR_H
#define MITKFILEREADERSELECTOR_H

#include <mitkIFileReader.h>

#include <mitkMimeType.h>

#include <usServiceReference.h>

#include <string>
#include <vector>

namespace mitk {

class BaseData;

class MITK_CORE_EXPORT FileReaderSelector
{
public:

  class Item
  {
  public:

    IFileReader* GetReader() const;
    std::string GetDescription() const;
    IFileReader::ConfidenceLevel GetConfidenceLevel() const;
    MimeType GetMimeType() const;
    us::ServiceReference<IFileReader> GetReference() const;
    long GetServiceId() const;

    bool operator<(const Item& other) const;

  private:

    friend class FileReaderSelector;

    Item();

    us::ServiceReference<IFileReader> m_FileReaderRef;
    IFileReader* m_FileReader;
    IFileReader::ConfidenceLevel m_ConfidenceLevel;
    MimeType m_MimeType;
    long m_Id;
  };

  FileReaderSelector(const FileReaderSelector& other);
  FileReaderSelector(const std::string& path);

  ~FileReaderSelector();

  FileReaderSelector& operator=(const FileReaderSelector& other);

  std::vector<MimeType> GetMimeTypes() const;

  /**
   * @brief Get a sorted list of file reader items.
   *
   * <ol>
   * <li>Confidence level (ascending)</li>
   * <li>MimeType ordering (ascending)</li>
   * <li>File Reader service ranking (ascending)</li>
   * </ol>
   *
   * This means the best matching item is at the back of the returned
   * container.
   *
   * @return
   */
  std::vector<Item> Get() const;

  Item Get(long id) const;

  Item GetDefault() const;
  long GetDefaultId() const;

  Item GetSelected() const;
  long GetSelectedId() const;

  bool Select(const Item& item);
  bool Select(long id);

  void Swap(FileReaderSelector& fws);

private:

  struct Impl;
  us::ExplicitlySharedDataPointer<Impl> m_Data;
};

void swap(FileReaderSelector& fws1, FileReaderSelector& fws2);

}

#endif // MITKFILEREADERSELECTOR_H
