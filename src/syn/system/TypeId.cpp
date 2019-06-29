#include "syn/syn.h"
#include "TypeId.h"

using namespace syn;

/******************************************************************************
** TypeId
******************************************************************************/

/*
CRAFT_TYPE_DEFINE(syn::TypeId)
{
	_.use<PStringer>().singleton<FunctionalStringer>([](instance<TypeId> n) -> std::string
	{
		return n->toString();
	});

	_.identify_byFullCppName("syn::TypeId");
	_.defaults();
}
 */

std::string TypeId::toString() const
{
	if (_node == nullptr)
	{
		return "None";
	}

	std::string from, name;

	auto cpp_name = thread_store().onlyPropOfTypeOnNode<Type_Property_CppIdentifier>(_node);
	if (cpp_name)
	{
		return cpp_name->cpp_identifier;
	}

	auto ns_name = thread_store().onlyPropOfTypeOnNode<Type_Property_NamespaceIdentifier>(_node);
	if (ns_name)
	{
		return ns_name->namespace_identifier;
	}

	auto l_name = thread_store().onlyPropOfTypeOnNode<Type_Property_LocalIdentifier>(_node);
	if (l_name)
	{
		return l_name->local_identifier;
	}

	return fmt::format("Unknown-{0}", (void*)_node);
}
