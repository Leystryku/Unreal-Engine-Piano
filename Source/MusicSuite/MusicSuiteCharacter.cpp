// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "MusicSuite.h"
#include "MusicSuiteCharacter.h"
#include "Runtime/Engine/Public/AudioDeviceManager.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Engine.h"
#include "UnrealNetwork.h"

#include "RtMidi.h"

#include <vector>

//////////////////////////////////////////////////////////////////////////
// AMusicSuiteCharacter


AMusicSuiteCharacter::AMusicSuiteCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

												// Create a follow camera
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->AttachParent = GetCapsuleComponent();

	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	notes.SetNumUninitialized(300);
	currentmidi.SetNumUninitialized(300);

	for (int i = 0; i < 300; i++)
	{
		currentmidi[i] = 0;
		notes[i] = 0;
	}

	bReplicates = true;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMusicSuiteCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// Set up gameplay key bindings
	check(InputComponent);
	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	InputComponent->BindAction("Use", IE_Pressed, this, &AMusicSuiteCharacter::PressUseKey);
	InputComponent->BindAction("Use", IE_Released, this, &AMusicSuiteCharacter::ReleaseUseKey);

	InputComponent->BindAction("Crouch", IE_Pressed, this, &AMusicSuiteCharacter::PressCrouchKey);
	InputComponent->BindAction("Crouch", IE_Released, this, &AMusicSuiteCharacter::ReleaseCrouchKey);

	InputComponent->BindAction("Escape", IE_Pressed, this, &AMusicSuiteCharacter::PressEscapeKey);
	InputComponent->BindAction("Escape", IE_Released, this, &AMusicSuiteCharacter::ReleaseEscapeKey);

	InputComponent->BindAction("Sprint", IE_Pressed, this, &AMusicSuiteCharacter::PressSprintKey);
	InputComponent->BindAction("Sprint", IE_Released, this, &AMusicSuiteCharacter::ReleaseSprintKey);


	InputComponent->BindAxis("MoveForward", this, &AMusicSuiteCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AMusicSuiteCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &AMusicSuiteCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AMusicSuiteCharacter::LookUpAtRate);

	// handle touch devices
	InputComponent->BindTouch(IE_Pressed, this, &AMusicSuiteCharacter::TouchStarted);
	InputComponent->BindTouch(IE_Released, this, &AMusicSuiteCharacter::TouchStopped);
}


void AMusicSuiteCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	// jump, but only on the first touch
	if (FingerIndex == ETouchIndex::Touch1)
	{
		Jump();
	}
}

void AMusicSuiteCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (FingerIndex == ETouchIndex::Touch1)
	{
		StopJumping();
	}
}

void AMusicSuiteCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMusicSuiteCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMusicSuiteCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMusicSuiteCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}


void AMusicSuiteCharacter::ServerReceiveUsed_Implementation()
{
	CheckUseState();

	if (!UseFocus)
		return;

	UseFocus->OnUsed(this);
	health = 100;
}

bool AMusicSuiteCharacter::ServerReceiveUsed_Validate()
{
	return true;
}


void AMusicSuiteCharacter::PressUseKey()
{

}

void AMusicSuiteCharacter::ReleaseUseKey()
{

	CheckUseState();

	if(!UseFocus)
		return;
	
	//UseFocus->OnUsed(this);
	ServerReceiveUsed();



}

void AMusicSuiteCharacter::PressCrouchKey()
{
	Crouch();
}

void AMusicSuiteCharacter::ReleaseCrouchKey()
{
	UnCrouch();
}

void AMusicSuiteCharacter::PressSprintKey()
{

}

void AMusicSuiteCharacter::ReleaseSprintKey()
{

}

void AMusicSuiteCharacter::PressEscapeKey()
{

}

void AMusicSuiteCharacter::ReleaseEscapeKey()
{

}

void AMusicSuiteCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMusicSuiteCharacter, usingpiano);
	DOREPLIFETIME(AMusicSuiteCharacter, health);
}


