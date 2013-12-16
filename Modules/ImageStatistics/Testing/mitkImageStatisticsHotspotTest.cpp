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

#include "mitkImageStatisticsCalculator.h"
#include "itkMultiGaussianImageSource.h"
#include "mitkTestingMacros.h"

#include <itkImageRegionIterator.h>

#include <stdexcept>

#include <itkDOMNode.h>
#include <itkDOMReader.h>

/**
 \section hotspotCalculationTestCases Testcases

 Every testcase has a defined hotspot, maximum and minimum includig their corresponding index-values and mean value.
 The XML-files to each testcase is located in \ImageStatistics\Testing\Data.

 The following cases describe situations of hotspot-calculation and their supposed results.

 <b>Note:</b> Below only the behaviour of maximum is mentioned mostly, but the other statistics (minimum and mean) behave
    in the same way like maximum.

 <b> Testcase 1: No values outside of hotspot are used for statistic-calculation </b>

  This testcase excludes that pixelvalues are used for the statistic-calculation which are located outside the hotspot.

   Description:
 - Defined location of hotspot in image: left upper corner
 - Defined location of maximum in image: bottom right corner
 - Segmenation is not available

 \image html mitkimagestatisticshotspottestcase1.jpg

   Assumed results:
 - Hotspot is calcualted correctly in the left upper corner of the image
 - Defined maximum is not inside hotspot
 - A maximum inside the hotspot is calculated

 <b> Testcase 2: Correct detection of hotspot </b>

 This testcase exclues that pixelvalues are used for statistics-calculation which are located outside of the segmentation.

   Description:
 - Segmentation is available
 - Defined location of hotspot: inside segmentation
 - Defined location of maximum: inside hotspot
 - Another "hotter" region outside of the segmenation

 \image html mitkimagestatisticshotspottestcase2.jpg

   Assumed results:
 - Defined hotspot is correctly calculated inside segmentation
 - Defined maximum is correctly calculated inside hotspot
 - "Hotter" region outside of segmentation is disregarded

 <b> Testcase 3: Correct calculation of statistics in hotspot, altough the whole hotspot is not inside segmenation </b>

 This testcase excludes that the whole hotspot has to be completly inside the segmentation for statistica-calculation. So it is
 possible to calculate hotspot-statistics even if the region of interest is smaller than the hotspot itself.

   Description:
 - Segmentation is available
 - Defined location of hotspot: inside segmentation
 - Defined location of maximum: outside of segmentation, but inside of hotspot

 \image html mitkimagestatisticshotspottestcase3.jpg

   Assumed results:
 - Defined hotspot is correctly calculated inside segmentation
 - Defined maximum is correctly calculated inside hotspot altough it is located outside of the segmentation

 <b> Testcase 4: Hotspot is not completly inside image </b>

 This testcase confirms that not the whole hotspot has to be inside the image. Only pixelvalues in the hotspot are considered
 which are located inside the image.

   Description:
 - Defined location of hotspot: At the border of the image
 - Defined location of maximum: Inside hotspot
 - Segmenation is not available

 \image html mitkimagestatisticshotspottestcase4.jpg

   Assumed result:
 - Just the part of the hotspot, which is located in the image, is used for statistics-calculation
 - Defined statistics are calculated correctly

 <b> Testcase 5: Hotspot has to be inside image </b>

 This testcase confirms that the whole hotspot has to be completly inside the image. If there is a possible hotspot-location for which
 the whole hotspot would not be completly inside the image, it will be disregarded.

   Description:
 - Defined location of hotspot: At the border of the image
 - Defined location of maximum: Inside hotspot
 - Segmenation is not available

 \image html mitkimagestatisticshotspottestcase5.jpg

   Assumed results:
 - Defined hotspot and statistics are not calculated, because hotspot is not completly inside image
 - A hotspot, which is not as hot as the defined one but is inside the image, is calculated

 <b> Testcase 6: Multilabel mask </b>

 This testcase confirms that mitkImageStatisticsCalculator has the possibility to calculate hotspot statistics even if
 there are multiple regions of interest.

   Description:
 - Two defined regions of interest with defined statistics for each one.

 \image html mitkimagestatisticshotspottestcase6.jpg

   Assumed results:
 - In every region of interest there are correctly calculated hotspot-statistics
 */

