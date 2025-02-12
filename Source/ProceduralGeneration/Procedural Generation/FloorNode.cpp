#include "FloorNode.h"

int32 FloorNode::NodeCount = 0;

FloorNode::FloorNode()
{
	++NodeCount;
	UE_LOG(LogTemp, Warning, TEXT("Create Floor Node"));
}

FloorNode::FloorNode(const FCornerCoordinates& Coordinates)
{
	CornerCoordinates.UpperLeftX = Coordinates.UpperLeftX;
	CornerCoordinates.UpperLeftY = Coordinates.UpperLeftY;
	CornerCoordinates.LowerRightX = Coordinates.LowerRightX;
	CornerCoordinates.LowerRightY = Coordinates.LowerRightY;

	UE_LOG(LogTemp, Warning, TEXT("Create Floor Node"));
}

FloorNode::~FloorNode()
{
	--NodeCount;
	UE_LOG(LogTemp, Warning, TEXT("Destroy Floor Node"));
}
