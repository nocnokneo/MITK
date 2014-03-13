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

#ifndef _MITK_STL_VECTOR_CONTAINER_SOURCE_H_
#define _MITK_STL_VECTOR_CONTAINER_SOURCE_H_


#include "mitkSTLVectorContainer.h"
#include "mitkCommon.h"
#include "mitkBaseProcess.h"

namespace mitk
{

template <typename T>
class STLVectorContainerSource : public BaseProcess
{
public:

    mitkClassMacro( STLVectorContainerSource, BaseProcess );

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    typedef mitk::STLVectorContainer<T> OutputType;

    typedef typename OutputType::Pointer OutputTypePointer;

    typedef itk::DataObject::Pointer DataObjectPointer;

    /**
     * Allocates a new output object and returns it. Currently the
     * index idx is not evaluated.
     * @param idx the index of the output for which an object should be created
     * @returns the new object
     */
    virtual DataObjectPointer MakeOutput ( unsigned int idx );

    /**
     * Generates the input requested region simply by calling the equivalent
     * method of the superclass.
     */
    void GenerateInputRequestedRegion();

    /**
     * Allows to set the output of the stl vector container source. According to the itk documentation
     * this method is outdated and should not be used. Instead GraftOutput(...)
     * should be used.
     * @param output the intended output of the lookup table source
     */
    void SetOutput( OutputType* output );

    /**
     * Replacement of the SetOutput method. I think it is not yet correcly
     * implemented, so you should better not use it.
     * @todo provide a more usefule implementation
     * @param output the intended output of the lookup table source.
     */
    virtual void GraftOutput( OutputType* output );

    /**
     * Returns the output with index 0 of the stl vector container source
     * @returns the output
     */
    OutputType* GetOutput();

    /**
     * Returns the n'th output of the stl vector container source
     * @param idx the index of the wanted output
     * @returns the output with index idx.
     */
    OutputType* GetOutput ( unsigned int idx );

protected:
    STLVectorContainerSource();
    virtual ~STLVectorContainerSource();

};

typedef STLVectorContainerSource<int> IntVectorContainerSource;
typedef STLVectorContainerSource<unsigned int> UnsignedIntVectorContainerSource;
typedef STLVectorContainerSource<long> LongVectorContainerSource;
typedef STLVectorContainerSource<unsigned long> UnsignedLongVectorContainerSource;
typedef STLVectorContainerSource<short int> ShortIntVectorContainerSource;
typedef STLVectorContainerSource<short unsigned int> ShortUnsignedIntVectorContainerSource;
typedef STLVectorContainerSource<float> FloatVectorContainerSource;
typedef STLVectorContainerSource<double> DoubleVectorContainerSource;

} // end of namespace mitk

#include "mitkSTLVectorContainerSource.txx"

#endif
