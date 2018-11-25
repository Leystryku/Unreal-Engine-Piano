// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MusicSuite.h"
#include "IUsable.generated.h"


/**This interface has to be added to every Actor that can be used, you have to implement the OnUsed function */
UINTERFACE(MinimalAPI)
class UUsable : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class IUsable
{
	GENERATED_IINTERFACE_BODY()
	// This function will be called when the user uses the object
	virtual void OnUsed(APawn *user);

	// This function is called each frame from the hud, it should be used to put messages to the screen, like the USE prompt in UDK
	virtual void DrawHUD(UCanvas *Canvas, APawn *user);
};