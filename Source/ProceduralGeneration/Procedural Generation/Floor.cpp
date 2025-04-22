#include "Floor.h"

#include "FloorNode.h"
#include "DrawDebugHelpers.h"

Floor::Floor()
{
	
}

Floor::Floor(FVector Origin_, FVector2D GridSize, float FloorSize, float Split, FVector2D MinBounds, bool bUseMaxSize)
{
	Origin = Origin_;
	
	FloorGridSizeX = GridSize.X;
	FloorGridSizeY = GridSize.Y;

	RoomMinX = MinBounds.X;
	RoomMinY = MinBounds.Y;

	RoomMaxX = GridSize.X - MinBounds.X;
	RoomMaxY = GridSize.Y - MinBounds.Y;

	GridLength = FloorSize;

	SplitRate = Split;

	bShouldCheckMax = bUseMaxSize;
}

Floor::~Floor()
{
	
}

void Floor::Reinitialise(FVector Origin_, FVector2D GridSize, float FloorSize, float Split, FVector2D MinBounds, bool bUseMaxSize)
{
	Origin = Origin_;
	
	FloorGridSizeX = GridSize.X;
	FloorGridSizeY = GridSize.Y;

	RoomMinX = MinBounds.X;
	RoomMinY = MinBounds.Y;

	RoomMaxX = GridSize.X - MinBounds.X;
	RoomMaxY = GridSize.Y - MinBounds.Y;

	GridLength = FloorSize;

	SplitRate = Split;

	bShouldCheckMax = bUseMaxSize;
}

void Floor::Partition()
{
	
	FCornerCoordinates CornerCoordinatesA = {0,0, FloorGridSizeX, FloorGridSizeY};
	
	FloorNodeStack.Push(TSharedPtr<FloorNode>(new FloorNode(CornerCoordinatesA)));

	while (FloorNodeStack.Num() > 0)
	{
		TSharedPtr<FloorNode> A = FloorNodeStack.Pop();
		//UE_LOG(LogTemp, Warning, TEXT("Pop FloorNode Off Stack"));

		bool bNodeWasSplit = SplitAttempt(A);

		if(!bNodeWasSplit) // if shouldn't be split - push to partitioned floor stack
		{
			if (bShouldCheckMax)
			{
				int Width = A->GetCornerCoordinates().LowerRightX - A->GetCornerCoordinates().UpperLeftX;
				int Height = A->GetCornerCoordinates().LowerRightY - A->GetCornerCoordinates().UpperLeftY;

				if (Width > RoomMaxX || Height > RoomMaxY)
				{
					UE_LOG(LogTemp, Warning, TEXT("Too Big"));
					FloorNodeStack.Push(A); // Push back to be split again
					continue;
				}
			}

			if (!IsPartitionValid(A))
			{
				FloorNodeStack.Push(A);
				continue;
			}
			
			PartitionedFloor.Push(A);
			//UE_LOG(LogTemp, Warning, TEXT("Push FloorNode to Partition Stack"));
		}
		
		//UE_LOG(LogTemp, Warning, TEXT("Nodes in Existance in loop: %d"), FloorNode::GetNodeCount());
		//UE_LOG(LogTemp, Warning, TEXT("Nodes in Existance in Stack: %d"), FloorNodeStack.Num());
	}
}

// This function will decide if the node SHOULD split
int32 Floor::SelectOrientation()
{
	return FMath::RandRange(0,1); // horizontal (0) or vertical (1)
}

