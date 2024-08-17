#include "ProcedualContainer.h"

ProcedualContainer::ProcedualContainer()
	: m_procedualMesh(std::make_unique<ProcedualMesh>())
{
}
