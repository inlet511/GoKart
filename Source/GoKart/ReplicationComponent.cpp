// Fill out your copyright notice in the Description page of Project Settings.


#include "ReplicationComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UReplicationComponent::UReplicationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	SetIsReplicated(true);
	// ...
}

void UReplicationComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UReplicationComponent, ServerState);
}

// Called when the game starts
void UReplicationComponent::BeginPlay()
{
	Super::BeginPlay();
	MovementComponent = GetOwner()->FindComponentByClass<UVehicleMovement>();
	
}


// Called every frame
void UReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MovementComponent == nullptr) return;

	// AutonomouseProxy
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		FVehiclePawnMove Move = MovementComponent->CreateMove(DeltaTime);
		MovementComponent->SimulateMove(Move);
		UnacknowledgedMoves.Add(Move);
		Server_SendMove(Move);
	}

	// 服务端控制的 Authority 
	if (GetOwnerRole() == ROLE_Authority && Cast<APawn>(GetOwner())->IsLocallyControlled())
	{
		FVehiclePawnMove Move = MovementComponent->CreateMove(DeltaTime);
		Server_SendMove(Move);
	}

	// Simulated Proxy
	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		MovementComponent->SimulateMove(ServerState.LastMove);
	}
}


void UReplicationComponent::Server_SendMove_Implementation(FVehiclePawnMove Move)
{
	if (MovementComponent == nullptr) return;
	MovementComponent->SimulateMove(Move);
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}

bool UReplicationComponent::Server_SendMove_Validate(FVehiclePawnMove Move)
{
	return true; // TODO 
}


void UReplicationComponent::OnRep_ServerState()
{
	GetOwner()->SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);
	ClearMoves(ServerState.LastMove);
	for (const FVehiclePawnMove& Move : UnacknowledgedMoves)
	{
		MovementComponent->SimulateMove(Move);
	}
}



void UReplicationComponent::ClearMoves(const FVehiclePawnMove& Move)
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