struct mitkImageStatisticsHotspotTestClass
{
  /**
    \brief Test parameters for one test case.

    Describes all aspects of a single test case:
     - parameters to generate a test image
     - parameters of a ROI that describes where to calculate statistics
     - expected statistics results
  */
  struct Parameters
  {
  public:

    // XML-Tag <testimage>

    /** \brief XML-Tag "image-rows": size of x-dimension  */
    int m_ImageRows;
    /** \brief  XML-Tag "image-columns": size of y-dimension  */
    int m_ImageColumns;
    /** \brief  XML-Tag "image-slices": size of z-dimension  */
    int m_ImageSlices;
    /** \brief  XML-Tag "numberOfGaussians": number of used gauss-functions */
    int m_NumberOfGaussian;

    /** \brief  XML-Tags "spacingX", "spacingY", "spacingZ": spacing of image in every direction */
    float m_Spacing[3];

    /** \brief XML-Tag "entireHotSpotInROI" */
    unsigned int m_EntireHotspotInROI;

    // XML-Tag <gaussian>

    /** \brief  XML-Tag "centerIndexX: gaussian parameter*/
    std::vector<int> m_CenterX;
    /** \brief  XML-Tag "centerIndexY: gaussian parameter */
    std::vector<int> m_CenterY;
    /** \brief  XML-Tag "centerIndexZ: gaussian parameter */
    std::vector<int> m_CenterZ;

    /** \brief  XML-Tag "deviationX: gaussian parameter  */
    std::vector<int> m_SigmaX;
    /** \brief  XML-Tag "deviationY: gaussian parameter  */
    std::vector<int> m_SigmaY;
    /** \brief  XML-Tag "deviationZ: gaussian parameter  */
    std::vector<int> m_SigmaZ;

    /** \brief  XML-Tag "altitude: gaussian parameter  */
    std::vector<int> m_Altitude;

    // XML-Tag <segmentation>

    /** \brief  XML-Tag "numberOfLabels": number of different labels which appear in the mask */
    unsigned int m_NumberOfLabels;
    /** \brief  XML-Tag "hotspotRadiusInMM": radius of hotspot */
    float m_HotspotRadiusInMM;

    // XML-Tag <roi>

    /** \brief  XML-Tag "maximumSizeX": maximum position of ROI in x-dimension */
    vnl_vector<int> m_MaxSizeX;
    /** \brief  XML-Tag "minimumSizeX": minimum position of ROI in x-dimension */
    vnl_vector<int> m_MinSizeX;
    /** \brief  XML-Tag "maximumSizeX": maximum position of ROI in y-dimension */
    vnl_vector<int> m_MaxSizeY;
    /** \brief  XML-Tag "minimumSizeX": minimum position of ROI in y-dimension */
    vnl_vector<int> m_MinSizeY;
    /** \brief  XML-Tag "maximumSizeX": maximum position of ROI in z-dimension */
    vnl_vector<int> m_MaxSizeZ;
    /** \brief  XML-Tag "minimumSizeX": minimum position of ROI in z-dimension */
    vnl_vector<int> m_MinSizeZ;

    /** \brief  XML-Tag "label": value of label */
    vnl_vector<unsigned int> m_Label;

    //XML-Tag <statistic>

    /** \brief  XML-Tag "minimum": minimum inside hotspot */
    vnl_vector<double> m_HotspotMin;
    /** \brief  XML-Tag "maximum": maximum inside hotspot */
    vnl_vector<double> m_HotspotMax;
    /** \brief  XML-Tag "mean": mean value of hotspot */
    vnl_vector<double> m_HotspotMean;

    /** \brief  XML-Tag "maximumIndexX": x-coordinate of maximum-location inside hotspot */
    vnl_vector<int> m_HotspotMaxIndexX;
    /** \brief  XML-Tag "maximumIndexX": y-coordinate of maximum-location inside hotspot */
    vnl_vector<int> m_HotspotMaxIndexY;
    /** \brief  XML-Tag "maximumIndexX": z-coordinate of maximum-location inside hotspot */
    vnl_vector<int> m_HotspotMaxIndexZ;

