#include "../common.h"
#include "../core.h"

using namespace craft::types;

// Core Providers
CRAFT_PROVIDER_DEFINE(PIdentifier);
CRAFT_PROVIDER_DEFINE(PConstructor);
CRAFT_PROVIDER_DEFINE(PStringer);
CRAFT_PROVIDER_DEFINE(PConversion);
CRAFT_PROVIDER_DEFINE(PParse);
CRAFT_PROVIDER_DEFINE(PRepr);
CRAFT_PROVIDER_DEFINE(PClone);

// Object Providers
CRAFT_PROVIDER_DEFINE(PObjectContextual);

// Core Aspects
CRAFT_ASPECT_DEFINE(SLifecycle);
CRAFT_ASPECT_DEFINE(SContainer);

// Object Aspects
CRAFT_ASPECT_DEFINE(SObjectComposite);
CRAFT_ASPECT_DEFINE(SObjectManipulation);
CRAFT_ASPECT_DEFINE(SObjectFungible);
