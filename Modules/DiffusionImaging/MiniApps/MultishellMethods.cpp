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

#include "MiniAppManager.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkExceptionObject.h>
#include <itkImageFileWriter.h>
#include <itkMetaDataObject.h>
#include <itkVectorImage.h>
#include <itkResampleImageFilter.h>

#include <mitkBaseDataIOFactory.h>
#include <mitkDiffusionImage.h>
#include <mitkQBallImage.h>
#include <mitkBaseData.h>
#include <mitkFiberBundleX.h>
#include "ctkCommandLineParser.h"
#include <boost/lexical_cast.hpp>

#include <itkRadialMultishellToSingleshellImageFilter.h>
#include <itkADCAverageFunctor.h>
#include <itkBiExpFitFunctor.h>
#include <itkKurtosisFitFunctor.h>
#include <mitkNrrdDiffusionImageWriter.h>
#include <itkDwiGradientLengthCorrectionFilter.h>

int MultishellMethods(int argc, char* argv[])
{
  ctkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  parser.addArgument("in", "i", ctkCommandLineParser::String, "input file", us::Any(), false);
  parser.addArgument("out", "o", ctkCommandLineParser::String, "output file", us::Any(), false);
  parser.addArgument("adc", "D", ctkCommandLineParser::Bool, "ADC Average", us::Any(), false);
  parser.addArgument("akc", "K", ctkCommandLineParser::Bool, "Kurtosis Fit", us::Any(), false);
  parser.addArgument("biexp", "B", ctkCommandLineParser::Bool, "BiExp fit", us::Any(), false);
  parser.addArgument("targetbvalue", "b", ctkCommandLineParser::String, "target bValue (mean, min, max)", us::Any(), false);

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  // mandatory arguments
  string inName = us::any_cast<string>(parsedArgs["in"]);
  string outName = us::any_cast<string>(parsedArgs["out"]);
  bool applyADC = us::any_cast<bool>(parsedArgs["adc"]);
  bool applyAKC = us::any_cast<bool>(parsedArgs["akc"]);
  bool applyBiExp = us::any_cast<bool>(parsedArgs["biexp"]);
  string targetType = us::any_cast<string>(parsedArgs["targetbvalue"]);

  try
  {
    MITK_INFO << "Loading " << inName;
    const std::string s1="", s2="";
    std::vector<mitk::BaseData::Pointer> infile = mitk::BaseDataIO::LoadBaseDataFromFile( inName, s1, s2, false );
    mitk::BaseData::Pointer baseData = infile.at(0);

    if ( dynamic_cast<mitk::DiffusionImage<short>*>(baseData.GetPointer()) )
    {
      MITK_INFO << "Writing " << outName;
      mitk::DiffusionImage<short>::Pointer dwi = dynamic_cast<mitk::DiffusionImage<short>*>(baseData.GetPointer());
      typedef itk::RadialMultishellToSingleshellImageFilter<short, short> FilterType;

      typedef itk::DwiGradientLengthCorrectionFilter  CorrectionFilterType;

      CorrectionFilterType::Pointer roundfilter = CorrectionFilterType::New();
      roundfilter->SetRoundingValue( 1000 );
      roundfilter->SetReferenceBValue(dwi->GetReferenceBValue());
      roundfilter->SetReferenceGradientDirectionContainer(dwi->GetDirections());
      roundfilter->Update();

      dwi->SetReferenceBValue( roundfilter->GetNewBValue() );
      dwi->SetDirections( roundfilter->GetOutputGradientDirectionContainer());

      // filter input parameter
      const mitk::DiffusionImage<short>::BValueMap
          &originalShellMap  = dwi->GetBValueMap();

      const mitk::DiffusionImage<short>::ImageType
          *vectorImage       = dwi->GetVectorImage();

      const mitk::DiffusionImage<short>::GradientDirectionContainerType::Pointer
          gradientContainer = dwi->GetDirections();

      const unsigned int
          &bValue            = dwi->GetReferenceBValue();

      // filter call


      vnl_vector<double> bValueList(originalShellMap.size()-1);
      double targetBValue = bValueList.mean();

      mitk::DiffusionImage<short>::BValueMap::const_iterator it = originalShellMap.begin();
      ++it; int i = 0 ;
      for(; it != originalShellMap.end(); ++it)
        bValueList.put(i++,it->first);

      if( targetType == "mean" )
        targetBValue = bValueList.mean();
      else if( targetType == "min" )
        targetBValue = bValueList.min_value();
      else if( targetType == "max" )
        targetBValue = bValueList.max_value();

      if(applyADC)
      {
        FilterType::Pointer filter = FilterType::New();
        filter->SetInput(vectorImage);
        filter->SetOriginalGradientDirections(gradientContainer);
        filter->SetOriginalBValueMap(originalShellMap);
        filter->SetOriginalBValue(bValue);

        itk::ADCAverageFunctor::Pointer functor = itk::ADCAverageFunctor::New();
        functor->setListOfBValues(bValueList);
        functor->setTargetBValue(targetBValue);

        filter->SetFunctor(functor);
        filter->Update();
        // create new DWI image
        mitk::DiffusionImage<short>::Pointer outImage = mitk::DiffusionImage<short>::New();
        outImage->SetVectorImage( filter->GetOutput() );
        outImage->SetReferenceBValue( targetBValue );
        outImage->SetDirections( filter->GetTargetGradientDirections() );
        outImage->InitializeFromVectorImage();

        mitk::NrrdDiffusionImageWriter<short>::Pointer writer = mitk::NrrdDiffusionImageWriter<short>::New();
        writer->SetFileName((string(outName) + "_ADC.dwi"));
        writer->SetInput(outImage);
        writer->Update();
      }
      if(applyAKC)
      {
        FilterType::Pointer filter = FilterType::New();
        filter->SetInput(vectorImage);
        filter->SetOriginalGradientDirections(gradientContainer);
        filter->SetOriginalBValueMap(originalShellMap);
        filter->SetOriginalBValue(bValue);

        itk::KurtosisFitFunctor::Pointer functor = itk::KurtosisFitFunctor::New();
        functor->setListOfBValues(bValueList);
        functor->setTargetBValue(targetBValue);

        filter->SetFunctor(functor);
        filter->Update();
        // create new DWI image
        mitk::DiffusionImage<short>::Pointer outImage = mitk::DiffusionImage<short>::New();
        outImage->SetVectorImage( filter->GetOutput() );
        outImage->SetReferenceBValue( targetBValue );
        outImage->SetDirections( filter->GetTargetGradientDirections() );
        outImage->InitializeFromVectorImage();

        mitk::NrrdDiffusionImageWriter<short>::Pointer writer = mitk::NrrdDiffusionImageWriter<short>::New();
        writer->SetFileName((string(outName) + "_AKC.dwi"));
        writer->SetInput(outImage);
        writer->Update();
      }
      if(applyBiExp)
      {
        FilterType::Pointer filter = FilterType::New();
        filter->SetInput(vectorImage);
        filter->SetOriginalGradientDirections(gradientContainer);
        filter->SetOriginalBValueMap(originalShellMap);
        filter->SetOriginalBValue(bValue);

        itk::BiExpFitFunctor::Pointer functor = itk::BiExpFitFunctor::New();
        functor->setListOfBValues(bValueList);
        functor->setTargetBValue(targetBValue);

        filter->SetFunctor(functor);
        filter->Update();
        // create new DWI image
        mitk::DiffusionImage<short>::Pointer outImage = mitk::DiffusionImage<short>::New();
        outImage->SetVectorImage( filter->GetOutput() );
        outImage->SetReferenceBValue( targetBValue );
        outImage->SetDirections( filter->GetTargetGradientDirections() );
        outImage->InitializeFromVectorImage();

        mitk::NrrdDiffusionImageWriter<short>::Pointer writer = mitk::NrrdDiffusionImageWriter<short>::New();
        writer->SetFileName((string(outName) + "_BiExp.dwi"));
        writer->SetInput(outImage);
        writer->Update();
      }
    }
  }
  catch (itk::ExceptionObject e)
  {
    MITK_INFO << e;
    return EXIT_FAILURE;
  }
  catch (std::exception e)
  {
    MITK_INFO << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    MITK_INFO << "ERROR!?!";
    return EXIT_FAILURE;
  }
  MITK_INFO << "DONE";
  return EXIT_SUCCESS;
}
RegisterDiffusionMiniApp(MultishellMethods);
