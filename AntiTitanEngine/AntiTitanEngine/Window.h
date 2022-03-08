#pragma once
class Window
{
	virtual bool InitWindow();

public:
	std::wstring mMainWndCaption = L"AntiTitanEngine";

	int mClientWidth = 1920;
	int mClientHeight = 1080;
};

