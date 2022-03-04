#pragma once
#include "string"
#include "vector"

struct Location
{
	float x;
	float y;
	float z;

};

struct Rotator
{
	/** Rotation around the right axis (around Y axis), Looking up and down (0=Straight Ahead, +Up, -Down) */
	float Pitch;

	/** Rotation around the up axis (around Z axis), Running in circles 0=East, +North, -South. */
	float Yaw;

	/** Rotation around the forward axis (around X axis), Tilting your head, 0=Straight, +Clockwise, -CCW. */
	float Roll;

	float w;
};

struct Scale
{
	float x;
	float y;
	float z;
};

struct Transform
{
	Rotator rotation;
	Location translation;
	Scale scale3D;
};

struct Quat
{
	float X;
	float Y;
	float Z;
	float W;
};

class ActorsInfo
{
public:
	std::vector<std::string> ActorNameArray;

	std::vector<std::string> MeshNameArray;

	std::vector<Transform> ActorsTransformArray;

	std::vector<Quat> ActorsQuatArray;

public:
	void SetSceneActorsInfoFromBat(const std::string& filepath);

	int Size() {
		return ActorNameArray.size();
	}
};

