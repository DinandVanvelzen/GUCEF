#include "../../Include/RmlUi/Core/RmlUiTraits.h"

namespace Rml {

int FamilyBase::GetNewId()
{
	static int id = 0;
	return id++;
}

} // namespace Rml
