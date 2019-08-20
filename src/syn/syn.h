#pragma once
#include "syn/common.h"

// exception types
#include "exceptions.hpp"

/******************************************************************************
** Runtime Library
******************************************************************************/

/* Instance Headers */
#include "runtime/instance.h"

/******************************************************************************
** System
******************************************************************************/

// prelude ////////////////////////////////////////////////////////////////////

/* system prelude:
    * Forward declares
    * Exceptions
*/
#include "system/prelude.h" 

/* cpp-support prelude:
    * Forward declares
    * Instance prelude
    * Type Template Lookups declarations
*/
#include "cpp/prelude_system.h" 


// stores /////////////////////////////////////////////////////////////////////

#include "system/Symbol.hpp"
#include "system/SymbolTable.h"

#include "system/Graph.hpp" /* Graph (and dependents) */

#include "system/TypeStore.h" /* TypeStore */
#include "system/TypeId.h" /* TypeId */


// graph //////////////////////////////////////////////////////////////////////

#include "core/system_graph.h"

//#include "system/expression/expression.h"
//#include "system/expression/queries.h"

#include "system/epilogue.h"


/******************************************************************************
** C++ Support/Adaptor
******************************************************************************/

#include "cpp/prelude.h" 

#include "cpp/instance/prelude.hpp"
#include "cpp/instance/policies.hpp"
#include "cpp/instance/containers.hpp"

#include "cpp/cpp_interface.h"

/*#include "system/GenericInvoke.hpp"

#include "system/SimpleDispatcher.hpp"

#include "boot/system_into_cpp.h"

//#include "cpp/graph.hpp"
#include "cpp/cpp_types.h"
#include "cpp/expressions.hpp"
#include "system/ExpressionDispatcher.hpp"
*/

/******************************************************************************
** Boot Runtime Library
******************************************************************************/

/*#include "cpp/DefineHelper.hpp"

#include "boot/default_types_c.h"
#include "boot/default_types_cpp.h"
*/

// TODO:
// * Rework objects to manage their own instance meta stuff (rather than from_raw) via SLifecycle
// * Hookup ->byConfiguring
// * Finish ObjectComposite and ObjectManipulation capabilities.
// * Rename ObjectDefineDetails to take into account types
// * Add default c_types
// * Add default cpp_types

/******************************************************************************
** Dll Stuff
******************************************************************************/

//#include "dll_helpers.h"