void AMusicSuiteCharacter::MultiCastReceiveMIDI_Implementation(uint8 type, uint8 chan, uint8 flag1, uint8 flag2)
{

	UE_LOG(LogTemp, Warning, TEXT("RECEIVED MIDI!\n"));


	/*int data = (type & 0xFF) | (chan << 8) | (flag1 << 16) | (flag2 << 24);
	
	int32 index = 0;

	for (index= 0; index < currentmidi.Max()-1; index++)
	{
		if (currentmidi[index]==0)
			break;
	}
	currentmidi[index] = data;
	*/
	UE_LOG(LogTemp, Warning, TEXT("DUDE MIDI: %x __ %x __ %x ___ %x __ %x\n"), type, chan, flag1, flag2, health);

	if (type == 0x90)//note on
	{
		notes[(size_t)flag1] = 1;
		if (usingpiano != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("YEPRESS"));

			usingpiano->PressKey(flag1, flag2);
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("NO PIANO"));
		}

	}


	if (type == 0x80)//note off
	{
		if (flag1 && notes[(size_t)flag1])
		{
			if (usingpiano != nullptr)
			{
				usingpiano->ReleaseKey(flag1, flag2);
			}
		}

	}
}

void AMusicSuiteCharacter::ServerReceiveMIDI_Implementation(uint8 type, uint8 chan, uint8 flag1, uint8 flag2)
{
	MultiCastReceiveMIDI(type, chan, flag1, flag2);
}


bool AMusicSuiteCharacter::ServerReceiveMIDI_Validate(uint8 type, uint8 chan, uint8 flag1, uint8 flag2)
{

	return true;	
}


void onMidiCallback(double deltatime, std::vector<unsigned char> *message, void *)
{
	/*
	size_t size = message->size();
	if (size == 0) return;


	UE_LOG(LogTemp, Warning, TEXT("DAUDE MIDI: %d "), size);

	//for (size_t i = 0; i < size; i++)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("%x"), message->at(i));

	//}


	if (!GEngine)
		return;
	
	
	
	if (!GEngine->GameViewport)
		return;

	UWorld*world = GEngine->GameViewport->GetWorld();

	if (!world)
		return;

	ACharacter *charac = UGameplayStatics::GetPlayerCharacter(world, 0);
	AMusicSuiteCharacter* curchar = dynamic_cast<AMusicSuiteCharacter*>(charac);

	if (!curchar)
		return;

	UE_LOG(LogTemp, Warning, TEXT("ENDLOG\n"));

	unsigned char type_and_chan = message->at(0);
	unsigned char type = type_and_chan & 0xF0;//high byte only
	unsigned char chan = type_and_chan & 0x0F;//low byte only

	if (type == 0x90)
	{
		unsigned char key = message->at(1);
		unsigned char velocity = message->at(2);

		if (velocity == 0)
		{
			curchar->ServerReceiveMIDI(0x80, chan, key, velocity);//It's actually a release signal
		}
		else {

			curchar->ServerReceiveMIDI(type, chan, key, velocity);
		}

		return;
	}

	if (type == 0x80)
	{
		unsigned char key = message->at(1);
		unsigned char velocity = message->at(2);

		curchar->ServerReceiveMIDI(type, chan, key, velocity);

		return;
	}

	if (type == 0xB0)
	{
		unsigned char controller = message->at(1);
		unsigned char controllervalue = message->at(2);

		curchar->ServerReceiveMIDI(type, chan, controller, controller);

		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("UNHANDLED: %x\n"), type);
	*/
}


std::vector <void*> AMusicSuiteCharacter::midiins;
std::vector <void*> AMusicSuiteCharacter::midiouts;
void AMusicSuiteCharacter::BeginPlay()
{
	Super::BeginPlay();



	RtMidiIn *midiin = 0;

	for (int i = 0; i < 6; i++)
	{
		try
		{
			midiin = new RtMidiIn;

			midiin->openPort(i);

			//midiin->setCallback(&onMidiCallback, (void*)this);
			//midiin->setErrorCallback(&onMidiErrorCallback, (void*)this);

			midiins.push_back((void*)midiin);
		}

		catch (RtMidiError&error)
		{

			if (midiin)
			{
				delete midiin;
				midiin = 0;
			}
			UE_LOG(LogTemp, Warning, TEXT("%dMidi: %s __ %d"), i, *FString(error.getMessage().c_str()), (int32)error.getType());
		}
	}




	FTimerHandle UniqueHandle;
	FTimerDelegate MidiDelegate = FTimerDelegate::CreateUObject(this, &AMusicSuiteCharacter::CheckMIDIInputs);
	GetWorldTimerManager().SetTimer(UniqueHandle, MidiDelegate, 0.0001f, true);


}


void AMusicSuiteCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	for (size_t i = 0; i < midiins.size(); i++)
	{
		RtMidiIn *midiin = (RtMidiIn*)midiins.at(i);
		midiin->closePort();
		midiin->cancelCallback();

		delete midiin;
		midiin = 0;

	}

	for (size_t i = 0; i < midiouts.size(); i++)
	{
		RtMidiOut *midiout = (RtMidiOut*)midiouts.at(i);
		midiout->closePort();

		delete midiout;
		midiout = 0;

	}

	midiins.clear();
	midiouts.clear();



}

void AMusicSuiteCharacter::Tick(float deltaseconds)
{
	Super::Tick(deltaseconds);
	
	CheckMIDIInputs();
	CheckUseState();

}

void AMusicSuiteCharacter::CheckUseState()
{
	if (!Controller)
		return;

	// we check the controller becouse we dont want bots to grab the use object and we need a controller for the Getplayerviewpoint function

	FVector CamLoc;
	FRotator CamRot;

	Controller->GetPlayerViewPoint(CamLoc, CamRot); // Get the camera position and rotation
	const FVector StartTrace = CamLoc; // trace start is the camera location
	const FVector Direction = CamRot.Vector();
	const FVector EndTrace = StartTrace + Direction * 200; // and trace end is the camera location + an offset in the direction you are looking, the 200 is the distance at wich it checks

														   // Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(FName(TEXT("UseTrace")), true, this);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingle(Hit, StartTrace, EndTrace, ECC_PhysicsBody, TraceParams); // simple trace function

	IUsable* usable = Cast<IUsable>(Hit.GetActor()); // we cast the hit actor to the IUsable interface
	if (usable==UseFocus)
		return;

	UseFocus = usable;



}

void AMusicSuiteCharacter::SendMIDIInput(std::vector<unsigned char> *message)
{
	if (!message || message->size() == 0)
		return;

	size_t size = message->size();
	if (size == 0) return;


	UE_LOG(LogTemp, Warning, TEXT("DAUDE MIDI: %d "), size);

	//for (size_t i = 0; i < size; i++)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("%x"), message->at(i));

	//}



	UE_LOG(LogTemp, Warning, TEXT("ENDLOG\n"));

	uint8 type_and_chan = message->at(0);
	uint8 type = type_and_chan & 0xF0;//high byte only
	uint8 chan = type_and_chan & 0x0F;//low byte only

	uint8 flag1 = 0;
	uint8 flag2 = 0;

	if (message->size() >= 2)
	{
		flag1 = message->at(1);
	}

	if (message->size() >= 3)
	{
		flag2 = message->at(2);
	}

	if (type == 0x90&&flag2==0)//pressed but 0 velocity = actually a release signal
		type = 0x80;

	ServerReceiveMIDI(type, chan, flag1, flag2);

	
}

void AMusicSuiteCharacter::CheckMIDIInputs()
{
	if (usingpiano==nullptr)
		return;

	for (int i = 0; i < midiins.size(); i++)
	{
		if (midiins[i] == 0)
			continue;

		RtMidiIn*midiin = (RtMidiIn*)midiins[i];
		
		std::vector<unsigned char> msg;
		midiin->getMessage(&msg);

		SendMIDIInput(&msg);

	}


	/*
	for (int i = 0; i < 255; i++)
	{
		if (currentmidi[i]==0)
			continue;

		CheckMIDIInput(currentmidi[i]);
		currentmidi[i] = 0;
	}*/


}