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

// MITK
#include "mitkOtsuTool3D.h"
#include "mitkToolManager.h"
#include "mitkRenderingManager.h"
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkRenderingModeProperty.h>
#include <mitkLevelWindowProperty.h>
#include <mitkLookupTableProperty.h>
#include "mitkOtsuSegmentationFilter.h"

// ITK
#include <itkOtsuMultipleThresholdsImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>

#include "mitkRegionGrow3DTool.xpm"

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, OtsuTool3D, "Otsu Segmentation");
}

mitk::OtsuTool3D::OtsuTool3D()
{
}

mitk::OtsuTool3D::~OtsuTool3D()
{

}

void mitk::OtsuTool3D::Activated()
{
  if (m_ToolManager)
  {
    m_OriginalImage = dynamic_cast<mitk::Image*>(m_ToolManager->GetReferenceData(0)->GetData());

    m_BinaryPreviewNode = mitk::DataNode::New();
    m_BinaryPreviewNode->SetName("Binary_Preview");
    //m_BinaryPreviewNode->SetBoolProperty("helper object", true);
    //m_BinaryPreviewNode->SetProperty("binary", mitk::BoolProperty::New(true));
    m_ToolManager->GetDataStorage()->Add( this->m_BinaryPreviewNode );

    m_MultiLabelResultNode = mitk::DataNode::New();
    m_MultiLabelResultNode->SetName("Otsu_Preview");
    //m_MultiLabelResultNode->SetBoolProperty("helper object", true);
    m_MultiLabelResultNode->SetVisibility(true);

    m_MaskedImagePreviewNode = mitk::DataNode::New();
    m_MaskedImagePreviewNode->SetName("Volume_Preview");
    //m_MultiLabelResultNode->SetBoolProperty("helper object", true);
    m_MaskedImagePreviewNode->SetVisibility(false);

    m_ToolManager->GetDataStorage()->Add( this->m_MultiLabelResultNode );
  }
}

void mitk::OtsuTool3D::Deactivated()
{
  m_ToolManager->GetDataStorage()->Remove( this->m_MultiLabelResultNode );
  m_MultiLabelResultNode = NULL;
  m_ToolManager->GetDataStorage()->Remove( this->m_BinaryPreviewNode );
  m_BinaryPreviewNode = NULL;
  m_ToolManager->ActivateTool(-1);
}

const char** mitk::OtsuTool3D::GetXPM() const
{
  return NULL;
}

us::ModuleResource mitk::OtsuTool3D::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Otsu_48x48.png");
  return resource;
}

void mitk::OtsuTool3D::RunSegmentation(int regions)
{
  //this->m_OtsuSegmentationDialog->setCursor(Qt::WaitCursor);

  int numberOfThresholds = regions - 1;

  mitk::OtsuSegmentationFilter::Pointer otsuFilter = mitk::OtsuSegmentationFilter::New();
  otsuFilter->SetNumberOfThresholds( numberOfThresholds );
  otsuFilter->SetInput( m_OriginalImage );

  try
  {
    otsuFilter->Update();
  }
  catch( ... )
  {
    mitkThrow() << "itkOtsuFilter error (image dimension must be in {2, 3} and image must not be RGB)";
  }

  m_ToolManager->GetDataStorage()->Remove( this->m_MultiLabelResultNode );
  m_MultiLabelResultNode = NULL;
  m_MultiLabelResultNode = mitk::DataNode::New();
  m_MultiLabelResultNode->SetName("Otsu_Preview");
  m_MultiLabelResultNode->SetVisibility(true);
  m_ToolManager->GetDataStorage()->Add( this->m_MultiLabelResultNode );
  m_MultiLabelResultNode->SetOpacity(1.0);

  this->m_MultiLabelResultNode->SetData( otsuFilter->GetOutput() );
  m_MultiLabelResultNode->SetProperty("binary", mitk::BoolProperty::New(false));
  mitk::RenderingModeProperty::Pointer renderingMode = mitk::RenderingModeProperty::New();
  renderingMode->SetValue( mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR );
  m_MultiLabelResultNode->SetProperty("Image Rendering.Mode", renderingMode);
  mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
  mitk::LookupTableProperty::Pointer prop = mitk::LookupTableProperty::New(lut);
  vtkLookupTable *lookupTable = vtkLookupTable::New();
  lookupTable->SetHueRange(1.0, 0.0);
  lookupTable->SetSaturationRange(1.0, 1.0);
  lookupTable->SetValueRange(1.0, 1.0);
  lookupTable->SetTableRange(-1.0, 1.0);
  lookupTable->Build();
  lut->SetVtkLookupTable(lookupTable);
  prop->SetLookupTable(lut);
  m_MultiLabelResultNode->SetProperty("LookupTable",prop);
  mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
  mitk::LevelWindow levelwindow;
  levelwindow.SetRangeMinMax(0, numberOfThresholds + 1);
  levWinProp->SetLevelWindow( levelwindow );
  m_MultiLabelResultNode->SetProperty( "levelwindow", levWinProp );

  //m_BinaryPreviewNode->SetVisibility(false);
//  m_MultiLabelResultNode->SetVisibility(true);
  //this->m_OtsuSegmentationDialog->setCursor(Qt::ArrowCursor);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::OtsuTool3D::ConfirmSegmentation()
{
  GetTargetSegmentationNode()->SetData(dynamic_cast<mitk::Image*>(m_BinaryPreviewNode->GetData()));
}

void mitk::OtsuTool3D::UpdateBinaryPreview(int regionID)
{
  m_MultiLabelResultNode->SetVisibility(false);
  //pixel with regionID -> binary image
  const unsigned short dim = 3;
  typedef unsigned char PixelType;

  typedef itk::Image< PixelType, dim > InputImageType;
  typedef itk::Image< PixelType, dim > OutputImageType;

  typedef itk::BinaryThresholdImageFilter< InputImageType, OutputImageType > FilterType;

  FilterType::Pointer filter = FilterType::New();

  InputImageType::Pointer itkImage;

  mitk::Image::Pointer multiLabelSegmentation = dynamic_cast<mitk::Image*>(m_MultiLabelResultNode->GetData());
  mitk::CastToItkImage(multiLabelSegmentation, itkImage);

  filter->SetInput(itkImage);
  filter->SetLowerThreshold(regionID);
  filter->SetUpperThreshold(regionID);
  filter->Update();
  mitk::Image::Pointer binarySegmentation;
  mitk::CastToMitkImage( filter->GetOutput(), binarySegmentation);
  m_BinaryPreviewNode->SetData(binarySegmentation);
  m_BinaryPreviewNode->SetVisibility(true);
  m_BinaryPreviewNode->SetProperty("outline binary", mitk::BoolProperty::New(false));
  m_BinaryPreviewNode->SetOpacity(1.0);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

const char* mitk::OtsuTool3D::GetName() const
{
  return "Otsu";
}

void mitk::OtsuTool3D::UpdateVolumePreview(bool volumeRendering)
{
  if (volumeRendering)
  {
    m_MaskedImagePreviewNode->SetBoolProperty("volumerendering", true);
    m_MaskedImagePreviewNode->SetBoolProperty("volumerendering.uselod", true);
  }
  else
  {
    m_MaskedImagePreviewNode->SetBoolProperty("volumerendering", false);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::OtsuTool3D::ShowMultiLabelResultNode(bool show)
{
  m_MultiLabelResultNode->SetVisibility(show);
  m_BinaryPreviewNode->SetVisibility(!show);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
