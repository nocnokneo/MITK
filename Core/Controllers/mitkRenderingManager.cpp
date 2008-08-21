/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkRenderingManager.h"
#include "mitkRenderingManagerFactory.h"
#include "mitkBaseRenderer.h"

#include <vtkRenderWindow.h>

#include <vtkRenderWindow.h>

#include <itkCommand.h>
#include <algorithm>

namespace mitk
{

RenderingManager::Pointer RenderingManager::s_Instance = 0;
RenderingManagerFactory *RenderingManager::s_RenderingManagerFactory = 0;


RenderingManager
::RenderingManager()
: m_UpdatePending( false ),
  m_CurrentLOD( 0 ),
  m_MaxLOD( 2 ),
  m_NumberOf3DRW( 0 ),
  m_ClippingPlaneEnabled( false ),
  m_TimeNavigationController( NULL ),
  m_LODIncreaseBlocked( false )
{
  m_ShadingEnabled.assign(3, false);
  m_ShadingValues.assign(4, 0.0);
}


RenderingManager
::~RenderingManager()
{

}


void
RenderingManager
::SetFactory( RenderingManagerFactory *factory )
{
  s_RenderingManagerFactory = factory;
}


const RenderingManagerFactory *
RenderingManager
::GetFactory()
{
  return s_RenderingManagerFactory;
}


bool
RenderingManager
::HasFactory()
{
  if ( RenderingManager::s_RenderingManagerFactory )
  {
    return true;
  }
  else
  {
    return false;
  }
}


RenderingManager::Pointer
RenderingManager
::New()
{
  const RenderingManagerFactory* factory = GetFactory();
  if(factory == NULL)
    return NULL;
  return factory->CreateRenderingManager();
}


RenderingManager *
RenderingManager
::GetInstance()
{
  if ( !RenderingManager::s_Instance )
  {
    if ( s_RenderingManagerFactory )
    {
      s_Instance = s_RenderingManagerFactory->CreateRenderingManager();
    }
  }

  return s_Instance;
}


bool
RenderingManager
::IsInstantiated()
{
  if ( RenderingManager::s_Instance )
    return true;
  else
    return false;
}


void
RenderingManager
::AddRenderWindow( vtkRenderWindow *renderWindow )
{
  m_RenderWindowList[renderWindow] = RENDERING_INACTIVE;
  m_AllRenderWindows.push_back( renderWindow );

  typedef itk::MemberCommand< RenderingManager > MemberCommandType;

  // Add callbacks for rendering abort mechanism
  BaseRenderer *renderer = BaseRenderer::GetInstance( renderWindow );
  if ( renderer )
  {
    MemberCommandType::Pointer startCallbackCommand = MemberCommandType::New();
    startCallbackCommand->SetCallbackFunction(
      this, &RenderingManager::RenderingStartCallback);
    renderer->AddObserver( itk::StartEvent(), startCallbackCommand );

    MemberCommandType::Pointer progressCallbackCommand = MemberCommandType::New();
    progressCallbackCommand->SetCallbackFunction(
      this, &RenderingManager::RenderingProgressCallback);
    renderer->AddObserver( itk::ProgressEvent(), progressCallbackCommand );

    MemberCommandType::Pointer endCallbackCommand = MemberCommandType::New();
    endCallbackCommand->SetCallbackFunction(
      this, &RenderingManager::RenderingEndCallback);
    renderer->AddObserver( itk::EndEvent(), endCallbackCommand );
 }
}


void
RenderingManager
::RemoveRenderWindow( vtkRenderWindow *renderWindow )
{
  m_RenderWindowList.erase( renderWindow );

  RenderWindowVector::iterator thisRenderWindowsPosition = std::find( m_AllRenderWindows.begin(), m_AllRenderWindows.end(), renderWindow );
  if ( thisRenderWindowsPosition != m_AllRenderWindows.end() )
  {
    m_AllRenderWindows.erase( thisRenderWindowsPosition );
  }
}


const RenderingManager::RenderWindowVector&
RenderingManager
::GetAllRegisteredRenderWindows()
{
  return m_AllRenderWindows;
}


void
RenderingManager
::RequestUpdate( vtkRenderWindow *renderWindow )
{
  if ( m_RenderWindowList[renderWindow] == RENDERING_INPROGRESS )
  {
    this->AbortRendering( renderWindow );
  }

  m_RenderWindowList[renderWindow] = RENDERING_REQUESTED;

  if ( !m_UpdatePending )
  {
    m_UpdatePending = true;
    this->RestartTimer();
  }
}


void
RenderingManager
::CheckUpdatePending()
{
  // Check if there are pending requests for any other windows
  m_UpdatePending = false;
  RenderWindowList::iterator it;
  m_NumberOf3DRW = 0;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    if ( it->second == RENDERING_REQUESTED )
    {
      m_UpdatePending = true;
    }
    if ( BaseRenderer::GetInstance(it->first)->GetMapperID() == 2 )
    {
      m_NumberOf3DRW++;
    }
  }
}


