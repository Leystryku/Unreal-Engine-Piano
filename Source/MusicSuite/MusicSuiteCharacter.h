// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once


#include <vector>

#include "GameFramework/Character.h"
#include "IUsable.h"


#include "piano.h"
#include "MusicSuiteCharacter.generated.h"

UCLASS(config=Game)
class AMusicSuiteCharacter : public ACharacter
{
	GENERATED_BODY()


	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;
public:
	AMusicSuiteCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCameraComponent; }

public: // overrides
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float deltaseconds) override;

public: // movement actions
	
	virtual void PressUseKey();
	virtual void ReleaseUseKey();

	virtual void PressCrouchKey();
	virtual void ReleaseCrouchKey();


	virtual void PressEscapeKey();
	virtual void ReleaseEscapeKey();

	virtual void PressSprintKey();
	virtual void ReleaseSprintKey();


public: //network midi

	UFUNCTION(reliable, server, WithValidation)
	void ServerReceiveMIDI(uint8 type, uint8 chan, uint8 flag1, uint8 flag2);
	virtual void ServerReceiveMIDI_Implementation(uint8 type, uint8 chan, uint8 flag1, uint8 flag2);
	virtual bool ServerReceiveMIDI_Validate(uint8 type, uint8 chan, uint8 flag1, uint8 flag2);


	UFUNCTION(Reliable, NetMulticast)
	void MultiCastReceiveMIDI(uint8 type, uint8 chan, uint8 flag1, uint8 flag2);

	virtual void MultiCastReceiveMIDI_Implementation(uint8 type, uint8 chan, uint8 flag1, uint8 flag2);

	virtual void SendMIDIInput(std::vector<unsigned char> *mididata);
	
	virtual void CheckMIDIInputs();
	virtual void CheckUseState();

public: //network usefocus

	UFUNCTION(reliable, server, WithValidation)
		void ServerReceiveUsed();
	virtual void ServerReceiveUsed_Implementation();
	virtual bool ServerReceiveUsed_Validate();


public:

	UPROPERTY()
		TArray<uint32> currentmidi;

	UPROPERTY()
		TArray<uint32> notes;


	UPROPERTY(replicated)
		int32 health;

	UPROPERTY(replicated)
		Apiano*usingpiano;



	IUsable *UseFocus;

	static std::vector <void*> midiins;
	static std::vector <void*> midiouts;

};

