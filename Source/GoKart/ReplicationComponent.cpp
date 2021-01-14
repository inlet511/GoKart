// Fill out your copyright notice in the Description page of Project Settings.


#include "ReplicationComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UReplicationComponent::UReplicationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
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
	SetIsReplicated(true);
	MovementComponent = GetOwner()->FindComponentByClass<UVehicleMovement>();
}


// Called every frame
void UReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MovementComponent == nullptr) return;

	FVehiclePawnMove LastMove = MovementComponent->GetLastMove();

	// AutonomouseProxy
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		
		UnacknowledgedMoves.Add(LastMove);
		Server_SendMove(LastMove);
	}

	// 服务端控制的 Authority 
	if (GetOwnerRole() == ROLE_Authority && Cast<APawn>(GetOwner())->IsLocallyControlled())
	{
		UpdateServerState(LastMove);
	}

	// Simulated Proxy
	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		ClientTick(DeltaTime);
	}
}


void UReplicationComponent::UpdateServerState(const FVehiclePawnMove& Move)
{
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}

void UReplicationComponent::Server_SendMove_Implementation(FVehiclePawnMove Move)
{
	if (MovementComponent == nullptr) return;
	MovementComponent->SimulateMove(Move);
	UpdateServerState(Move);
}

bool UReplicationComponent::Server_SendMove_Validate(FVehiclePawnMove Move)
{
	return true; // TODO 
}


void UReplicationComponent::OnRep_ServerState()
{
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		AutonomousProxy_OnRep_ServerState();
	}
	else if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		SimulatedProxy_OnRep_ServerState();
	}	
}


void UReplicationComponent::SimulatedProxy_OnRep_ServerState()
{
	if (MovementComponent == nullptr) return;
	CLientTimeBetweenLastUpdates = ClientTimeSinceUpdate;
	ClientTimeSinceUpdate = 0;

	ClientStartTransform = GetOwner()->GetActorTransform();
	ClientStartVelocity = MovementComponent->GetVelocity();
}

void UReplicationComponent::AutonomousProxy_OnRep_ServerState()
{
	if (MovementComponent == nullptr) return;
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

void UReplicationComponent::ClientTick(float DeltaTime)
{
	ClientTimeSinceUpdate += DeltaTime;

	if (CLientTimeBetweenLastUpdates < KINDA_SMALL_NUMBER) return;
	if (MovementComponent == nullptr) return;

	auto TargetLocation = ServerState.Transform.GetLocation();
	float LerpRatio = ClientTimeSinceUpdate / CLientTimeBetweenLastUpdates;
	//UE_LOG(LogTemp, Warning, TEXT("ClientTimeSinceUpdate:%f"), ClientTimeSinceUpdate);
	//UE_LOG(LogTemp, Warning, TEXT("CLientTimeBetweenLastUpdates:%f"), CLientTimeBetweenLastUpdates);
	//UE_LOG(LogTemp, Warning, TEXT("----------------------------------"));

	if (LerpRatio > 1)
		LerpRatio = 1;
	FVector StartLocation = ClientStartTransform.GetLocation();
	float VelocityToDerivative = CLientTimeBetweenLastUpdates * 100;
	FVector StartDerivative = ClientStartVelocity  * VelocityToDerivative;
	FVector TargetDerivative = ServerState.Velocity *VelocityToDerivative;

	FVector NewLocation = FMath::CubicInterp(StartLocation,StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	GetOwner()->SetActorLocation(NewLocation);

	//UE_LOG(LogTemp, Warning, TEXT("StartLocation:%d,%d,%d"), StartLocation.X, StartLocation.Y, StartLocation.Z);
	//UE_LOG(LogTemp, Warning, TEXT("LerpRatio:%d"), LerpRatio);


	FVector NewDerivative = FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	FVector NewVelocity = NewDerivative / VelocityToDerivative;
	MovementComponent->SetVelocity(NewVelocity);

	auto TargetRotation = ServerState.Transform.GetRotation();
	FQuat StartRotation = ClientStartTransform.GetRotation();
	FQuat NewRotation = FQuat::Slerp(StartRotation, TargetRotation, LerpRatio);
	GetOwner()->SetActorRotation(NewRotation);
}