void
RenderingManager
::ForceImmediateUpdate( vtkRenderWindow *renderWindow )
{
  // Erase potentially pending requests for this window
  m_RenderWindowList[renderWindow] = RENDERING_INACTIVE;

  this->CheckUpdatePending();

  // Stop the timer if no more requests are pending
  if ( !m_UpdatePending )
  {
    this->StopTimer();
  }

  m_LastUpdatedRW = renderWindow;

  // Immediately repaint this window (implementation platform specific)
  renderWindow->Render();

}


void
RenderingManager
::RequestUpdateAll( unsigned int requestType )
{
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    int id = BaseRenderer::GetInstance(it->first)->GetMapperID();
    if ( (requestType == REQUEST_UPDATE_ALL)
      || ((requestType == REQUEST_UPDATE_2DWINDOWS) && (id == 1))
      || ((requestType == REQUEST_UPDATE_3DWINDOWS) && (id == 2)) )
    {
      this->RequestUpdate( it->first);

      if ( m_RenderWindowList[it->first] == RENDERING_INACTIVE )
      {
        m_RenderWindowList[it->first] = RENDERING_REQUESTED;
      }
    }
  }

  // Restart the timer if there are no requests already
  if ( !m_UpdatePending )
  {
    m_UpdatePending = true;
    this->RestartTimer();
  }
}


void
RenderingManager
::ForceImmediateUpdateAll( unsigned int requestType )
{
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    int id = BaseRenderer::GetInstance(it->first)->GetMapperID();
    if ( (requestType == REQUEST_UPDATE_ALL)
      || ((requestType == REQUEST_UPDATE_2DWINDOWS) && (id == 1))
      || ((requestType == REQUEST_UPDATE_3DWINDOWS) && (id == 2)) )
    {
      //it->second = RENDERING_INPROGRESS;

      // Immediately repaint this window (implementation platform specific)
      it->first->Render();

      it->second = RENDERING_INACTIVE;
    }
  }

  if ( m_UpdatePending )
  {
    this->StopTimer();
    m_UpdatePending = false;
  }

  this->CheckUpdatePending();
}


bool
RenderingManager
::InitializeViews( DataTreeIteratorBase * dataIt, unsigned int requestType )
{
  mitk::Geometry3D::Pointer geometry;
  if ( dataIt != NULL )
  {
    geometry = mitk::DataTree::ComputeVisibleBoundingGeometry3D(
      dataIt, NULL, "includeInBoundingBox" );

    if ( geometry.IsNotNull() )
    {
      // let's see if we have data with a limited live-span ...
      mitk::TimeBounds timebounds = geometry->GetTimeBounds();
      if ( timebounds[1] < mitk::ScalarTypeNumericTraits::max() )
      {
        mitk::ScalarType duration = timebounds[1]-timebounds[0];

        mitk::TimeSlicedGeometry::Pointer timegeometry =
          mitk::TimeSlicedGeometry::New();
        timegeometry->InitializeEvenlyTimed(
          geometry, (unsigned int) duration );
        timegeometry->SetTimeBounds( timebounds );

        timebounds[1] = timebounds[0] + 1.0;
        geometry->SetTimeBounds( timebounds );

        geometry = timegeometry;
      }
    }
  }

  // Use geometry for initialization
  return this->InitializeViews( geometry.GetPointer(), requestType );
}