    /** \brief  XML-Tag "maximumIndexX": x-coordinate of maximum-location inside hotspot */
    vnl_vector<int> m_HotspotMinIndexX;
    /** \brief  XML-Tag "maximumIndexX": y-coordinate of maximum-location inside hotspot */
    vnl_vector<int> m_HotspotMinIndexY;
    /** \brief  XML-Tag "maximumIndexX": z-coordinate of maximum-location inside hotspot */
    vnl_vector<int> m_HotspotMinIndexZ;

    /** \brief  XML-Tag "maximumIndexX": x-coordinate of hotspot-location */
    vnl_vector<int> m_HotspotIndexX;
    /** \brief  XML-Tag "maximumIndexX": y-coordinate of hotspot-location */
    vnl_vector<int> m_HotspotIndexY;
    /** \brief  XML-Tag "maximumIndexX": z-coordinate of hotspot-location */
    vnl_vector<int> m_HotspotIndexZ;
  };

  /**
    \brief Find/Convert integer attribute in itk::DOMNode.
  */
  static int GetIntegerAttribute(itk::DOMNode* domNode, const std::string& tag)
  {
    assert(domNode);
    MITK_TEST_CONDITION_REQUIRED( domNode->HasAttribute(tag), "Tag '" << tag << "' is defined in test parameters" );
    std::string attributeValue = domNode->GetAttribute(tag);

    int resultValue;
    try
    {
      //MITK_TEST_OUTPUT( << "Converting tag value '" << attributeValue << "' for tag '" << tag << "' to integer");
      std::stringstream(attributeValue) >> resultValue;
      return resultValue;
    }
    catch(std::exception& e)
    {
      MITK_TEST_CONDITION_REQUIRED(false, "Convert tag value '" << attributeValue << "' for tag '" << tag << "' to integer");
      return 0; // just to satisfy compiler
    }
  }
  /**
    \brief Find/Convert double attribute in itk::DOMNode.
  */
  static double GetDoubleAttribute(itk::DOMNode* domNode, const std::string& tag)
  {
    assert(domNode);
    MITK_TEST_CONDITION_REQUIRED( domNode->HasAttribute(tag), "Tag '" << tag << "' is defined in test parameters" );
    std::string attributeValue = domNode->GetAttribute(tag);

    double resultValue;
    try
    {
      //MITK_TEST_OUTPUT( << "Converting tag value '" << attributeValue << "' for tag '" << tag << "' to double");
      std::stringstream(attributeValue) >> resultValue;
      return resultValue;
    }
    catch(std::exception& e)
    {
      MITK_TEST_CONDITION_REQUIRED(false, "Convert tag value '" << attributeValue << "' for tag '" << tag << "' to double");
      return 0.0; // just to satisfy compiler
    }
  }

  /**
  \brief Read XML file describing the test parameters.

  Reads XML file given in first commandline parameter in order
  to construct a Parameters structure. The XML file should be
  structurs as the following example, i.e. we describe the
  three test aspects of Parameters in four different tags,
  with all the details described as tag attributes. */

