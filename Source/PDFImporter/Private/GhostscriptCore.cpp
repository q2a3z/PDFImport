#include "GhostscriptCore.h"
#include "PDF.h"
#include "Engine/Texture2D.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "HAL/FileManager.h"
#include "AssetRegistryModule.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "IPluginManager.h"

#include "AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include "HideWindowsPlatformTypes.h"

const FString FGhostscriptCore::PagesDirectoryPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("PDFImporter"))->GetBaseDir(), TEXT("Content")));

FGhostscriptCore::FGhostscriptCore()
{
	// dll�t�@�C���̃p�X���擾
	FString GhostscriptDllPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("PDFImporter"), TEXT("ThirdParty")));
#ifdef _WIN64
	GhostscriptDllPath = FPaths::Combine(GhostscriptDllPath, TEXT("Win64"));
#elif _WIN32
	GhostscriptDllPath = FPaths::Combine(GhostscriptDllPath, TEXT("Win32"));
#endif
	GhostscriptDllPath = FPaths::Combine(GhostscriptDllPath, TEXT("gsdll.dll"));

	// ���W���[�������[�h
	GhostscriptModule = FPlatformProcess::GetDllHandle(*GhostscriptDllPath);
	if (GhostscriptModule == nullptr)
	{
		UE_LOG(PDFImporter, Fatal, TEXT("Failed to load Ghostscript module"));
	}

	// �֐��|�C���^���擾
	CreateInstance = (CreateAPIInstance)FPlatformProcess::GetDllExport(GhostscriptModule, TEXT("gsapi_new_instance"));
	DeleteInstance = (DeleteAPIInstance)FPlatformProcess::GetDllExport(GhostscriptModule, TEXT("gsapi_delete_instance"));
	Init = (InitAPI)FPlatformProcess::GetDllExport(GhostscriptModule, TEXT("gsapi_init_with_args"));
	Exit = (ExitAPI)FPlatformProcess::GetDllExport(GhostscriptModule, TEXT("gsapi_exit"));
	if (CreateInstance == nullptr || DeleteInstance == nullptr || Init == nullptr || Exit == nullptr)
	{
		UE_LOG(PDFImporter, Fatal, TEXT("Failed to get Ghostscript function pointer"));
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);

	UE_LOG(PDFImporter, Log, TEXT("Ghostscrip dll loaded"));
}

FGhostscriptCore::~FGhostscriptCore()
{
	FPlatformProcess::FreeDllHandle(GhostscriptModule);
	UE_LOG(PDFImporter, Log, TEXT("Ghostscript dll unloaded"));
}

UPDF* FGhostscriptCore::ConvertPdfToPdfAsset(const FString& InputPath, int Dpi, int FirstPage, int LastPage, bool bIsImportIntoEditor)
{
	IFileManager& FileManager = IFileManager::Get();
	
	// PDF�����邩�m�F
	if (!FileManager.FileExists(*InputPath))
	{
		UE_LOG(PDFImporter, Error, TEXT("File not found : %s"), *InputPath);
		return nullptr;
	}

	// ��Ɨp�̃f�B���N�g�����쐬
	FString TempDirPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("ConvertTemp"));
	TempDirPath = FPaths::ConvertRelativePathToFull(TempDirPath);
	if (FileManager.DirectoryExists(*TempDirPath))
	{
		FileManager.DeleteDirectory(*TempDirPath);
	}
	FileManager.MakeDirectory(*TempDirPath);
	UE_LOG(PDFImporter, Log, TEXT("A working directory has been created (%s)"), *TempDirPath);

	// Ghostscript��p����PDF����jpg�摜���쐬
	FString OutputPath = FPaths::Combine(TempDirPath, FPaths::GetBaseFilename(InputPath) + TEXT("%010d.jpg"));
	TArray<UTexture2D*> Buffer; 
	UPDF* PDFAsset = nullptr;

	if (ConvertPdfToJpeg(InputPath, OutputPath, Dpi, FirstPage, LastPage))
	{
		// �摜�̃t�@�C���p�X���擾
		TArray<FString> PageNames;
		IFileManager::Get().FindFiles(PageNames, *TempDirPath, L"jpg");
		
		// �쐬����jpg�摜��ǂݍ���
		UTexture2D* TextureTemp;
		for (const FString& PageName : PageNames)
		{
			bool bResult = false;
			if (bIsImportIntoEditor)
			{
#if WITH_EDITORONLY_DATA
				bResult = CreateTextureAssetFromFile(FPaths::Combine(TempDirPath, PageName), TextureTemp);
#endif
			}
			else
			{
				bResult = LoadTexture2DFromFile(FPaths::Combine(TempDirPath, PageName), TextureTemp);
			}
			
			if (bResult)
			{
				Buffer.Add(TextureTemp);
			}
		}

		// PDF�A�Z�b�g���쐬
		PDFAsset = NewObject<UPDF>();

		if (FirstPage <= 0 || LastPage <= 0 || FirstPage > LastPage)
		{
			FirstPage = 1;
			LastPage = Buffer.Num();
		}

		PDFAsset->PageRange = FPageRange(FirstPage, LastPage);
		PDFAsset->Dpi = Dpi;
		PDFAsset->Pages = Buffer;
	}

	// ��ƃf�B���N�g�����폜
	if (FileManager.DirectoryExists(*TempDirPath))
	{
		FileManager.DeleteDirectory(*TempDirPath, true, true);
		UE_LOG(PDFImporter, Log, TEXT("Successfully deleted working directory (%s)"), *TempDirPath);
	}

	return PDFAsset;
}

