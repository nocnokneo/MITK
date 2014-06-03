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
  m_ViewDirection(Axial),
  m_TrackingVolume(NULL),
  m_JustExceededTrackingVolume(true),
  m_FirstUpdate(true),
  m_LastUserSelectedSlice(0)
{
  m_TipOffset[0] = 0.0f;
  m_TipOffset[1] = 0.0f;
  m_TipOffset[2] = 0.0f;

  m_ToolTrajectory[0] = 0;
  m_ToolTrajectory[1] = 0;
  m_ToolTrajectory[2] = -1;

  m_WorldUpVector[0] = 0.0;
  m_WorldUpVector[1] = 1.0;
  m_WorldUpVector[2] = 0.0;

  m_LastUserSelectedSliceAxes[0][0] = 1.0;
  m_LastUserSelectedSliceAxes[0][1] = 0.0;
  m_LastUserSelectedSliceAxes[0][2] = 0.0;

  m_LastUserSelectedSliceAxes[1][0] = 0.0;
  m_LastUserSelectedSliceAxes[1][1] = 1.0;
  m_LastUserSelectedSliceAxes[1][2] = 0.0;
}

void mitk::NavigationDataSliceVisualization::SetToolTrajectory(Vector3D direction)
{
  if (Equal(direction.GetNorm(), 0.0))
  {
    MITK_WARN << "Ignoring invalid direction of projection: " << direction;
    return;
  }

  if (m_ToolTrajectory != direction)
  {
    m_ToolTrajectory = direction;
    this->SetViewDirection(Oblique);
    this->Modified();
  }
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
  if (numberOfInputs == 0)
  {
    return;
  }
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

  // Nothing left to do if we don't have an input with valid data
  if (numberOfInputs == 0 || !this->GetInput()->IsDataValid())
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

    Vector3D transformedTipOffset;
    transformedTipOffset.SetVnlVector(orientation.rotate(m_TipOffset.GetVnlVector()));

    slicePosition += transformedTipOffset;

    mitk::SliceNavigationController::Pointer snc = m_Renderer->GetSliceNavigationController();

    if (Axial == m_ViewDirection)
    {
      snc->SetViewDirection(mitk::SliceNavigationController::Axial);
      snc->SelectSliceByPoint(slicePosition);
    }
    else if (Sagittal == m_ViewDirection)
    {
      snc->SetViewDirection(mitk::SliceNavigationController::Sagittal);
      snc->SelectSliceByPoint(slicePosition);
    }
    else if (Frontal == m_ViewDirection)
    {
      snc->SetViewDirection(mitk::SliceNavigationController::Frontal);
      snc->SelectSliceByPoint(slicePosition);
    }
    else if (AxialOblique == m_ViewDirection || SagittalOblique == m_ViewDirection)
    {
      const int slicingPlaneXAxis = AxialOblique == m_ViewDirection ? 0 : 2;

      // The column 0 is the slicing plane's x-axis, column 1 is the slicing plane's y-axis
      const mitk::Geometry2D::TransformType::MatrixType &m =
            m_Renderer->GetCurrentWorldGeometry2D()->GetIndexToWorldTransform()->GetMatrix();

      // Rotate the tool trajectory vector into world coordinate frame (assuming
      // NavigationData has passed through a NavigationDataTransformFilter to
      // convert it into world coordinate frame)
      Vector3D slicingPlaneYAxisVector;
      slicingPlaneYAxisVector.SetVnlVector(orientation.rotate(m_ToolTrajectory.GetVnlVector()));

      // Project the tool trajectory onto the sagittal plane. This defines the
      // y-axis ("up") of the axial oblique slicing plane
      slicingPlaneYAxisVector[slicingPlaneXAxis] = 0.0;

      // Do nothing for ambigous/undefined case (the R-L component of the x-axis is zero or
      // the A-P component of the y-axis is zero)
      if ( m(slicingPlaneXAxis,0) == 0.0 ||
           m(1,1) == 0.0 ||
           (slicingPlaneXAxis != 0 && slicingPlaneYAxisVector[0] == 0.0) ||
           (slicingPlaneXAxis != 1 && slicingPlaneYAxisVector[1] == 0.0) ||
           (slicingPlaneXAxis != 2 && slicingPlaneYAxisVector[2] == 0.0) )
      {
        return;
      }

      // Maintain the A-P orientation of the slice's y-axis regardless of what
      // direction the tool trajectory points
      /// @todo<C++11> Use std::signbit
      if ( (m(1,1) > 0) != (slicingPlaneYAxisVector[1] > 0) )
      {
        slicingPlaneYAxisVector *= -1;
      }

      Vector3D slicingPlaneXAxisVector;
      slicingPlaneXAxisVector.Fill(0.0);
      // For AxialOblique: maintain the Left/Right direction of the slice's x-axis
      // For SagittalOblique: maintain the Superior/Inferior direction of the slice's x-axis
      /// @todo<C++11> Use std::copysign
      slicingPlaneXAxisVector[slicingPlaneXAxis] = m(slicingPlaneXAxis,0) > 0 ? 1.0 : -1.0;

      snc->ReorientSlices(slicePosition, slicingPlaneXAxisVector, slicingPlaneYAxisVector);
    }
    else if (Oblique == m_ViewDirection)
    {
      Vector3D slicingPlaneNormalVector;
      slicingPlaneNormalVector.SetVnlVector(orientation.rotate(m_ToolTrajectory.GetVnlVector()));

      mitk::PlaneGeometry::Pointer slicingPlane = mitk::PlaneGeometry::New();
      slicingPlane->InitializePlane(slicePosition, slicingPlaneNormalVector);

      // Project the world y-axis onto the cutting plane to define the up
      // vector of the reoriented slices
      mitk::Vector3D slicingPlaneUpVector; // *normalized*
      if ( slicingPlane->Project(m_WorldUpVector, slicingPlaneUpVector) )
      {
        // slicingPlaneUpVector CROSS slicingPlaneNormalVector -> slicingPlaneRightVector
        Vector3D slicingPlaneRightVector = itk::CrossProduct(slicingPlaneUpVector,
                                                             slicingPlaneNormalVector);

        snc->ReorientSlices(slicePosition, slicingPlaneRightVector, slicingPlaneUpVector);
      }
    }
    else
    {
      MITK_ERROR << "Unsupported ViewDirection: " << m_ViewDirection;
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
  TimeSlicedGeometry::ConstPointer userWorldGeometry = m_Renderer->GetSliceNavigationController()->GetCreatedWorldGeometry();
  if (userWorldGeometry)
  {
     m_LastUserSelectedSlice = m_Renderer->GetSliceNavigationController()->GetSlice()->GetPos();
     m_LastUserSelectedSliceAxes[0] = userWorldGeometry->GetAxisVector(0);
     m_LastUserSelectedSliceAxes[1] = userWorldGeometry->GetAxisVector(1);
  }
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
mitk::NavigationDataSliceVisualization::SetBoxTrackingVolume(Point3D origin, double size)
{
  double uniformSize[3] = {size, size, size};
  this->SetBoxTrackingVolume(origin, uniformSize);
}

void
mitk::NavigationDataSliceVisualization::SetBoxTrackingVolume(Point3D origin, double size[3])
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