  /**
  \verbatim
  <testcase>
  <!--
  Test case: multi-label mask
  -->

  <testimage image-rows="50" image-columns="50" image-slices="20" numberOfGaussians="2" spacingX="1" spacingY="1" spacingZ="1" entireHotSpotInROI="1">
    <gaussian centerIndexX="10" centerIndexY="10" centerIndexZ="10" deviationX="5" deviationY="5" deviationZ="5" altitude="200"/>
    <gaussian centerIndexX="40" centerIndexY="40" centerIndexZ="10" deviationX="2" deviationY="4" deviationZ="6" altitude="180"/>
  </testimage>
  <segmentation numberOfLabels="2" hotspotRadiusInMM="6.2035">
    <roi label="1" maximumSizeX="20" minimumSizeX="0" maximumSizeY="20" minimumSizeY="0" maximumSizeZ="20" minimumSizeZ="0"/>
    <roi label="2" maximumSizeX="50" minimumSizeX="30" maximumSizeY="50" minimumSizeY="30" maximumSizeZ="20" minimumSizeZ="0"/>
  </segmentation>
  <statistic hotspotIndexX="10" hotspotIndexY="10" hotspotIndexZ="10" mean="122.053" maximumIndexX="10" maximumIndexY="10" maximumIndexZ="10" maximum="200" minimumIndexX="9" minimumIndexY="9" minimumIndexZ="4" minimum="93.5333"/>
  <statistic hotspotIndexX="40" hotspotIndexY="40" hotspotIndexZ="10" mean="61.1749" maximumIndexX="40" maximumIndexY="40" maximumIndexZ="10" maximum="180" minimumIndexX="46" minimumIndexY="39" minimumIndexZ="9" minimum="1.91137"/>
 </testcase>

  \endverbatim
  */
  static Parameters ParseParameters(int argc, char* argv[])
  {
    // - parse parameters
    // - fill ALL values of result structure
    // - if necessary, provide c'tor and default parameters to Parameters

    MITK_TEST_CONDITION_REQUIRED(argc == 2, "Test is invoked with exactly 1 parameter (XML parameters file)");
    MITK_INFO << "Reading parameters from file '" << argv[1] << "'";
    std::string filename = argv[1];

    Parameters result;

    itk::DOMNodeXMLReader::Pointer xmlReader = itk::DOMNodeXMLReader::New();
    xmlReader->SetFileName( filename );
    try
    {
      xmlReader->Update();
      itk::DOMNode::Pointer domRoot = xmlReader->GetOutput();
      typedef std::vector<itk::DOMNode*> NodeList;
      // read test image parameters, fill result structure
      NodeList testimages;
      domRoot->GetChildren("testimage", testimages);
      MITK_TEST_CONDITION_REQUIRED( testimages.size() == 1, "One test image defined" )
      itk::DOMNode* testimage = testimages[0];

      result.m_ImageRows = GetIntegerAttribute( testimage, "image-rows" );
      result.m_ImageColumns = GetIntegerAttribute( testimage, "image-columns" );
      result.m_ImageSlices = GetIntegerAttribute( testimage, "image-slices" );

      result.m_NumberOfGaussian = GetIntegerAttribute( testimage, "numberOfGaussians" );

      result.m_Spacing[0] = GetDoubleAttribute(testimage, "spacingX");
      result.m_Spacing[1] = GetDoubleAttribute(testimage, "spacingY");
      result.m_Spacing[2] = GetDoubleAttribute(testimage, "spacingZ");

      result.m_EntireHotspotInROI = GetIntegerAttribute( testimage, "entireHotSpotInROI" );

      MITK_TEST_OUTPUT( << "Read size parameters (x,y,z): " << result.m_ImageRows << "," << result.m_ImageColumns << "," << result.m_ImageSlices);
      MITK_TEST_OUTPUT( << "Read spacing parameters (x,y,z): " << result.m_Spacing[0] << "," << result.m_Spacing[1] << "," << result.m_Spacing[2]);

      NodeList gaussians;
      testimage->GetChildren("gaussian", gaussians);
      MITK_TEST_CONDITION_REQUIRED( gaussians.size() >= 1, "At least one gaussian is defined" )

      result.m_CenterX.resize(result.m_NumberOfGaussian);
      result.m_CenterY.resize(result.m_NumberOfGaussian);
      result.m_CenterZ.resize(result.m_NumberOfGaussian);

      result.m_SigmaX.resize(result.m_NumberOfGaussian);
      result.m_SigmaY.resize(result.m_NumberOfGaussian);
      result.m_SigmaZ.resize(result.m_NumberOfGaussian);

      result.m_Altitude.resize(result.m_NumberOfGaussian);


      for(int i = 0; i < result.m_NumberOfGaussian ; ++i)
      {
        itk::DOMNode* gaussian = gaussians[i];

        result.m_CenterX[i] = GetIntegerAttribute(gaussian, "centerIndexX");
        result.m_CenterY[i] = GetIntegerAttribute(gaussian, "centerIndexY");
        result.m_CenterZ[i] = GetIntegerAttribute(gaussian, "centerIndexZ");

        result.m_SigmaX[i] = GetIntegerAttribute(gaussian, "deviationX");
        result.m_SigmaY[i] = GetIntegerAttribute(gaussian, "deviationY");
        result.m_SigmaZ[i] = GetIntegerAttribute(gaussian, "deviationZ");

        result.m_Altitude[i] = GetIntegerAttribute(gaussian, "altitude");
      }

      NodeList segmentations;
      domRoot->GetChildren("segmentation", segmentations);
      MITK_TEST_CONDITION_REQUIRED( segmentations.size() == 1, "One segmentation defined");
      itk::DOMNode* segmentation = segmentations[0];

      result.m_NumberOfLabels = GetIntegerAttribute(segmentation, "numberOfLabels");
      result.m_HotspotRadiusInMM = GetDoubleAttribute(segmentation, "hotspotRadiusInMM");


      // read ROI parameters, fill result structure
      NodeList rois;
      segmentation->GetChildren("roi", rois);
      MITK_TEST_CONDITION_REQUIRED( rois.size() >= 1, "At least one ROI defined" )

      result.m_MaxSizeX.set_size(result.m_NumberOfLabels);
      result.m_MinSizeX.set_size(result.m_NumberOfLabels);
      result.m_MaxSizeY.set_size(result.m_NumberOfLabels);
      result.m_MinSizeY.set_size(result.m_NumberOfLabels);
      result.m_MaxSizeZ.set_size(result.m_NumberOfLabels);
      result.m_MinSizeZ.set_size(result.m_NumberOfLabels);
      result.m_Label.set_size(result.m_NumberOfLabels);

      for(int i = 0; i < rois.size(); ++i)
      {
        result.m_MaxSizeX[i] = GetIntegerAttribute(rois[i], "maximumSizeX");
        result.m_MinSizeX[i] = GetIntegerAttribute(rois[i], "minimumSizeX");
        result.m_MaxSizeY[i] = GetIntegerAttribute(rois[i], "maximumSizeY");
        result.m_MinSizeY[i] = GetIntegerAttribute(rois[i], "minimumSizeY");
        result.m_MaxSizeZ[i] = GetIntegerAttribute(rois[i], "maximumSizeZ");
        result.m_MinSizeZ[i] = GetIntegerAttribute(rois[i], "minimumSizeZ");

        result.m_Label[i] = GetIntegerAttribute(rois[i], "label");
      }

      // read statistic parameters, fill result structure
      NodeList statistics;
      domRoot->GetChildren("statistic", statistics);
      MITK_TEST_CONDITION_REQUIRED( statistics.size() == rois.size(), "Same number of rois and corresponding statistics defined");
      MITK_TEST_CONDITION_REQUIRED( statistics.size() >= 1 , "At least one statistic defined" )

      result.m_HotspotMin.set_size(statistics.size());
      result.m_HotspotMax.set_size(statistics.size());
      result.m_HotspotMean.set_size(statistics.size());

      result.m_HotspotMinIndexX.set_size(statistics.size());
      result.m_HotspotMinIndexY.set_size(statistics.size());
      result.m_HotspotMinIndexZ.set_size(statistics.size());

      result.m_HotspotMaxIndexX.set_size(statistics.size());
      result.m_HotspotMaxIndexY.set_size(statistics.size());
      result.m_HotspotMaxIndexZ.set_size(statistics.size());

      result.m_HotspotIndexX.set_size(statistics.size());
      result.m_HotspotIndexY.set_size(statistics.size());
      result.m_HotspotIndexZ.set_size(statistics.size());

      for(int i = 0; i < statistics.size(); ++i)
      {
        result.m_HotspotMin[i] = GetDoubleAttribute(statistics[i], "minimum");
        result.m_HotspotMax[i] = GetDoubleAttribute(statistics[i], "maximum");
        result.m_HotspotMean[i] = GetDoubleAttribute(statistics[i], "mean");

        result.m_HotspotMinIndexX[i] = GetIntegerAttribute(statistics[i], "minimumIndexX");
        result.m_HotspotMinIndexY[i] = GetIntegerAttribute(statistics[i], "minimumIndexY");
        result.m_HotspotMinIndexZ[i] = GetIntegerAttribute(statistics[i], "minimumIndexZ");

        result.m_HotspotMaxIndexX[i] = GetIntegerAttribute(statistics[i], "maximumIndexX");
        result.m_HotspotMaxIndexY[i] = GetIntegerAttribute(statistics[i], "maximumIndexY");
        result.m_HotspotMaxIndexZ[i] = GetIntegerAttribute(statistics[i], "maximumIndexZ");

        result.m_HotspotIndexX[i] = GetIntegerAttribute(statistics[i], "hotspotIndexX");
        result.m_HotspotIndexY[i] = GetIntegerAttribute(statistics[i], "hotspotIndexY");
        result.m_HotspotIndexZ[i] = GetIntegerAttribute(statistics[i], "hotspotIndexZ");
      }

      return result;
    }
    catch (std::exception& e)
    {
      MITK_TEST_CONDITION_REQUIRED(false, "Reading test parameters from XML file. Error message: " << e.what());
    }
  }

