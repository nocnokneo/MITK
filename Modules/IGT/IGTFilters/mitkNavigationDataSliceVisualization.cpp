#include "mitkNavigationDataSliceVisualization.h"

#include "mitkBaseRenderer.h"
#include "mitkPropertyList.h"
#include "mitkProperties.h"
#include "mitkWeakPointerProperty.h"

#include <itkBoxSpatialObject.h>
#include <itkEllipseSpatialObject.h>
#include <itkNumericTraits.h>

mitk::NavigationDataSliceVisualization::NavigationDataSliceVisualization() : mitk::NavigationDataToNavigationDataFilter(),
  m_Renderer(NULL),
  m_SliceOrientation(SLICE_ORTHO),
  m_TrackingVolume(NULL),
  m_JustExceededTrackingVolume(true),
  m_FirstUpdate(true),
  m_LastUserSelectedSlice(0)
{
  m_TipOffset[0] = 0.0f;
  m_TipOffset[1] = 0.0f;
  m_TipOffset[2] = 0.0f;

  m_DirectionOfProjection[0] = 0;
  m_DirectionOfProjection[1] = 0;
  m_DirectionOfProjection[2] = -1;

  m_LastUserSelectedSliceAxes[0][0] = 1.0;
  m_LastUserSelectedSliceAxes[0][1] = 0.0;
  m_LastUserSelectedSliceAxes[0][2] = 0.0;

  m_LastUserSelectedSliceAxes[1][0] = 0.0;
  m_LastUserSelectedSliceAxes[1][1] = 1.0;
  m_LastUserSelectedSliceAxes[1][2] = 0.0;
}

void mitk::NavigationDataSliceVisualization::GenerateData()
{
  // check if renderer was set
  if (m_Renderer.IsNull())
  {
    itkExceptionMacro(<< "Renderer was not properly set");
  }

  if (m_FirstUpdate)
  {
    m_FirstUpdate = false;
    this->SaveLastUserSelectedSlice();
  }

  /* update outputs with tracking data from tools */
  unsigned int numberOfInputs = this->GetNumberOfInputs();
  for (unsigned int i = 0; i < numberOfInputs ; ++i)
  {
    NavigationData* output = this->GetOutput(i);
    assert(output);
    const NavigationData* input = this->GetInput(i);
    assert(input);

    if (!input->IsDataValid())
      continue;

    output->Graft(input); // First, copy all information from input to output
  }

  // Nothing left to do if data is not valid
  if (!this->GetInput()->IsDataValid())
    return;

  // get position from NavigationData to move the slice to this position
  Point3D slicePosition = this->GetInput()->GetPosition();

  // Check if we are within range
  if (m_TrackingVolume.IsNull() || m_TrackingVolume->IsInside(slicePosition))
  {
    if (m_JustExceededTrackingVolume == false)
    {
      // We're in tracking range now, but the next time we exceed tracking
      // range this should be true
      m_JustExceededTrackingVolume = true;
      this->SaveLastUserSelectedSlice();
    }

    NavigationData::OrientationType orientation = this->GetInput()->GetOrientation();

    vnl_vector<ScalarType> transformedTipOffsetVnl = orientation.rotate(m_TipOffset.GetVnlVector());
    Vector3D transformedTipOffset;
    transformedTipOffset.SetVnlVector(transformedTipOffsetVnl);

    if (SLICE_ORTHO == m_SliceOrientation)
    {
      m_Renderer->GetSliceNavigationController()->SelectSliceByPoint(slicePosition + transformedTipOffset);
    }
    else if (SLICE_NORMAL_TO_TOOL_TIP == m_SliceOrientation)
    {
      vnl_vector<ScalarType> directionOfProjectionVnl = orientation.rotate(m_DirectionOfProjection.GetVnlVector());
      Vector3D directionOfProjection;
      directionOfProjection.SetVnlVector(directionOfProjectionVnl);

      // plane_z X user_x -> plane_y
      Vector3D normalPlaneYAxis = itk::CrossProduct(directionOfProjection, m_LastUserSelectedSliceAxes[0]);
      // plane_y X plane_z -> plane_x
      Vector3D normalPlaneXAxis = itk::CrossProduct(normalPlaneYAxis, directionOfProjection);

      m_Renderer->GetSliceNavigationController()->ReorientSlices(slicePosition + transformedTipOffset,
                                                                 normalPlaneXAxis, normalPlaneYAxis);
    }
    else
    {
      MITK_ERROR << "Unsupported SliceOrientation: " << m_SliceOrientation;
    }

    m_Renderer->RequestUpdate();
  }
  else if (m_JustExceededTrackingVolume == true)
  {
    m_JustExceededTrackingVolume = false;
    this->RestoreLastUserSelectedSlice();
  }
}

