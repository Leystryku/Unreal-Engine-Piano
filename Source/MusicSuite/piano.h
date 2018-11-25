// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Components/PoseableMeshComponent.h"


#include <string>
#include "IUsable.h"

#include "piano.generated.h"

UCLASS()
class MUSICSUITE_API Apiano : public AActor, public IUsable
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	Apiano();

	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	virtual void OnUsed(APawn *user) override;
	virtual void DrawHUD(UCanvas *Canvas, APawn *user) override;

public:
	
	void PressKey(uint8 keynum, uint8 velocity);
	void ReleaseKey(uint8 keynum, uint8 velocity);
	uint8 ConvertToModelKey(uint8 keynum);
	std::string TranslateToNoteName(uint8 keynum);
	UPROPERTY()
		TArray<int16> keypresses;

	UPROPERTY()
		TArray<uint8> keypresses_state;

	UPROPERTY()
		TArray<uint8> keypresses_velocity;

	UPROPERTY()
		TArray<USoundWave*> pianokeys;

	UPROPERTY()
		TArray<UAudioComponent*> pianokeyscomp;

	UPROPERTY()
		UFont* font;

	UPROPERTY()
		int8 lidstatus = 0;

public: //play .mid/.midi file

	bool PlayMIDIFile(TCHAR *file);

	uint8* midibuf = 0;
	int64 midibuf_size = 0;

};
