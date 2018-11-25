// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"
#include "MusicSuiteGameMode.generated.h"

UCLASS(minimalapi)
class AMusicSuiteGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AMusicSuiteGameMode();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController *cont) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> wMainMenu;

	UUserWidget* main_menu;
};