bool
RenderingManager
::InitializeViews( const DataStorage * storage, unsigned int requestType )
{
  //TODO native DataStorage code
  mitk::DataTreePreOrderIterator it(storage->m_DataTree);
  return this->InitializeViews(&it, requestType);
}


bool
RenderingManager
::InitializeViews( const Geometry3D * geometry, unsigned int requestType )
{
  bool boundingBoxInitialized = false;

  int warningLevel = vtkObject::GetGlobalWarningDisplay();
  vtkObject::GlobalWarningDisplayOff();

  if ( (geometry != NULL ) && (const_cast< mitk::BoundingBox * >(
    geometry->GetBoundingBox())->GetDiagonalLength2() > mitk::eps) )
  {
    boundingBoxInitialized = true;
  }

  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    mitk::BaseRenderer *baseRenderer =
      mitk::BaseRenderer::GetInstance( it->first );
    int id = baseRenderer->GetMapperID();
    if ( (requestType == REQUEST_UPDATE_ALL)
      || ((requestType == REQUEST_UPDATE_2DWINDOWS) && (id == 1))
      || ((requestType == REQUEST_UPDATE_3DWINDOWS) && (id == 2)) )
    {
      this->InternalViewInitialization( baseRenderer, geometry,
        boundingBoxInitialized, id );
    }
  }

  if ( m_TimeNavigationController != NULL )
  {
    if ( boundingBoxInitialized )
    {
      m_TimeNavigationController->SetInputWorldGeometry( geometry );
    }
    m_TimeNavigationController->Update();
  }

  this->RequestUpdateAll( requestType );

  vtkObject::SetGlobalWarningDisplay( warningLevel );

  // Inform listeners that views have been initialized
  this->InvokeEvent( mitk::RenderingManagerViewsInitializedEvent() );


  return boundingBoxInitialized;
}


bool
RenderingManager
::InitializeViews( unsigned int requestType )
{
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    mitk::BaseRenderer *baseRenderer =
      mitk::BaseRenderer::GetInstance( it->first );
    int id = baseRenderer->GetMapperID();
    if ( (requestType == REQUEST_UPDATE_ALL)
      || ((requestType == REQUEST_UPDATE_2DWINDOWS) && (id == 1))
      || ((requestType == REQUEST_UPDATE_3DWINDOWS) && (id == 2)) )
    {
      mitk::SliceNavigationController *snc =
        baseRenderer->GetSliceNavigationController();

      // Re-initialize view direction
      snc->SetViewDirectionToDefault();

      // Update the SNC
      snc->Update();
    }
  }

  this->RequestUpdateAll( requestType );

  return true;
}

bool
RenderingManager
::InitializeView( vtkRenderWindow * renderWindow,
  DataTreeIteratorBase * dataIt,
  bool initializeGlobalTimeSNC )
{
  mitk::Geometry3D::Pointer geometry;
  if ( dataIt != NULL )
  {
    geometry = mitk::DataTree::ComputeVisibleBoundingGeometry3D(
      dataIt, NULL, "includeInBoundingBox" );

    if ( geometry.IsNotNull() )
    {
      // let's see if we have data with a limited live-span ...
      mitk::TimeBounds timebounds = geometry->GetTimeBounds();
      if ( timebounds[1] < mitk::ScalarTypeNumericTraits::max() )
      {
        mitk::ScalarType duration = timebounds[1]-timebounds[0];

        mitk::TimeSlicedGeometry::Pointer timegeometry =
          mitk::TimeSlicedGeometry::New();
        timegeometry->InitializeEvenlyTimed(
          geometry, (unsigned int) duration );
        timegeometry->SetTimeBounds( timebounds );

        timebounds[1] = timebounds[0] + 1.0;
        geometry->SetTimeBounds( timebounds );

        geometry = timegeometry;
      }
    }
  }

  // Use geometry for initialization
  return this->InitializeView( renderWindow,
    geometry.GetPointer(), initializeGlobalTimeSNC );
}

