// Copyright 2018 Phyronnaz

#include "VoxelGraphNode_Base.h"
#include "EdGraph/EdGraphSchema.h"
#include "VoxelGraphSchema.h"
#include "EdGraph/EdGraphPin.h"

void UVoxelGraphNode_Base::GetOutputPins(TArray<UEdGraphPin*>& OutOutputPins)
{
	OutOutputPins.Empty();

	for (int32 PinIndex = 0; PinIndex < Pins.Num(); PinIndex++)
	{
		if (Pins[PinIndex]->Direction == EGPD_Output)
		{
			OutOutputPins.Add(Pins[PinIndex]);
		}
	}
}

void UVoxelGraphNode_Base::GetInputPins(TArray<UEdGraphPin*>& OutInputPins)
{
	OutInputPins.Empty();

	for (int32 PinIndex = 0; PinIndex < Pins.Num(); PinIndex++)
	{
		if (Pins[PinIndex]->Direction == EGPD_Input)
		{
			OutInputPins.Add(Pins[PinIndex]);
		}
	}
}

UEdGraphPin* UVoxelGraphNode_Base::GetInputPin(int32 InputIndex)
{
	check(InputIndex >= 0 && InputIndex < GetInputCount());

	for (int32 PinIndex = 0, FoundInputs = 0; PinIndex < Pins.Num(); PinIndex++)
	{
		if (Pins[PinIndex]->Direction == EGPD_Input)
		{
			if (InputIndex == FoundInputs)
			{
				return Pins[PinIndex];
			}
			else
			{
				FoundInputs++;
			}
		}
	}

	return NULL;
}

UEdGraphPin* UVoxelGraphNode_Base::GetOutputPin(int32 OutputIndex)
{
	check(OutputIndex >= 0 && OutputIndex < GetOutputCount());

	for (int32 PinIndex = 0, FoundOutputs = 0; PinIndex < Pins.Num(); PinIndex++)
	{
		if (Pins[PinIndex]->Direction == EGPD_Output)
		{
			if (OutputIndex == FoundOutputs)
			{
				return Pins[PinIndex];
			}
			else
			{
				FoundOutputs++;
			}
		}
	}

	return NULL;
}

int32 UVoxelGraphNode_Base::GetInputCount() const
{
	int32 InputCount = 0;

	for (int32 PinIndex = 0, FoundInputs = 0; PinIndex < Pins.Num(); PinIndex++)
	{
		if (Pins[PinIndex]->Direction == EGPD_Input)
		{
			InputCount++;
		}
	}

	return InputCount;
}

int32 UVoxelGraphNode_Base::GetOutputCount() const
{
	int32 OutputCount = 0;

	for (int32 PinIndex = 0, FoundInputs = 0; PinIndex < Pins.Num(); PinIndex++)
	{
		if (Pins[PinIndex]->Direction == EGPD_Output)
		{
			OutputCount++;
		}
	}

	return OutputCount;
}

void UVoxelGraphNode_Base::InsertNewNode(UEdGraphPin* FromPin, UEdGraphPin* NewLinkPin, TSet<UEdGraphNode*>& OutNodeList)
{
	const UVoxelGraphSchema* Schema = CastChecked<UVoxelGraphSchema>(GetSchema());

	// The pin we are creating from already has a connection that needs to be broken. We want to "insert" the new node in between, so that the output of the new node is hooked up too
	UEdGraphPin* OldLinkedPin = FromPin->LinkedTo[0];
	check(OldLinkedPin);

	FromPin->BreakAllPinLinks();

	// Hook up the old linked pin to the first valid output pin on the new node
	for (int32 OutpinPinIdx = 0; OutpinPinIdx < Pins.Num(); OutpinPinIdx++)
	{
		UEdGraphPin* OutputExecPin = Pins[OutpinPinIdx];
		check(OutputExecPin);
		if (ECanCreateConnectionResponse::CONNECT_RESPONSE_MAKE == Schema->CanCreateConnection(OldLinkedPin, OutputExecPin).Response)
		{
			if (Schema->TryCreateConnection(OldLinkedPin, OutputExecPin))
			{
				OutNodeList.Add(OldLinkedPin->GetOwningNode());
				OutNodeList.Add(this);
			}
			break;
		}
	}

	if (Schema->TryCreateConnection(FromPin, NewLinkPin))
	{
		OutNodeList.Add(FromPin->GetOwningNode());
		OutNodeList.Add(this);
	}
}