void
mitk::NavigationDataSliceVisualization::SaveLastUserSelectedSlice()
{
  m_LastUserSelectedSlice = m_Renderer->GetSliceNavigationController()->GetSlice()->GetPos();
  TimeSlicedGeometry::ConstPointer userWorldGeometry = m_Renderer->GetSliceNavigationController()->GetCreatedWorldGeometry();
  m_LastUserSelectedSliceAxes[0] = userWorldGeometry->GetAxisVector(0);
  m_LastUserSelectedSliceAxes[1] = userWorldGeometry->GetAxisVector(1);
}

void
mitk::NavigationDataSliceVisualization::RestoreLastUserSelectedSlice()
{
  Point3D dummyPoint;
  dummyPoint.Fill(0.0);
  m_Renderer->GetSliceNavigationController()->ReorientSlices(dummyPoint, m_LastUserSelectedSliceAxes[0], m_LastUserSelectedSliceAxes[1]);
  m_Renderer->GetSliceNavigationController()->GetSlice()->SetPos(m_LastUserSelectedSlice);
  m_Renderer->GetSliceNavigationController()->Modified();
  m_Renderer->RequestUpdate();
}

void
mitk::NavigationDataSliceVisualization::SetSphereTrackingVolume(
  Point3D origin,
  double radius)
{
  double sphereRadius[3] = {radius, radius, radius };
  this->SetEllipseTrackingVolume(origin, sphereRadius);
}

void
mitk::NavigationDataSliceVisualization::SetEllipseTrackingVolume(
  Point3D origin,
  double radius[3])
{
  typedef itk::EllipseSpatialObject<3> EllipseSpatialObjectType;
  EllipseSpatialObjectType::Pointer ellipse = EllipseSpatialObjectType::New();
  ellipse->SetRadius(radius);

  // Shift the origin
  typedef EllipseSpatialObjectType::TransformType                 TransformType;
  TransformType::Pointer transform = TransformType::New();
  transform->SetIdentity();
  TransformType::OutputVectorType  translation;
  translation[0] = origin[0];
  translation[1] = origin[1];
  translation[2] = origin[2];
  transform->Translate(translation, false);
  ellipse->SetObjectToParentTransform( transform );

  this->SetTrackingVolume(ellipse);
}

void
mitk::NavigationDataSliceVisualization::SetBoxTrackingVolumeVolume(Point3D origin, double size)
{
  double uniformSize[3] = {size, size, size};
  this->SetBoxTrackingVolumeVolume(origin, uniformSize);
}

void
mitk::NavigationDataSliceVisualization::SetBoxTrackingVolumeVolume(Point3D origin, double size[3])
{
  typedef itk::BoxSpatialObject<3> BoxSpatialObjectType;
  BoxSpatialObjectType::Pointer box = BoxSpatialObjectType::New();
  box->SetSize(size);

  // Shift the origin
  typedef BoxSpatialObjectType::TransformType                 TransformType;
  TransformType::Pointer transform = TransformType::New();
  transform->SetIdentity();
  TransformType::OutputVectorType  translation;
  translation[0] = origin[0];
  translation[1] = origin[1];
  translation[2] = origin[2];
  transform->Translate(translation, false);
  box->SetObjectToParentTransform( transform );

  this->SetTrackingVolume(box);
}
