// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PDFImporter.h"
#include "GhostscriptCore.h"
#include "PDFiumCore.h"

#define LOCTEXT_NAMESPACE "FPDFImporterModule"

void FPDFImporterModule::StartupModule()
{
	GhostscriptCore = MakeShareable(new FGhostscriptCore());
	PDFiumCore = MakeShareable(new FPDFiumCore());
}

void FPDFImporterModule::ShutdownModule()
{
	GhostscriptCore.Reset();
	PDFiumCore.Reset();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPDFImporterModule, PDFImporter)