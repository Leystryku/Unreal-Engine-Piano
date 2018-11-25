// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "MusicSuite.h"
#include "MusicSuiteGameMode.h"
#include "MusicSuiteCharacter.h"

#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Blueprint/UserWidget.h"

#include "RtMidi.h"

AMusicSuiteGameMode::AMusicSuiteGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Characters/BasePlayer/BasePlayer"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

}

void AMusicSuiteGameMode::BeginPlay()
{
	Super::BeginPlay();
	RtMidiIn  *midiin = nullptr;
	RtMidiOut *midiout = nullptr;
	// RtMidiIn constructor
	try {
		midiin = new RtMidiIn();
	}
	catch (RtMidiError &error) {
		error.printMessage();
		exit(EXIT_FAILURE);
	}
	// Check inputs.
	unsigned int nPorts = midiin->getPortCount();

	UE_LOG(LogTemp, Warning, TEXT("\nThere are %d MIDI input sources available.\n"), nPorts);

	std::string portName;
	for (unsigned int i = 0; i<nPorts; i++) {
		try {
			portName = midiin->getPortName(i);
		}
		catch (RtMidiError &error) {
			error.printMessage();
			goto cleanup;
		}


		UE_LOG(LogTemp, Warning, TEXT("  Input Port #%d : %s\n"), i+1, *FString(portName.c_str()));
	}
	// RtMidiOut constructor
	try {
		midiout = new RtMidiOut();
	}
	catch (RtMidiError &error) {
		error.printMessage();
		exit(EXIT_FAILURE);
	}
	// Check outputs.
	nPorts = midiout->getPortCount();
	UE_LOG(LogTemp, Warning, TEXT("\nThere are %d MIDI output ports available.\n"), nPorts);
	for (unsigned int i = 0; i<nPorts; i++) {
		try {
			portName = midiout->getPortName(i);
		}
		catch (RtMidiError &error) {
			error.printMessage();
			goto cleanup;
		}
		UE_LOG(LogTemp, Warning, TEXT("  Output Port #%d : %s\n"), i + 1, *FString(portName.c_str()));
	}
	// Clean up
cleanup:
	delete midiin;
	delete midiout;

	if (!GetWorld()) return;

	APlayerController* myplayer = GetWorld()->GetFirstPlayerController();

	if (!myplayer)
		return;

	if (wMainMenu&&GetWorld()->GetCurrentLevel()->GetOutermost()->GetName().Contains("MainMenu"))
	{
		if (!main_menu)
		{
			main_menu = CreateWidget<UUserWidget>(GetWorld(), wMainMenu);
		}

		if (main_menu&&!main_menu->GetIsVisible())
		{
			main_menu->AddToViewport();
		}


		myplayer->bShowMouseCursor = true;

		FInputModeUIOnly inputmode;
		inputmode.SetLockMouseToViewport(true);
		inputmode.SetWidgetToFocus(main_menu->GetCachedWidget());

		myplayer->SetInputMode(inputmode);
	}
	else {

		if (wMainMenu&&main_menu)
		{
			main_menu->RemoveFromViewport();
		}
		myplayer->bShowMouseCursor = false;

		FInputModeGameOnly inputmode;

		myplayer->SetInputMode(inputmode);
	}


}


void AMusicSuiteGameMode::PostLogin(APlayerController *cont)
{
	Super::PostLogin(cont);


}