  /**
    \brief Generate an image that contains a couple of 3D gaussian distributions.

    Uses the given parameters to produce a test image using class MultiGaussianImageSource.
  */

  static mitk::Image::Pointer BuildTestImage(const Parameters& testParameters)
  {
    // evaluate parameters, create corresponding image
    mitk::Image::Pointer result;

    typedef double PixelType;
    const unsigned int Dimension = 3;
    typedef itk::Image<PixelType, Dimension> ImageType;
    ImageType::Pointer image = ImageType::New();
    typedef itk::MultiGaussianImageSource< ImageType > MultiGaussianImageSource;
    MultiGaussianImageSource::Pointer gaussianGenerator = MultiGaussianImageSource::New();
    ImageType::SizeValueType size[3];
    size[0] = testParameters.m_ImageColumns;
    size[1] = testParameters.m_ImageRows;
    size[2] = testParameters.m_ImageSlices;

    itk::MultiGaussianImageSource<ImageType>::VectorType centerXVec, centerYVec, centerZVec, sigmaXVec, sigmaYVec, sigmaZVec, altitudeVec;

    for(int i = 0; i < testParameters.m_NumberOfGaussian; ++i)
    {
      centerXVec.push_back(testParameters.m_CenterX[i]);
      centerYVec.push_back(testParameters.m_CenterY[i]);
      centerZVec.push_back(testParameters.m_CenterZ[i]);

      sigmaXVec.push_back(testParameters.m_SigmaX[i]);
      sigmaYVec.push_back(testParameters.m_SigmaY[i]);
      sigmaZVec.push_back(testParameters.m_SigmaZ[i]);

      altitudeVec.push_back(testParameters.m_Altitude[i]);
    }

    ImageType::SpacingType spacing;

    for(int i = 0; i < Dimension; ++i)
      spacing[i] = testParameters.m_Spacing[i];

    gaussianGenerator->SetSize( size );
    gaussianGenerator->SetSpacing( spacing );
    gaussianGenerator->SetRadiusStepNumber(5);
    gaussianGenerator->SetRadius(testParameters.m_HotspotRadiusInMM);
    gaussianGenerator->SetNumberOfGausssians(testParameters.m_NumberOfGaussian);

    gaussianGenerator->AddGaussian(centerXVec, centerYVec, centerZVec,
      sigmaXVec, sigmaYVec, sigmaZVec, altitudeVec);

    gaussianGenerator->Update();

    image = gaussianGenerator->GetOutput();

    mitk::CastToMitkImage(image, result);

    return result;
  }

