// Fill out your copyright notice in the Description page of Project Settings.


#include "VehiclePawn.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"

// Sets default values
AVehiclePawn::AVehiclePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MovementComp = CreateDefaultSubobject<UVehicleMovement>(TEXT("Movement Component"));	
}

// Called when the game starts or when spawned
void AVehiclePawn::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		NetUpdateFrequency = 1.0f;
	}
}

void AVehiclePawn::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVehiclePawn, ServerState);
}

FString GetEnumText(ENetRole Role)
{
	switch (Role)
	{
	case ROLE_None:
		return "None";		
	case ROLE_SimulatedProxy:
		return "Simulated";
	case ROLE_AutonomousProxy:
		return "AutonomousProxy";
	case ROLE_Authority:
		return "Authority";
	default:
		return "Error";
	}
}

// Called every frame
void AVehiclePawn::Tick(float DeltaTime)
{

	Super::Tick(DeltaTime);	

	if (MovementComp == nullptr) return;

	// AutonomouseProxy
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		FVehiclePawnMove Move = MovementComp->CreateMove(DeltaTime);
		MovementComp->SimulateMove(Move);
		UnacknowledgedMoves.Add(Move);
		Server_SendMove(Move);
	}

	// 服务端控制的 Authority 
	if (GetLocalRole() == ROLE_Authority && IsLocallyControlled())
	{
		FVehiclePawnMove Move = MovementComp->CreateMove(DeltaTime);
		Server_SendMove(Move);
	}

	// Simulated Proxy
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		MovementComp->SimulateMove(ServerState.LastMove);
	}

	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(GetLocalRole()), this, FColor::White, DeltaTime);
}



void AVehiclePawn::Server_SendMove_Implementation(FVehiclePawnMove Move)
{
	if (MovementComp == nullptr) return;
	MovementComp->SimulateMove(Move);
	ServerState.LastMove = Move;
	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = MovementComp->GetVelocity();
}

bool AVehiclePawn::Server_SendMove_Validate(FVehiclePawnMove Move)
{
	return true; // TODO 
}

// Called to bind functionality to input
void AVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AVehiclePawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVehiclePawn::MoveRight);
}


void AVehiclePawn::OnRep_ServerState()
{
	SetActorTransform(ServerState.Transform);
	MovementComp->SetVelocity(ServerState.Velocity);
	ClearMoves(ServerState.LastMove);
	for (const FVehiclePawnMove& Move : UnacknowledgedMoves)
	{
		MovementComp->SimulateMove(Move);
	}
}


void AVehiclePawn::MoveForward(float Value)
{
	if (MovementComp == nullptr) return;
	MovementComp->SetThrottle(Value);
}

void AVehiclePawn::MoveRight(float Value)
{
	if (MovementComp == nullptr) return;
	MovementComp->SetSteering(Value);
}


void AVehiclePawn::ClearMoves(const FVehiclePawnMove& Move)
{
	TArray<FVehiclePawnMove> NewMoves;

	for (const FVehiclePawnMove& item : UnacknowledgedMoves)
	{
		if (item.Time > Move.Time)
		{
			NewMoves.Add(item);
		}
	}

	UnacknowledgedMoves = NewMoves;
}
