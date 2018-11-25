// Fill out your copyright notice in the Description page of Project Settings.

#include "MusicSuite.h"
#include "IUsable.h"


void IUsable::OnUsed(APawn *user)
{

}

void IUsable::DrawHUD(UCanvas *Canvas, APawn *user)
{

}

//////////////////////////////////////////////////////////////////////////
// ToStringInterface

UUsable::UUsable(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}