  /**
    \brief Calculates hotspot statistics for given test image and ROI parameters.

    Uses ImageStatisticsCalculator to find a hotspot in a defined ROI within the given image.
  */
  static mitk::ImageStatisticsCalculator::Statistics CalculateStatistics(mitk::Image* image, const Parameters& testParameters,  unsigned int label)
  {
    mitk::ImageStatisticsCalculator::Statistics result;
    const unsigned int Dimension = 3;
    typedef itk::Image<unsigned short, Dimension> MaskImageType;
    MaskImageType::Pointer mask = MaskImageType::New();

    MaskImageType::SizeType size;
    MaskImageType::SpacingType spacing;
    MaskImageType::IndexType start;

    mitk::ImageStatisticsCalculator::Pointer statisticsCalculator = mitk::ImageStatisticsCalculator::New();
    statisticsCalculator->SetImage(image);
    mitk::Image::Pointer mitkMaskImage;

    if((testParameters.m_MaxSizeX[label] > testParameters.m_MinSizeX[label] && testParameters.m_MinSizeX[label] >= 0) &&
      (testParameters.m_MaxSizeY[label] > testParameters.m_MinSizeY[label] && testParameters.m_MinSizeY[label] >= 0) &&
      (testParameters.m_MaxSizeZ[label] > testParameters.m_MinSizeZ[label] && testParameters.m_MinSizeZ[label] >= 0))
    {
      for(int i = 0; i < Dimension; ++i)
      {
        start[i] = 0;
        spacing[i] = testParameters.m_Spacing[i];
      }

      size[0] = testParameters.m_ImageColumns;
      size[1] = testParameters.m_ImageRows;
      size[2] = testParameters.m_ImageSlices;

      MaskImageType::RegionType region;
      region.SetIndex(start);
      region.SetSize(size);

      mask->SetSpacing(spacing);
      mask->SetRegions(region);
      mask->Allocate();

      typedef itk::ImageRegionIteratorWithIndex<MaskImageType> MaskImageIteratorType;
      MaskImageIteratorType maskIt(mask, region);

      for(maskIt.GoToBegin(); !maskIt.IsAtEnd(); ++maskIt)
      {
        maskIt.Set(0);
      }

      for(int i = 0; i < testParameters.m_NumberOfLabels; ++i)
      {

        for(maskIt.GoToBegin(); !maskIt.IsAtEnd(); ++maskIt)
        {
          MaskImageType::IndexType index = maskIt.GetIndex();

          if((index[0] >= testParameters.m_MinSizeX[i] && index[0] <= testParameters.m_MaxSizeX[i]) &&
            (index[1] >= testParameters.m_MinSizeY[i] && index[1] <= testParameters.m_MaxSizeY[i]) &&
            (index[2] >= testParameters.m_MinSizeZ[i] && index[2] <= testParameters.m_MaxSizeZ[i]))
          {
            maskIt.Set(testParameters.m_Label[i]);
          }
        }
      }

      MITK_INFO << "Masking mode has set to image";
      mitk::CastToMitkImage(mask, mitkMaskImage);
      statisticsCalculator->SetImageMask(mitkMaskImage);
      statisticsCalculator->SetMaskingModeToImage();
    }
    else
    {
      MITK_INFO << "Masking mode has set to none";
      statisticsCalculator->SetMaskingModeToNone();
    }

    statisticsCalculator->SetHotspotRadiusInMM(testParameters.m_HotspotRadiusInMM);
    statisticsCalculator->SetCalculateHotspot(true);

    if(testParameters.m_EntireHotspotInROI == 1)
    {
      MITK_INFO << "Hotspot must be completly inside image";
      statisticsCalculator->SetHotspotMustBeCompletlyInsideImage(true);
    }
    else
    {
      MITK_INFO << "Hotspot must not be completly inside image";
      statisticsCalculator->SetHotspotMustBeCompletlyInsideImage(false);
    }

    statisticsCalculator->ComputeStatistics();
    result = statisticsCalculator->GetStatistics(0, label);

    // create calculator object
    // fill parameters (mask, planar figure, etc.)
    // execute calculation
    // retrieve result and return from function
    // handle errors w/o crash!

    return result;
  }

