// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "IDetailCustomization.h"

// See sky light details in the engine code

class AVoxelMagicaVoxelImporter;

class FVoxelMagicaVoxelImporterDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();
private:
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

	FReply OnCreate();

private:
	/** The selected landscape modifier */
	TWeakObjectPtr<AVoxelMagicaVoxelImporter> Importer;
};
