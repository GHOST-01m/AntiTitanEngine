#pragma once
class Primitive_Heap
{
public:
	virtual ~Primitive_Heap();

public:
	virtual int GetCurrentHeapSize()=0;

public:
	int currentHeapSize;
};

