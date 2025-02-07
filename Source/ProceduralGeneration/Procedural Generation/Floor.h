#pragma once
#include "FloorNode.h"

// scoped Enum
enum class ESplitOrientation
{
	ESO_Horizontal,
	ESO_Vertical
};

class FloorNode;

class Floor
{
public:
	Floor();
	Floor(FVector Origin, FVector2D, float, float, FVector2D, bool);
	~Floor();

	void Reinitialise(FVector Origin, FVector2D, float, float, FVector2D, bool);

	void Partition();
	int32 SelectOrientation();
	bool ShouldSplitNode(TSharedPtr<class FloorNode> InNode, ESplitOrientation Orientation);
	bool SplitAttempt(TSharedPtr<class FloorNode> InNode);
	void SplitHorizontal(TSharedPtr<class FloorNode> InA, TSharedPtr<class FloorNode> InB, TSharedPtr<class FloorNode> InC);
	void SplitVertical(TSharedPtr<class FloorNode> InA, TSharedPtr<class FloorNode> InB, TSharedPtr<class FloorNode> InC);

	FORCEINLINE TArray<TSharedPtr<FloorNode>> GetPartitionedFloor() const {	return PartitionedFloor; }
	FORCEINLINE void ClearPartitionedFloor() { PartitionedFloor.Empty(); }

	bool IsPartitionValid(TSharedPtr<FloorNode> Node);
	
	void DrawFloorNodes(class UWorld* World);
	void DrawFloorNode(UWorld* World, FCornerCoordinates Coordinates);

private:
	TArray<TSharedPtr<FloorNode>> FloorNodeStack;
	TArray<TSharedPtr<FloorNode>> PartitionedFloor;
	
	int32 FloorGridSizeX;
	int32 FloorGridSizeY;

	int32 RoomMinX;
	int32 RoomMinY;

	int32 RoomMaxX;
	int32 RoomMaxY;

	float GridLength;

	float SplitRate;
	
	FVector Origin;

	bool bShouldCheckMax;
};
