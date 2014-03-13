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

#include "vtkMitkRectangleProp.h"
#include "vtkObjectFactory.h"

#include "mitkGL.h"

vtkStandardNewMacro(vtkMitkRectangleProp);

vtkMitkRectangleProp::vtkMitkRectangleProp()
{
}

vtkMitkRectangleProp::~vtkMitkRectangleProp()
{
}

double *vtkMitkRectangleProp::GetBounds()
{
  return NULL;
}

int vtkMitkRectangleProp::RenderOverlay(vtkViewport* /*viewport*/)
{
  m_RenderWindow->MakeCurrent();

  Enable2DOpenGL();

  //make it nicer
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);

  //size and position
  int * i = m_RenderWindow->GetSize();
  GLdouble bbox[8] = {0.f , 0.f, (double)i[0], 0.f, (double)i[0], (double)i[1], 0.f, (double)i[1]};

  //render rectangle
  glLineWidth(5.0f);
  glBegin(GL_LINE_LOOP);
  for (int j = 0; j < 4; j++)
  {
    glColor3f(m_Color[0],m_Color[1],m_Color[2]);
    glVertex2dv(&bbox[2*j]);
  }
  glEnd();
  glLineWidth(1.0f);

  glDisable(GL_LINE_SMOOTH);

  Disable2DOpenGL();

  return 1;
}

void vtkMitkRectangleProp::SetRenderWindow(vtkRenderWindow* renWin)
{
  m_RenderWindow = renWin;
}

void vtkMitkRectangleProp::SetColor(float col1, float col2, float col3)
{
  m_Color[0] = col1;
  m_Color[1] = col2;
  m_Color[2] = col3;
}


int vtkMitkRectangleProp::RenderTranslucentGeometry(vtkViewport* /*viewport*/)
{
  return 0;
}

int vtkMitkRectangleProp::RenderOpaqueGeometry(vtkViewport* /*viewport*/)
{
  return 0;
}

void vtkMitkRectangleProp::Enable2DOpenGL()
{
  GLint iViewport[4];

  // Get a copy of the viewport
  glGetIntegerv( GL_VIEWPORT, iViewport );

  // Save a copy of the projection matrix so that we can restore it
  // when it's time to do 3D rendering again.
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();

  // Set up the orthographic projection
  glOrtho( iViewport[0], iViewport[0]+iViewport[2],
  iViewport[1]+iViewport[3], iViewport[1], -1, 1 );
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();

  // Make sure depth testing and lighting are disabled for 2D rendering until
  // we are finished rendering in 2D
  glPushAttrib( GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT | GL_TEXTURE_2D);
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_LIGHTING   );
  glDisable( GL_TEXTURE_2D );
}

void vtkMitkRectangleProp::Disable2DOpenGL()
{
  glPopAttrib();
  glMatrixMode( GL_PROJECTION );
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();
}

