// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/HUD.h"
#include "MusicSuiteHUD.generated.h"

/**
 * 
 */
UCLASS()
class MUSICSUITE_API AMusicSuiteHUD : public AHUD
{
	GENERATED_BODY()


	virtual void DrawHUD() override;
	
};
