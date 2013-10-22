#ifndef MITKNAVIGATIONDATASLICEVISUALIZATION_H_HEADER_INCLUDED_
#define MITKNAVIGATIONDATASLICEVISUALIZATION_H_HEADER_INCLUDED_

#include "mitkNavigationDataToNavigationDataFilter.h"
#include "mitkBaseRenderer.h"
#include "mitkVector.h"

#include <itkSpatialObject.h>

namespace mitk
{
/**Documentation
* \brief Control the position and orientation of rendered slices with NavigationData
*
* A NavigationDataToNavigationDataFilter that takes NavigationData as input and
* sets the position and, optionally, the orientation of the slice plane for a
* user-specified renderer.
*
* \ingroup IGT
*/
class MitkIGT_EXPORT NavigationDataSliceVisualization : public NavigationDataToNavigationDataFilter
{
  public:
    mitkClassMacro(NavigationDataSliceVisualization, NavigationDataToNavigationDataFilter)
    itkNewMacro(Self)

    enum ViewDirection
    {
      /**
       * Tracked slice planes are NOT re-oriented, only the position
       * of the slice plane is controlled by the input navigation data.
       */
      Axial = 0,
      Sagittal,
      Frontal,
      /**
       * Axial plane "tilted" about the lateral vector so that it is coplanar
       * with the tool trajectory
       */
      AxialOblique,
      /**
       * Sagittal plane "tilted" about the axial vector so that it is coplanar
       * with the tool trajectory
       */
      SagittalOblique,
      /**
       * Slice plane normal to the tool trajectory
       */
      Oblique
    };

    typedef itk::SpatialObject<3> SpatialObjectType;

    /**
     * \brief Set/get the renderer that visualizes the navigation data
     */
    itkSetObjectMacro(Renderer,BaseRenderer)
    itkGetConstObjectMacro(Renderer,BaseRenderer)

    /**
     * \brief Set/get the tip offset used for plane tracking
     *
     * This is an additional offset vector applied to the input navigation
     * data. It is defined in tool tip coordinates. In other words:
     *
     * \code
     * position_slice = position_input + orient_input.rotate(TipOffset)
     * \endcode
     *
     * Default is [0,0,0].
     */
    itkSetMacro(TipOffset, Vector3D)
    itkGetConstMacro(TipOffset,Vector3D)

    /**
     * \brief Set/get the tool trajectory used to control oblique slices
     *
     * This vector is defined in tool tip coordinates.
     *
     * Default is [0,0,-1].
     */
    virtual void SetToolTrajectory(Vector3D direction);
    itkGetConstMacro(ToolTrajectory, Vector3D)

    /**
     * \brief Set/get the world up vector used to define the y-axis of the
     * cutting plane (only for ViewDirection == Oblique)
     *
     * This vector, define in world coordinates, is projected onto the cutting
     * plane to define the orientation of slices.
     *
     * Default is [0,1,0].
     */
    itkSetMacro(WorldUpVector, Vector3D)
    itkGetConstMacro(WorldUpVector, Vector3D)

    /**
     * \brief Set/get the orientation of the sliced plane
     *
     * Default is Axial.
     */
    itkSetEnumMacro(ViewDirection,ViewDirection)
    itkGetEnumMacro(ViewDirection,ViewDirection)

    /**
     * \brief Set/get the tracking volume
     *
     * When the input position information falls outside this volume the
     * sliced geometry visualization updates will stop (allowing for control
     * by, e.g., a user interface GUI).
     *
     * Set to NULL to completely disable tracking volume limit.
     *
     * Default is NULL.
     */
    itkSetObjectMacro(TrackingVolume, SpatialObjectType);
    itkGetConstObjectMacro(TrackingVolume, SpatialObjectType);

    /**
     * \brief Convenience methods for setting an elliptically shaped tracking volume
     */
    void SetSphereTrackingVolume(Point3D origin, double radius);
    void SetEllipseTrackingVolume(Point3D origin, double radius[3]);

    /**
     * \brief Convenience methods for setting a box shaped tracking volume
     */
    void SetBoxTrackingVolume(Point3D origin, double size);
    void SetBoxTrackingVolume(Point3D origin, double size[3]);

  protected:
    NavigationDataSliceVisualization();
    virtual void GenerateData();

    /**
     * \brief Save the position and orientation of the user-selected slice
     *
     * Called the first time Update() is called on the filter with a valid
     * renderer set and on the first Update() each time the tool tip enters
     * the tracking volume thereafter.
     */
    virtual void SaveLastUserSelectedSlice();

    /**
     * \brief Restore the position and orientation of the last user-selected slice
     *
     * Called on the first Update() each time the tool tip leaves the
     * tracking volume.
     */
    virtual void RestoreLastUserSelectedSlice();

    BaseRenderer::Pointer m_Renderer;
    Vector3D m_TipOffset;
    Vector3D m_ToolTrajectory;
    Vector3D m_WorldUpVector;
    ViewDirection m_ViewDirection;
    SpatialObjectType::Pointer m_TrackingVolume;
    bool m_JustExceededTrackingVolume;
    bool m_FirstUpdate;
    unsigned int m_LastUserSelectedSlice;
    Vector3D m_LastUserSelectedSliceAxes[2];
};

} // end namespace mitk

#endif // NEMOSLICEVISUALIZATIONFILTER_H
