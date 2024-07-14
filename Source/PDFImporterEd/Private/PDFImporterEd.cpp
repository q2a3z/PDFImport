// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PDFImporterEd.h"
#include "AssetTypeActions_PDF.h"
#include "SlateStyle.h"
#include "IPluginManager.h"

#define LOCTEXT_NAMESPACE "FPDFImporterModuleEd"

void FPDFImporterEdModule::StartupModule()
{
	// PDF�A�Z�b�g��AssetTypeActions��AssetTools���W���[���ɓo�^
	PDF_AssetTypeActions = MakeShareable(new FAssetTypeActions_PDF);
	FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get().RegisterAssetTypeActions(PDF_AssetTypeActions.ToSharedRef());

	// PDF�A�Z�b�g�̃A�C�R����o�^
	StyleSet = MakeShareable(new FSlateStyleSet(TEXT("PdfStyle")));
	StyleSet->SetContentRoot(IPluginManager::Get().FindPlugin(TEXT("PDFImporter"))->GetBaseDir());
	
	FSlateImageBrush* ThumbnailBrush = new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Resources/Icon128"), TEXT(".png")), FVector2D(128.f, 128.f));
	if (ThumbnailBrush != nullptr)
	{
		StyleSet->Set(TEXT("ClassThumbnail.PDF"), ThumbnailBrush);
		FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
	}
}

void FPDFImporterEdModule::ShutdownModule()
{
	if (PDF_AssetTypeActions.IsValid())
	{
		if (FModuleManager::Get().IsModuleLoaded(TEXT("AssetTools")))
		{
			FModuleManager::GetModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get().UnregisterAssetTypeActions(PDF_AssetTypeActions.ToSharedRef());
		}
		PDF_AssetTypeActions.Reset();
	}

	FSlateStyleRegistry::UnRegisterSlateStyle(StyleSet->GetStyleSetName());
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPDFImporterEdModule, PDFImporterEd)