// Fill out your copyright notice in the Description page of Project Settings.


#include "RWTxt.h"

bool URWTxt::ReadTxt(FString FilePath, FString& Text)
{
	return FFileHelper::LoadFileToString(Text, *(FilePath));
}

bool URWTxt::WriteTxt(FString FilePath, FString Text)
{
	return FFileHelper::SaveStringToFile(Text, *(FilePath));
}