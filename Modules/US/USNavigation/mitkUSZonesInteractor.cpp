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

#include "mitkUSZonesInteractor.h"
#include "mitkDataStorage.h"
#include "mitkDataNode.h"
#include "mitkSurface.h"
#include "mitkInteractionPositionEvent.h"

#include "vtkSphereSource.h"

void mitk::USZonesInteractor::UpdateSurface(mitk::DataNode::Pointer dataNode)
{
  if ( ! dataNode->GetData())
  {
    MITK_WARN("USZonesInteractor")("DataInteractor")
        << "Cannot update surface for node as no data is set to the node.";
    return;
  }

  mitk::Point3D origin = dataNode->GetData()->GetGeometry()->GetOrigin();

  mitk::ScalarType radius;
  if ( ! dataNode->GetFloatProperty("zone.size", radius) )
  {
    MITK_WARN("USZonesInteractor")("DataInteractor")
        << "Cannut update surface for node as no radius is specified in the node properties.";
    return;
  }

  mitk::Surface::Pointer zone = mitk::Surface::New();

  vtkSphereSource *vtkData = vtkSphereSource::New();
  vtkData->SetRadius( radius );
  vtkData->SetCenter(0,0,0);
  vtkData->Update();
  zone->SetVtkPolyData(vtkData->GetOutput());
  vtkData->Delete();

  dataNode->SetData(zone);

  dataNode->GetData()->GetGeometry()->SetOrigin(origin);

  // update the RenderWindow to show the changed surface
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

mitk::USZonesInteractor::USZonesInteractor()
{
}

mitk::USZonesInteractor::~USZonesInteractor()
{
}

void mitk::USZonesInteractor::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("addCenter", AddCenter);
  CONNECT_FUNCTION("changeRadius", ChangeRadius);
  CONNECT_FUNCTION("endCreation", EndCreation);
  CONNECT_FUNCTION("abortCreation", AbortCreation);
}

void mitk::USZonesInteractor::DataNodeChanged()
{
  if ( this->GetDataNode()->GetData() == 0 )
  {
    this->GetDataNode()->SetData(mitk::Surface::New());
  }
}

bool mitk::USZonesInteractor::AddCenter(mitk::StateMachineAction* , mitk::InteractionEvent* interactionEvent)
{
  // cast InteractionEvent to a position event in order to read out the mouse position
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  if (positionEvent == NULL) { return false; }

  this->GetDataNode()->SetBoolProperty("zone.created", false);

  this->GetDataNode()->SetData(mitk::Surface::New());

  // set origin of the data node to the mouse click position
  this->GetDataNode()->GetData()->GetGeometry()->SetOrigin(positionEvent->GetPositionInWorld());

  this->GetDataNode()->SetFloatProperty("opacity", 0.60f );

  return true;
}

bool mitk::USZonesInteractor::ChangeRadius(mitk::StateMachineAction* , mitk::InteractionEvent* interactionEvent)
{
  // cast InteractionEvent to a position event in order to read out the mouse position
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  if (positionEvent == NULL) { return false; }

  mitk::DataNode::Pointer curNode = this->GetDataNode();
  mitk::Point3D mousePosition = positionEvent->GetPositionInWorld();

  mitk::ScalarType radius = mousePosition.EuclideanDistanceTo(curNode->GetData()->GetGeometry()->GetOrigin());
  //MITK_INFO << "Radius: " << radius;
  curNode->SetFloatProperty("zone.size", radius);

  mitk::USZonesInteractor::UpdateSurface(curNode);
  /*mitk::Point3D origin = curNode->GetData()->GetGeometry()->GetOrigin();
  curNode->SetData(this->MakeSphere(radius));
  this->GetDataNode()->GetData()->GetGeometry()->SetOrigin(origin);*/

  // update the RenderWindow to show the changed surface
  //mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  return true;
}

bool mitk::USZonesInteractor::EndCreation(mitk::StateMachineAction* , mitk::InteractionEvent* /*interactionEvent*/)
{
  this->GetDataNode()->SetBoolProperty("zone.created", true);
  return true;
}

bool mitk::USZonesInteractor::AbortCreation(mitk::StateMachineAction* , mitk::InteractionEvent*)
{
  this->GetDataNode()->SetData(mitk::Surface::New());

  // update the RenderWindow to remove the surface
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  return true;
}