bool FGhostscriptCore::ConvertPdfToJpeg(const FString& InputPath, const FString& OutputPath, int Dpi, int FirstPage, int LastPage)
{
	if (!(FirstPage > 0 && LastPage > 0 && FirstPage <= LastPage))
	{
		FirstPage = 1;
		LastPage = INT_MAX;
	}

	TArray<char> FirstPageBuffer = FStringToCharPtr(FString(TEXT("-dFirstPage=") + FString::FromInt(FirstPage)));
	TArray<char> LastPageBuffer = FStringToCharPtr(FString(TEXT("-dLastPage=") + FString::FromInt(LastPage)));
	TArray<char> DpiXBuffer = FStringToCharPtr(FString(TEXT("-dDEVICEXRESOLUTION=") + FString::FromInt(Dpi)));
	TArray<char> DpiYBuffer = FStringToCharPtr(FString(TEXT("-dDEVICEYRESOLUTION=") + FString::FromInt(Dpi)));
	TArray<char> OutputPathBuffer = FStringToCharPtr(FString(TEXT("-sOutputFile=") + OutputPath));
	TArray<char> InputPathBuffer = FStringToCharPtr(InputPath);

	const char* Args[20] =
	{
		// Ghostscript���W���o�͂ɏ����o�͂��Ȃ��悤��
		"-q",
		"-dQUIET",

		"-dPARANOIDSAFER",			// �Z�[�t���[�h�Ŏ��s
		"-dBATCH",					// Ghostscript���C���^���N�e�B�u���[�h�ɂȂ�Ȃ��悤��
		"-dNOPAUSE",				// �y�[�W���Ƃ̈ꎞ��~�����Ȃ��悤��
		"-dNOPROMPT",				// �R�}���h�v�����v�g���łȂ��悤��           
		"-dMaxBitmap=500000000",	// �p�t�H�[�}���X�����コ����
		"-dNumRenderingThreads=4",	// �}���`�R�A�Ŏ��s

		// �o�͉摜�̃A���`�G�C���A�X��𑜓x�Ȃ�
		"-dAlignToPixels=0",
		"-dGridFitTT=0",
		"-dTextAlphaBits=4",
		"-dGraphicsAlphaBits=4",

		"-sDEVICE=jpeg",	// jpeg�`���ŏo��
		"-sPAPERSIZE=a7",	// ���̃T�C�Y

		FirstPageBuffer.GetData(),	// 14 : �n�߂̃y�[�W���w��
		LastPageBuffer.GetData(),	// 15 : �I���̃y�[�W���w��
		DpiXBuffer.GetData(),		// 16 : ����DPI
		DpiYBuffer.GetData(),		// 17 : �c��DPI
		OutputPathBuffer.GetData(), // 18 : �o�̓p�X
		InputPathBuffer.GetData()	// 19 : ���̓p�X
	};

	// Ghostscript�̃C���X�^���X���쐬
	void* GhostscriptInstance = nullptr;
	CreateInstance(&GhostscriptInstance, 0);
	if (GhostscriptInstance != nullptr)
	{
		// Ghostscript�����s
		int Result = Init(GhostscriptInstance, 20, (char**)Args);

		// Ghostscript���I��
		Exit(GhostscriptInstance);
		DeleteInstance(GhostscriptInstance);

		UE_LOG(PDFImporter, Log, TEXT("Ghostscript Return Code : %d"), Result);

		return Result == 0;
	}
	else
	{
		UE_LOG(PDFImporter, Error, TEXT("Failed to create Ghostscript instance"));
		return false;
	}
}

bool FGhostscriptCore::LoadTexture2DFromFile(const FString& FilePath, class UTexture2D*& LoadedTexture)
{
	// �摜�f�[�^��ǂݍ���
	TArray<uint8> RawFileData;
	if (FFileHelper::LoadFileToArray(RawFileData, *FilePath) &&
		ImageWrapper.IsValid() && 
		ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num())
		)
	{
		// �񈳏k�̉摜�f�[�^���擾
		const TArray<uint8>* UncompressedRawData = nullptr;
		if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedRawData))
		{
			// Texture2D���쐬
			UTexture2D* NewTexture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);
			if (!NewTexture)
			{
				return false;
			}

			// �s�N�Z���f�[�^���e�N�X�`���ɏ�������
			void* TextureData = NewTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, UncompressedRawData->GetData(), UncompressedRawData->Num());
			NewTexture->PlatformData->Mips[0].BulkData.Unlock();
			NewTexture->UpdateResource();

			LoadedTexture = NewTexture;

			return true;
		}
	}
	
	return false;
}