void UVoxelGraphNode_Base::AllocateDefaultPins()
{
	check(Pins.Num() == 0);

	CreateInputPins();
	CreateOutputPins();
}

void UVoxelGraphNode_Base::ReconstructNode()
{
	// Break any links to 'orphan' pins
	for (auto Pin : Pins)
	{
		for (auto OtherPin : Pin->LinkedTo)
		{
			// If we are linked to a pin that its owner doesn't know about, break that link
			if (!OtherPin->GetOwningNode()->Pins.Contains(OtherPin))
			{
				Pin->LinkedTo.Remove(OtherPin);
			}
		}
	}

	// Store the old Input and Output pins
	TArray<UEdGraphPin*> OldInputPins;
	GetInputPins(OldInputPins);
	TArray<UEdGraphPin*> OldOutputPins;
	GetOutputPins(OldOutputPins);

	// Move the existing pins to a saved array
	TArray<UEdGraphPin*> OldPins(Pins);
	Pins.Reset();

	// Recreate the new pins
	AllocateDefaultPins();

	// Get new Input and Output pins
	TArray<UEdGraphPin*> NewInputPins;
	GetInputPins(NewInputPins);
	TArray<UEdGraphPin*> NewOutputPins;
	GetOutputPins(NewOutputPins);

	for (int32 PinIndex = 0; PinIndex < OldInputPins.Num(); PinIndex++)
	{
		if (PinIndex < NewInputPins.Num())
		{
			NewInputPins[PinIndex]->MovePersistentDataFromOldPin(*OldInputPins[PinIndex]);
		}
	}

	for (int32 PinIndex = 0; PinIndex < OldOutputPins.Num(); PinIndex++)
	{
		if (PinIndex < NewOutputPins.Num())
		{
			NewOutputPins[PinIndex]->MovePersistentDataFromOldPin(*OldOutputPins[PinIndex]);
		}
	}

	// Throw away the original pins
	for (UEdGraphPin* OldPin : OldPins)
	{
		OldPin->Modify();
		UEdGraphNode::DestroyPin(OldPin);
	}
}

void UVoxelGraphNode_Base::AutowireNewNode(UEdGraphPin* FromPin)
{
	if (FromPin != NULL)
	{
		const UVoxelGraphSchema* Schema = CastChecked<UVoxelGraphSchema>(GetSchema());

		TSet<UEdGraphNode*> NodeList;

		// auto-connect from dragged pin to first compatible pin on the new node
		for (int32 i = 0; i < Pins.Num(); i++)
		{
			UEdGraphPin* Pin = Pins[i];
			check(Pin);
			FPinConnectionResponse Response = Schema->CanCreateConnection(FromPin, Pin);

			if (ECanCreateConnectionResponse::CONNECT_RESPONSE_MAKE == Response.Response)
			{
				if (Schema->TryCreateConnection(FromPin, Pin))
				{
					NodeList.Add(FromPin->GetOwningNode());
					NodeList.Add(this);
				}
				break;
			}
			else if (ECanCreateConnectionResponse::CONNECT_RESPONSE_BREAK_OTHERS_A == Response.Response)
			{
				InsertNewNode(FromPin, Pin, NodeList);
				break;
			}
		}

		// Send all nodes that received a new pin connection a notification
		for (auto It = NodeList.CreateConstIterator(); It; ++It)
		{
			UEdGraphNode* Node = (*It);
			Node->NodeConnectionListChanged();
		}
	}
}

bool UVoxelGraphNode_Base::CanCreateUnderSpecifiedSchema(const UEdGraphSchema* Schema) const
{
	return Schema->IsA(UVoxelGraphSchema::StaticClass());
}