bool
RenderingManager
::InitializeView( vtkRenderWindow * renderWindow,
  const Geometry3D * geometry,
  bool initializeGlobalTimeSNC )
{
  bool boundingBoxInitialized = false;

  int warningLevel = vtkObject::GetGlobalWarningDisplay();
  vtkObject::GlobalWarningDisplayOff();

  if ( (geometry != NULL ) && (const_cast< mitk::BoundingBox * >(
   geometry->GetBoundingBox())->GetDiagonalLength2() > mitk::eps) )
  {
   boundingBoxInitialized = true;
  }

  mitk::BaseRenderer *baseRenderer =
   mitk::BaseRenderer::GetInstance( renderWindow );

  int id = baseRenderer->GetMapperID();

  this->InternalViewInitialization( baseRenderer, geometry,
   boundingBoxInitialized, id );

  if ( m_TimeNavigationController != NULL )
  {
   if ( boundingBoxInitialized && initializeGlobalTimeSNC )
   {
     m_TimeNavigationController->SetInputWorldGeometry( geometry );
   }
   m_TimeNavigationController->Update();
  }

  this->RequestUpdate( renderWindow );

  vtkObject::SetGlobalWarningDisplay( warningLevel );

  return boundingBoxInitialized;
}


bool
RenderingManager
::InitializeView( vtkRenderWindow * renderWindow )
{
  mitk::BaseRenderer *baseRenderer =
      mitk::BaseRenderer::GetInstance( renderWindow );

  mitk::SliceNavigationController *snc =
    baseRenderer->GetSliceNavigationController();

  // Re-initialize view direction
  snc->SetViewDirectionToDefault();

  // Update the SNC
  snc->Update();

  this->RequestUpdate( renderWindow );

  return true;
}

void
RenderingManager
::InternalViewInitialization(
  mitk::BaseRenderer *baseRenderer, const mitk::Geometry3D *geometry,
  bool boundingBoxInitialized, int mapperID )
{
  mitk::SliceNavigationController *snc =
    baseRenderer->GetSliceNavigationController();

  // Re-initialize view direction
  snc->SetViewDirectionToDefault();

  if ( boundingBoxInitialized )
  {
    // Set geometry for SNC
    snc->SetInputWorldGeometry( geometry );
    snc->Update();

    if ( mapperID == 1 )
    {
      // For 2D SNCs, steppers are set so that the cross is centered
      // in the image
      snc->GetSlice()->SetPos( snc->GetSlice()->GetSteps() / 2 );
    }

    // Fit the render window DisplayGeometry
    baseRenderer->GetDisplayGeometry()->Fit();

    vtkRenderer *renderer = baseRenderer->GetVtkRenderer();

    if ( renderer != NULL ) renderer->ResetCamera();
  }
  else
  {
    snc->Update();
  }
}


void
RenderingManager
::SetTimeNavigationController( SliceNavigationController *snc )
{
  m_TimeNavigationController = snc;
}


const SliceNavigationController *
RenderingManager
::GetTimeNavigationController() const
{
  return m_TimeNavigationController;
}


SliceNavigationController *
RenderingManager
::GetTimeNavigationController()
{
  return m_TimeNavigationController;
}


void
RenderingManager
::UpdateCallback()
{
  // Satisfy all pending update requests
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    if ( it->second == RENDERING_REQUESTED )
    {
      this->ForceImmediateUpdate( it->first );
    }
  }
  m_UpdatePending = false;
}


