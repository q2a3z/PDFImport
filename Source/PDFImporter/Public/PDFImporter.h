// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FPDFImporterModule : public IModuleInterface
{
private:
	// An instance with the same life as this module class
	TSharedPtr<class FGhostscriptCore> GhostscriptCore;
	TSharedPtr<class FPDFiumCore> PDFiumCore;

public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	// Get an instance of GhostscriptCore
	TSharedPtr<class FGhostscriptCore> GetGhostscriptCore() const { return GhostscriptCore; }
	TSharedPtr<class FPDFiumCore> GetPDFiumCore() const { return PDFiumCore; }
};

DEFINE_LOG_CATEGORY_STATIC(PDFImporter, Log, All);