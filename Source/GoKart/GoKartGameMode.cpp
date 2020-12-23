// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "GoKartGameMode.h"
#include "GoKartPawn.h"
#include "GoKartHud.h"

AGoKartGameMode::AGoKartGameMode()
{
	DefaultPawnClass = AGoKartPawn::StaticClass();
	HUDClass = AGoKartHud::StaticClass();
}