void
RenderingManager
::RenderingStartCallback( itk::Object* object, const itk::EventObject& /*event*/ )
{
  //std::cout<< m_CurrentLOD << "<S";
  BaseRenderer* renderer = dynamic_cast< BaseRenderer* >( object );
  if (renderer)
  {
    m_RenderWindowList[renderer->GetRenderWindow()] = RENDERING_INPROGRESS;
  }
}


void
RenderingManager
::RenderingProgressCallback( itk::Object* /*object*/, const itk::EventObject& /*event*/ )
{
  //std::cout << "P";
  this->DoMonitorRendering();

}

void
RenderingManager
::RenderingEndCallback( itk::Object* object, const itk::EventObject& /*event*/ )
{
  //std::cout<<"E> "<<std::endl;
  BaseRenderer* renderer = dynamic_cast< BaseRenderer* >( object );
  if (renderer)
  {
    m_RenderWindowList[renderer->GetRenderWindow()] = RENDERING_INACTIVE;
    this->DoFinishAbortRendering();

    /** Level-Of-Detail **/
   if(m_NumberOf3DRW > 0)
    {
      if(m_CurrentLOD < m_MaxLOD)
      {
        if ( !m_LODIncreaseBlocked )
        {
          this->SetCurrentLOD( m_CurrentLOD + 1 );
          this->RequestUpdate(renderer->GetRenderWindow());
        }
      }
    }
  }
}


bool
RenderingManager
::IsRendering() const
{
  RenderWindowList::const_iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    if ( it->second == RENDERING_INPROGRESS )
    {
      return true;
    }
  }

  return false;
}


void
RenderingManager
::AbortRendering( vtkRenderWindow* renderWindow )
{
  //std::cout << "A";
  if ( (m_RenderWindowList.count( renderWindow ) != 0)
    && (m_RenderWindowList[renderWindow] == RENDERING_INPROGRESS) )
  {
    renderWindow->SetAbortRender( true );
  }
  else
  {
    RenderWindowList::iterator it;
    for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
    {
      if ( it->second == RENDERING_INPROGRESS )
      {
        it->first->SetAbortRender( true );
      }
    }
  }
}


int
RenderingManager
::GetCurrentLOD()
{
  return m_CurrentLOD;
}


void
RenderingManager
::SetCurrentLOD( int lod )
{
  //std::cout << lod << std::endl;
  if ( m_CurrentLOD != lod )
  {
    if( lod > m_MaxLOD )
    {
      itkWarningMacro(<<"LOD out of range requested: " << lod << " maxLOD: " << m_MaxLOD);
      return;
    }
    m_CurrentLOD = lod;
  }
}


void
RenderingManager
::SetNumberOfLOD( int number )
{
  m_MaxLOD = number - 1;
}


//enable/disable shading
void
RenderingManager
::SetShading(bool state, int lod)
{
  if(lod>m_MaxLOD)
  {
    itkWarningMacro(<<"LOD out of range requested: " << lod << " maxLOD: " << m_MaxLOD);
    return;
  }
  m_ShadingEnabled[lod] = state;

}

bool
RenderingManager
::GetShading(int lod)
{
  if(lod>m_MaxLOD)
  {
    itkWarningMacro(<<"LOD out of range requested: " << lod << " maxLOD: " << m_MaxLOD);
    return false;
  }
  return m_ShadingEnabled[lod];
}


//enable/disable the clipping plane
void
RenderingManager
::SetClippingPlaneStatus(bool status)
{
  m_ClippingPlaneEnabled = status;
}


bool
RenderingManager
::GetClippingPlaneStatus()
{
  return m_ClippingPlaneEnabled;
}


void
RenderingManager
::SetShadingValues(float ambient, float diffuse, float specular, float specpower)
{
  m_ShadingValues[0] = ambient;
  m_ShadingValues[1] = diffuse;
  m_ShadingValues[2] = specular;
  m_ShadingValues[3] = specpower;
}


RenderingManager::FloatVector &
RenderingManager
::GetShadingValues()
{
  return m_ShadingValues;
}


// Create and register generic RenderingManagerFactory.
GenericRenderingManagerFactory renderingManagerFactory;


} // namespace
