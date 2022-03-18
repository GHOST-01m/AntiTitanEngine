#pragma once

class Device {

};


class RHIResource
{
public:
	virtual std::shared_ptr<Device> GetDevice() = 0;
};

