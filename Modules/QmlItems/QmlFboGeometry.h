#ifndef MITK_QMLFBOGEOMETRY_H
#define MITK_QMLFBOGEOMETRY_H

#include "mitkDisplayGeometry.h"

namespace mitk {

/*
 * \brief A type of DisplayGeometry for QML FBO rendering
 *
 * With the new FBO-based intergration approach used by QVTKQuickItem
 * rendering mouse coordinates are already have in a coordinate frame with
 * the upper left viewport as the origin (aka "ULDisplay") so
 * DisplayToULDisplay and ULDisplayToDisplay should not do any transformation.
 */
class QmlFboGeometry : public DisplayGeometry
{
public:
  mitkClassMacro(QmlFboGeometry, DisplayGeometry)
  mitkNewMacro1Param(Self, mitk::DisplayGeometry*)

  /** \brief Simply copies pt_ULdisplay to pt_display */
  virtual void ULDisplayToDisplay(const Point2D &pt_ULdisplay, Point2D &pt_display) const
  { pt_display = pt_ULdisplay; }
  /** \brief Simply copies pt_display to pt_ULdisplay */
  virtual void DisplayToULDisplay(const Point2D &pt_display, Point2D &pt_ULdisplay) const
  { pt_ULdisplay = pt_display; }
  /** \brief Simply copies vec_ULdisplay to vec_display */
  virtual void ULDisplayToDisplay(const Vector2D &vec_ULdisplay, Vector2D &vec_display) const
  { vec_display = vec_ULdisplay; }
  /** \brief Simply copies vec_display to vec_ULdisplay */
  virtual void DisplayToULDisplay(const Vector2D &vec_display, Vector2D &vec_ULdisplay) const
  { vec_ULdisplay = vec_display; }

protected:

  QmlFboGeometry(mitk::DisplayGeometry *displayGeometry) : Superclass(*displayGeometry) {}
  virtual ~QmlFboGeometry() {}
};

} // namespace mitk

#endif // MITK_QMLFBOGEOMETRY_H