  /**
    \brief Compares calculated against actual statistics values.

    Checks validness of all statistics aspects. Lets test fail if any aspect is not sufficiently equal.
  */
  static void ValidateStatistics(const mitk::ImageStatisticsCalculator::Statistics& statistics, const Parameters& testParameters, unsigned int label)
  {
    // check all expected test result against actual results

    double eps = 0.001;

    // float comparisons, allow tiny differences
    MITK_TEST_CONDITION( ::fabs(testParameters.m_HotspotMean[label] - statistics.GetHotspotStatistics().GetMean() ) < eps, "Mean value of hotspot in XML-File: " << testParameters.m_HotspotMean[label] << " (Mean value of hotspot calculated in mitkImageStatisticsCalculator: " << statistics.GetHotspotStatistics().GetMean() << ")" );
    MITK_TEST_CONDITION( ::fabs(testParameters.m_HotspotMax[label]- statistics.GetHotspotStatistics().GetMax() ) < eps, "Maximum of hotspot in XML-File:  " << testParameters.m_HotspotMax[label] << " (Maximum of hotspot calculated in mitkImageStatisticsCalculator: "  << statistics.GetHotspotStatistics().GetMax() << ")" );
    MITK_TEST_CONDITION( ::fabs(testParameters.m_HotspotMin[label] - statistics.GetHotspotStatistics().GetMin() ) < eps, "Minimum of hotspot in XML-File: " << testParameters.m_HotspotMin[label] << " (Minimum of hotspot calculated in mitkImageStatisticsCalculator: " << statistics.GetHotspotStatistics().GetMin() << ")" );

     MITK_TEST_CONDITION( statistics.GetHotspotStatistics().GetHotspotIndex()[0] == testParameters.m_HotspotIndexX[label] &&
      statistics.GetHotspotStatistics().GetHotspotIndex()[1] == testParameters.m_HotspotIndexY[label] &&
      statistics.GetHotspotStatistics().GetHotspotIndex()[2] == testParameters.m_HotspotIndexZ[label] ,
      "Index of hotspot in XML-File: " << testParameters.m_HotspotIndexX[label] << " " << testParameters.m_HotspotIndexY[label] << " " << testParameters.m_HotspotIndexZ[label]
    << " (Index of hotspot calculated in mitkImageStatisticsCalculator: " << statistics.GetHotspotStatistics().GetHotspotIndex() << ")" );

    MITK_TEST_CONDITION( statistics.GetHotspotStatistics().GetMaxIndex()[0] == testParameters.m_HotspotMaxIndexX[label] &&
      statistics.GetHotspotStatistics().GetMaxIndex()[1] == testParameters.m_HotspotMaxIndexY[label] &&
      statistics.GetHotspotStatistics().GetMaxIndex()[2] == testParameters.m_HotspotMaxIndexZ[label] ,
      "Index of hotspot in XML-File: " << testParameters.m_HotspotIndexX[label] << " " << testParameters.m_HotspotIndexY[label] << " " << testParameters.m_HotspotMaxIndexZ[label]
    << " (Index of hotspot calculated in mitkImageStatisticsCalculator: " << statistics.GetHotspotStatistics().GetMaxIndex() << ")" );

    MITK_TEST_CONDITION( statistics.GetHotspotStatistics().GetMinIndex()[0] == testParameters.m_HotspotMinIndexX[label] &&
      statistics.GetHotspotStatistics().GetMinIndex()[1] == testParameters.m_HotspotMinIndexY[label] &&
      statistics.GetHotspotStatistics().GetMinIndex()[2] == testParameters.m_HotspotMinIndexZ[label] ,
      "Index of hotspot in XML-File: " << testParameters.m_HotspotMinIndexX[label] << " " << testParameters.m_HotspotMinIndexY[label] << " " << testParameters.m_HotspotMinIndexZ[label]
    << " (Index of hotspot calculated in mitkImageStatisticsCalculator: " << statistics.GetHotspotStatistics().GetMinIndex() << ")" );

  }
};




