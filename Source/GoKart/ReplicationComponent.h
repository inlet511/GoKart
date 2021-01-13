// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VehicleMovement.h"
#include "ReplicationComponent.generated.h"

USTRUCT()
struct FVehiclePawnState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVehiclePawnMove LastMove;

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	FVector Velocity;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GOKART_API UReplicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UReplicationComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	TArray<FVehiclePawnMove> UnacknowledgedMoves;

	void UpdateServerState(const FVehiclePawnMove& Move);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FVehiclePawnMove Move);

	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FVehiclePawnState ServerState;

	UFUNCTION()
	void OnRep_ServerState();
	void SimulatedProxy_OnRep_ServerState();
	void AutonomousProxy_OnRep_ServerState();

	void ClearMoves(const FVehiclePawnMove& Move);

	UPROPERTY()
	UVehicleMovement* MovementComponent;

	float ClientTimeSinceUpdate;
	float CLientTimeBetweenLastUpdates;
	FVector ClientStartLocation;
	FQuat ClientStartRotation;

	void ClientTick(float DeltaTime);
};