#if WITH_EDITORONLY_DATA
bool FGhostscriptCore::CreateTextureAssetFromFile(const FString& FilePath, class UTexture2D*& LoadedTexture)
{
	// �摜�f�[�^��ǂݍ���
	TArray<uint8> RawFileData;
	if (FFileHelper::LoadFileToArray(RawFileData, *FilePath))
	{
		// �񈳏k�̉摜�f�[�^���擾
		const TArray<uint8>* UncompressedRawData = nullptr;
		if (ImageWrapper.IsValid() &&
			ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()) &&
			ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedRawData)
			)
		{
			FString Filename = FPaths::GetBaseFilename(FilePath);
			Filename = Filename.Left(Filename.Len() - 10);
			int Width = ImageWrapper->GetWidth();
			int Height = ImageWrapper->GetHeight();

			// �p�b�P�[�W���쐬
			FString PackagePath(TEXT("/PDFImporter/") + Filename + TEXT("/"));
			FString AbsolutePackagePath = PagesDirectoryPath + TEXT("/") + Filename + TEXT("/");

			FPackageName::RegisterMountPoint(PackagePath, AbsolutePackagePath);

			PackagePath += Filename;

			UPackage* Package = CreatePackage(nullptr, *PackagePath);
			Package->FullyLoad();

			// �e�N�X�`�����쐬
			FName TextureName = MakeUniqueObjectName(Package, UTexture2D::StaticClass(), FName(*Filename));
			UTexture2D* NewTexture = NewObject<UTexture2D>(Package, TextureName, RF_Public | RF_Standalone);

			// �e�N�X�`���̐ݒ�
			NewTexture->PlatformData = new FTexturePlatformData();
			NewTexture->PlatformData->SizeX = Width;
			NewTexture->PlatformData->SizeY = Height;
			NewTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
			NewTexture->NeverStream = false;

			// �s�N�Z���f�[�^���e�N�X�`���ɏ�������
			FTexture2DMipMap* Mip = new FTexture2DMipMap();
			NewTexture->PlatformData->Mips.Add(Mip);
			Mip->SizeX = Width;
			Mip->SizeY = Height;
			Mip->BulkData.Lock(LOCK_READ_WRITE);
			uint8* TextureData = (uint8*)Mip->BulkData.Realloc(UncompressedRawData->Num());
			FMemory::Memcpy(TextureData, UncompressedRawData->GetData(), UncompressedRawData->Num());
			Mip->BulkData.Unlock();

			// �e�N�X�`�����X�V
			NewTexture->AddToRoot();
			NewTexture->Source.Init(Width, Height, 1, 1, ETextureSourceFormat::TSF_BGRA8, UncompressedRawData->GetData());
			NewTexture->UpdateResource();

			// �p�b�P�[�W��ۑ�
			Package->MarkPackageDirty();
			FAssetRegistryModule::AssetCreated(NewTexture);
			LoadedTexture = NewTexture;

			FString PackageFilename = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
			return UPackage::SavePackage(Package, NewTexture, RF_Public | RF_Standalone, *PackageFilename, GError, nullptr, true, true, SAVE_NoError);
		}
	}

	return false;
}
#endif

TArray<char> FGhostscriptCore::FStringToCharPtr(const FString& Text)
{
	int Size = GetFStringSize(Text) + 1;
	TArray<char> Buffer("", Size);
	WideCharToMultiByte(CP_ACP, 0, *Text, Size, Buffer.GetData(), Buffer.Num(), NULL, NULL);
	return Buffer;
}

int FGhostscriptCore::GetFStringSize(const FString& InString)
{
	int Size = 0;

	for (TCHAR Char : InString)
	{
		const char* Temp = TCHAR_TO_UTF8(*FString::Chr(Char));
		uint8 Code = static_cast<uint8>(*Temp);

		if ((Code >= 0x00) && (Code <= 0x7f))
		{
			Size += 1;
		}
		else if ((Code >= 0xc2) && (Code <= 0xdf))
		{
			Size += 2;
		}
		else if ((Code >= 0xe0) && (Code <= 0xef))
		{
			Size += 3;
		}
		else if ((Code >= 0xf0) && (Code <= 0xf7))
		{
			Size += 4;
		}
		else if ((Code >= 0xf8) && (Code <= 0xfb))
		{
			Size += 5;
		}
		else if ((Code >= 0xfc) && (Code <= 0xfd))
		{
			Size += 6;
		}
	}

	return Size;
}
