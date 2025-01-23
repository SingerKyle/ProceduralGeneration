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
	Floor(FVector Origin, FVector2D, float, float, FVector2D);
	~Floor();

	void Partition();
	int32 SelectOrientation();
	bool ShouldSplitNode(TSharedPtr<class FloorNode> InNode, ESplitOrientation Orientation);
	bool SplitAttempt(TSharedPtr<class FloorNode> InNode);
	void SplitHorizontal(TSharedPtr<class FloorNode> InA, TSharedPtr<class FloorNode> InB, TSharedPtr<class FloorNode> InC);
	void SplitVertical(TSharedPtr<class FloorNode> InA, TSharedPtr<class FloorNode> InB, TSharedPtr<class FloorNode> InC);

	FORCEINLINE TArray<TSharedPtr<FloorNode>> GetPartitionedFloor() const {	return PartitionedFloor; }
	FORCEINLINE void ClearPartitionedFloor() { PartitionedFloor.Empty(); }

	void DrawFloorNodes(class UWorld* World);
	void DrawFloorNode(UWorld* World, FCornerCoordinates Coordinates);
private:
	TArray<TSharedPtr<FloorNode>> FloorNodeStack;
	TArray<TSharedPtr<FloorNode>> PartitionedFloor;
	
	int32 FloorGridSizeX;
	int32 FloorGridSizeY;

	int32 RoomMinX;
	int32 RoomMinY;

	float GridLength;

	float SplitRate;
	
	FVector Origin;
};
