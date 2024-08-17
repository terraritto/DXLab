#pragma once

class ASInterface
{
protected:
	virtual void PreBuildAS() = 0;
	virtual void MakeResourceAndConstructAS() = 0;
	virtual void MakeCommand() = 0;
};
