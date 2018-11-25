// Fill out your copyright notice in the Description page of Project Settings.

#include "MusicSuite.h"
#include "MusicSuiteHUD.h"
#include "MusicSuiteCharacter.h"



void AMusicSuiteHUD::DrawHUD()
{
	Super::DrawHUD();


	AMusicSuiteCharacter* player = dynamic_cast<AMusicSuiteCharacter*>(GetOwningPawn()); // grab the  player

	if (player && player->UseFocus != 0)// check that is correct
	{
		player->UseFocus->DrawHUD(Canvas, GetOwningPawn()); // call the DrawHUD function so the Usable actor can draw itself
	}
}