// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "VehicleMovement.h"
#include "VehiclePawn.generated.h"



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


UCLASS()
class GOKART_API AVehiclePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AVehiclePawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;	

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere)
	UVehicleMovement* MovementComp;

private:


	UPROPERTY(ReplicatedUsing= OnRep_ServerState)
	FVehiclePawnState ServerState;

	UFUNCTION()
	void OnRep_ServerState();

	void MoveForward(float Value);
	void MoveRight(float Value);

	UFUNCTION(Server,Reliable,WithValidation)
	void Server_SendMove(FVehiclePawnMove Move);
	void ClearMoves(const FVehiclePawnMove& Move);

	TArray<FVehiclePawnMove> UnacknowledgedMoves;

	
};