/**
  \brief Verifies that hotspot statistics part of ImageStatisticsCalculator.

  The test reads parameters from an XML-file to generate a test-image, calculates the hotspot statistics of the image
  and checks if the calculated statistics are the same as the specified values of the XML-file.
*/
int mitkImageStatisticsHotspotTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkImageStatisticsHotspotTest")
  try {
  // parse commandline parameters (see CMakeLists.txt)
  mitkImageStatisticsHotspotTestClass::Parameters parameters = mitkImageStatisticsHotspotTestClass::ParseParameters(argc,argv);

  // build a test image as described in parameters
  mitk::Image::Pointer image = mitkImageStatisticsHotspotTestClass::BuildTestImage(parameters);
  MITK_TEST_CONDITION_REQUIRED( image.IsNotNull(), "Generate test image" );

  for(int label = 0; label < parameters.m_NumberOfLabels; ++label)
  {
    // calculate statistics for this image (potentially use parameters for statistics ROI)
    mitk::ImageStatisticsCalculator::Statistics statistics = mitkImageStatisticsHotspotTestClass::CalculateStatistics(image, parameters, label);

    // compare statistics against stored expected values
    mitkImageStatisticsHotspotTestClass::ValidateStatistics(statistics, parameters, label);
    std::cout << std::endl;
  }


  }
  catch (std::exception& e)
  {
    std::cout << "Error: " <<  e.what() << std::endl;
  }


  MITK_TEST_END()
}