// Will always return true if the node is larger than the minimum size (1x1)
bool Floor::ShouldSplitNode(TSharedPtr<FloorNode> InNode, ESplitOrientation Orientation)
{
	FCornerCoordinates CornerCoordinates = InNode->GetCornerCoordinates();
	
	if (Orientation == ESplitOrientation::ESO_Horizontal)
	{
		int32 FloorLength = CornerCoordinates.LowerRightY - CornerCoordinates.UpperLeftY;
		float SplitChance = (float)FloorLength / (float)FloorGridSizeY;

		SplitChance *= SplitRate;
		
		float ShouldSplit = FMath::RandRange(0,1);
		if (ShouldSplit > SplitChance)
		{
			return false;
		}

		if (FloorLength >= 2 * RoomMinY)
		{
			return true;
		}
	}
	else//if (Orientation == ESplitOrientation::ESO_Vertical)
	{
		int32 FloorWidth = CornerCoordinates.LowerRightX - CornerCoordinates.UpperLeftX;
		float SplitChance = (float)FloorWidth / (float)FloorGridSizeX;

		SplitChance *= SplitRate;
		
		float ShouldSplit = FMath::RandRange(0,1);
		if (ShouldSplit > SplitChance)
		{
			return false;
		}
		
		if (FloorWidth >= 2 * RoomMinX)
		{
			return true;
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("False in ShouldSplit"));
	return false;
}

bool Floor::SplitAttempt(TSharedPtr<FloorNode> InNode)
{
	
	int32 OrientationSelection = SelectOrientation();
	
	if (OrientationSelection == 0)
	{
		// Split horizontally
		if(ShouldSplitNode(InNode, ESplitOrientation::ESO_Horizontal))
		{
			TSharedPtr<FloorNode> B(new FloorNode());
			TSharedPtr<FloorNode> C(new FloorNode());
			
			SplitHorizontal(InNode, B, C);
			return true;
		}
		else if (ShouldSplitNode(InNode, ESplitOrientation::ESO_Vertical))
		{
			TSharedPtr<FloorNode> B(new FloorNode());
			TSharedPtr<FloorNode> C(new FloorNode());
			
			SplitVertical(InNode, B, C);
			return true;
		}
	}
	else// if (OrientationSelection == 1)
	{
		// Split vertically
		if(ShouldSplitNode(InNode, ESplitOrientation::ESO_Vertical))
		{
			TSharedPtr<FloorNode> B(new FloorNode());
			TSharedPtr<FloorNode> C(new FloorNode());
			
			SplitVertical(InNode, B, C);
			return true;
		}
		else if (ShouldSplitNode(InNode, ESplitOrientation::ESO_Horizontal))
		{
			TSharedPtr<FloorNode> B(new FloorNode());
			TSharedPtr<FloorNode> C(new FloorNode());
			
			SplitHorizontal(InNode, B, C);
			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("False in SplitAttempt"));
	return false;
}

void Floor::SplitHorizontal(TSharedPtr<FloorNode> InA, TSharedPtr<FloorNode> InB, TSharedPtr<FloorNode> InC)
{
	// add room min to not split on the edge!
	int32 SplitPointY = FMath::RandRange(InA->GetCornerCoordinates().UpperLeftY + RoomMinY, InA->GetCornerCoordinates().LowerRightY - RoomMinY);

	FCornerCoordinates CornerCoordinatesB;
	CornerCoordinatesB.UpperLeftX = InA->GetCornerCoordinates().UpperLeftX;
	CornerCoordinatesB.UpperLeftY = InA->GetCornerCoordinates().UpperLeftY;
	CornerCoordinatesB.LowerRightY = SplitPointY;
	CornerCoordinatesB.LowerRightX = InA->GetCornerCoordinates().LowerRightX;

	InB->SetCornerCoordinates(CornerCoordinatesB);
	FloorNodeStack.Push(InB);

	FCornerCoordinates CornerCoordinatesC;
	CornerCoordinatesC.UpperLeftX = InA->GetCornerCoordinates().UpperLeftX;
	CornerCoordinatesC.UpperLeftY = SplitPointY;
	CornerCoordinatesC.LowerRightY = InA->GetCornerCoordinates().LowerRightY;
	CornerCoordinatesC.LowerRightX = InA->GetCornerCoordinates().LowerRightX;

	InC->SetCornerCoordinates(CornerCoordinatesC);
	FloorNodeStack.Push(InC);
}

void Floor::SplitVertical(TSharedPtr<FloorNode> InA, TSharedPtr<FloorNode> InB, TSharedPtr<FloorNode> InC)
{
	int32 SplitPointX = FMath::RandRange(InA->GetCornerCoordinates().UpperLeftX + RoomMinX, InA->GetCornerCoordinates().LowerRightX - RoomMinX);

	// UpperLeftX and Y are unchanged but the LowerRight X changes as the split is vertical.
	
	FCornerCoordinates CornerCoordinatesB;
	CornerCoordinatesB.UpperLeftX = InA->GetCornerCoordinates().UpperLeftX;
	CornerCoordinatesB.UpperLeftY = InA->GetCornerCoordinates().UpperLeftY;
	CornerCoordinatesB.LowerRightY = InA->GetCornerCoordinates().LowerRightY;
	CornerCoordinatesB.LowerRightX = SplitPointX;

	InB->SetCornerCoordinates(CornerCoordinatesB);
	FloorNodeStack.Push(InB);

	FCornerCoordinates CornerCoordinatesC;
	CornerCoordinatesC.UpperLeftX = SplitPointX;
	CornerCoordinatesC.UpperLeftY = InA->GetCornerCoordinates().UpperLeftY;
	CornerCoordinatesC.LowerRightY = InA->GetCornerCoordinates().LowerRightY;
	CornerCoordinatesC.LowerRightX = InA->GetCornerCoordinates().LowerRightX;

	InC->SetCornerCoordinates(CornerCoordinatesC);
	FloorNodeStack.Push(InC);
}

bool Floor::IsPartitionValid(TSharedPtr<FloorNode> Node)
{
	FCornerCoordinates Coordinates = Node->GetCornerCoordinates();

	int32 Width = Coordinates.LowerRightX - Coordinates.UpperLeftX;
	int32 Height = Coordinates.LowerRightY - Coordinates.UpperLeftY;

	// Check against maximum bounds
	return (Width <= RoomMaxX && Height <= RoomMaxY);
}

void Floor::DrawFloorNodes(UWorld* World)
{
	for(int32 i = 0; i < PartitionedFloor.Num(); i++)
	{
		FCornerCoordinates Coordinates = PartitionedFloor[i]->GetCornerCoordinates();

		const FVector UpperLeft(Coordinates.UpperLeftX * GridLength + Origin.X, Coordinates.UpperLeftY * GridLength + Origin.Y,  i * 50.f); 
		const FVector UpperRight(Coordinates.LowerRightX * GridLength + Origin.X, Coordinates.UpperLeftY * GridLength + Origin.Y, i * 50.f);
		const FVector LowerLeft(Coordinates.UpperLeftX * GridLength + Origin.X, Coordinates.LowerRightY * GridLength + Origin.Y, i * 50.f);
		const FVector LowerRight(Coordinates.LowerRightX * GridLength + Origin.X, Coordinates.LowerRightY * GridLength + Origin.Y, i * 50.f);
	
		DrawDebugLine(World, UpperLeft, UpperRight, FColor::Magenta, true, -1.f, 0, 3.5f);
		DrawDebugLine(World, UpperLeft, LowerLeft, FColor::Magenta, true, -1.f, 0, 1.5f);
		DrawDebugLine(World, LowerLeft, LowerRight, FColor::Magenta, true, -1.f, 0, 1.5f);
		DrawDebugLine(World, UpperRight, LowerRight, FColor::Magenta, true, -1.f, 0, 1.5f);
		
		//DrawFloorNode(World, Coordinates);
	}
}

void Floor::DrawFloorNode(UWorld* World, FCornerCoordinates Coordinates)
{
	const FVector UpperLeft(Coordinates.UpperLeftX * GridLength, Coordinates.UpperLeftY * GridLength,  200.f); 
	const FVector UpperRight(Coordinates.LowerRightX * GridLength, Coordinates.UpperLeftY * GridLength, 200.f);
	const FVector LowerLeft(Coordinates.UpperLeftX * GridLength, Coordinates.LowerRightY * GridLength, 200.f);
	const FVector LowerRight(Coordinates.LowerRightX * GridLength, Coordinates.LowerRightY * GridLength, 200.f);
	
	DrawDebugLine(World, UpperLeft, UpperRight, FColor::Magenta, true, -1.f, 0, 3.5f);
	DrawDebugLine(World, UpperLeft, LowerLeft, FColor::Magenta, true, -1.f, 0, 1.5f);
	DrawDebugLine(World, LowerLeft, LowerRight, FColor::Magenta, true, -1.f, 0, 1.5f);
	DrawDebugLine(World, UpperRight, LowerRight, FColor::Magenta, true, -1.f, 0, 1.5f);
}
