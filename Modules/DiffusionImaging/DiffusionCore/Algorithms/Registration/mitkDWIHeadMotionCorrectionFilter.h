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

#ifndef MITKDWIHEADMOTIONCORRECTIONFILTER_H
#define MITKDWIHEADMOTIONCORRECTIONFILTER_H

#include "mitkDiffusionImageToDiffusionImageFilter.h"

#include <itkAccumulateImageFilter.h>
#include "mitkITKImageImport.h"

namespace mitk
{

/**
 * @class DWIHeadMotionCorrectionFilter
 *
 * @brief Performs standard head-motion correction by using affine registration of the gradient images.
 *
 * (Head) motion correction is a essential pre-processing step before performing any further analysis of a diffusion-weighted
 * images since all model fits ( tensor, QBI ) rely on an aligned diffusion-weighted dataset. The correction is done in two steps. First the
 * unweighted images ( if multiple present ) are separately registered on the first one by means of rigid registration and normalized correlation
 * as error metric. Second, the weighted gradient images are registered to the unweighted reference ( computed as average from the aligned images from first step )
 * by an affine transformation using the MattesMutualInformation metric as optimizer guidance.
 *
 */
template< typename DiffusionPixelType>
class DWIHeadMotionCorrectionFilter
    : public DiffusionImageToDiffusionImageFilter< DiffusionPixelType >
{
public:

  // class macros
  mitkClassMacro( DWIHeadMotionCorrectionFilter,
                  DiffusionImageToDiffusionImageFilter<DiffusionPixelType> )

  itkNewMacro(Self)

  // public typedefs
  typedef typename Superclass::InputImageType         DiffusionImageType;
  typedef typename Superclass::InputImagePointerType  DiffusionImagePointerType;

  typedef typename Superclass::OutputImageType        OutputImageType;
  typedef typename Superclass::OutputImagePointerType OutputImagePointerType;

protected:
  DWIHeadMotionCorrectionFilter();
  virtual ~DWIHeadMotionCorrectionFilter() {}

  virtual void GenerateData();

  /**
   * @brief Averages an 3d+t image along the time axis.
   *
   * The method uses the AccumulateImageFilter as provided by ITK and collapses the given 3d+t image
   * to an 3d image while computing the average over the time axis for each of the spatial voxels.
   */
  template< typename TPixel, unsigned int VDimensions>
  static void ItkAccumulateFilter(
      const itk::Image< TPixel, VDimensions>* image,
      mitk::Image::Pointer& output)
  {
    // input 3d+t --> output 3d
    typedef itk::Image< TPixel, 4> InputItkType;
    typedef itk::Image< TPixel, 3> OutputItkType;
    typedef typename itk::AccumulateImageFilter< InputItkType, OutputItkType > FilterType;

    typename FilterType::Pointer filter = FilterType::New();
    filter->SetInput( image );
    filter->SetAccumulateDimension( 3 );
    filter->SetAverage( true );

    try
    {
      filter->Update();
    }
    catch( const itk::ExceptionObject& e)
    {
      mitkThrow() << " Exception while averaging: " << e.what();
    }

    output = mitk::GrabItkImageMemory( filter->GetOutput() );
  }

};

} //end namespace mitk

#include "mitkDWIHeadMotionCorrectionFilter.cpp"


#endif // MITKDWIHEADMOTIONCORRECTIONFILTER_H
