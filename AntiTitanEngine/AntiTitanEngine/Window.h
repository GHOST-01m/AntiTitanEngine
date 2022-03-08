#pragma once
class Window
{
public:
	virtual bool InitWindow() { return false; };

public:
	std::wstring mMainWndCaption = L"AntiTitanEngine";

	int mClientWidth = 1920;
	int mClientHeight = 1080;
};

