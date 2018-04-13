#pragma once
#include "../common.h"
#include "../core.h"

namespace craft {
namespace types
{
	//
	// expression
	//

	template<typename T>
	struct VarArgs final
	{
		typedef typename T value_type;
		std::vector<T> args;
	};

	template<typename T,
		typename std::enable_if< stdext::is_specialization<T, instance>::value && !std::is_same<typename T::instance_type, void>::value >::type* = nullptr>
	IExpression* to_expression()
	{
		return new ExpressionConcrete(graph().recoverNode(cpptype<typename T::instance_type>::typeDesc().asNode()));
	}

	template<typename T,
		typename std::enable_if< stdext::is_specialization<T, instance>::value && std::is_same<typename T::instance_type, void>::value >::type* = nullptr>
		IExpression* to_expression()
	{
		return ExpressionSpecial::Any;
	}

	template<typename T,
		typename std::enable_if< stdext::is_specialization<T, VarArgs>::value >::type* = nullptr>
		IExpression* to_expression()
	{
		return to_expression<typename T::value_type>();
	}

	template<typename T,
		typename std::enable_if< std::is_same<T, void>::value >::type* = nullptr>
	IExpression* to_expression()
	{
		return ExpressionSpecial::Void;
	}

	template<typename ...TArgs,
		typename std::enable_if< stdext::is_specialization<typename stdext::parampack_last<TArgs...>::type, VarArgs>::value >::type* = nullptr>
	IExpression* to_expression_tuple()
	{
		std::vector<IExpression*> vec = { to_expression<TArgs>()... };
		IExpression* collect = vec.back(); vec.pop_back();

		return new ExpressionTuple(vec, collect);
	}

	template<typename ...TArgs,
		typename std::enable_if< !stdext::is_specialization<typename stdext::parampack_last<TArgs...>::type, VarArgs>::value >::type* = nullptr>
		IExpression* to_expression_tuple()
	{
		return new ExpressionTuple({ to_expression<TArgs>()... });
	}

	template<typename ...TArgs,
		typename std::enable_if< sizeof...(TArgs) == 0 >::type* = nullptr>
		IExpression* to_expression_tuple()
	{
		return new ExpressionTuple({ to_expression<TArgs>()... });
	}

	template<typename TRet, typename ...TArgs>
	std::tuple<ExpressionStore, Function> to_expression_and_function(TRet (*fn)(TArgs...))
	{
		auto arrow = new ExpressionArrow(to_expression_tuple<TArgs...>(), to_expression<TRet>());

		return std::make_tuple(ExpressionStore(arrow), Function{ fn });
	}

	template<typename TClass, typename TRet, typename ...TArgs>
	std::tuple<ExpressionStore, Function> to_expression_and_function(TRet (TClass::*fn)(TArgs...))
	{
		
	}

	//
	// invoke
	//

	inline instance<> invoke(ExpressionStore const& exs, Function const& f, GenericInvoke const& i)
	{
		assert(exs.root()->kind().ptr() == cpptype<ExpressionArrow>::typeDesc().asNode());
		auto arrow = (ExpressionArrow const*)exs.root();

		assert(arrow->input->kind().ptr() == cpptype<ExpressionArrow>::typeDesc().asNode());
		auto tuple = (ExpressionTuple const*)arrow->input;

		auto t_size = tuple->entries.size();

		// varargs
		if (tuple->varType != nullptr)
		{
			assert(t_size <= i.args.size());

			// TODO: improve
			typedef VarArgs<instance<>> VarArgs;
			VarArgs va;
			va.args.resize(i.args.size() - t_size);
			std::copy(i.args.begin() + t_size, i.args.end(), std::back_inserter(va.args));

			// WITH ret
			if (arrow->output != ExpressionSpecial::Void)
			{
				switch (t_size)
				{
				case 0:
					return reinterpret_cast<instance<>(*)(VarArgs)>(f._fn)(va);
				case 1:
					return reinterpret_cast<instance<>(*)(instance<>, VarArgs)>(f._fn)(i.args[0], va);
				case 2:
					return reinterpret_cast<instance<>(*)(instance<>, instance<>, VarArgs)>(f._fn)(i.args[0], i.args[1], va);
				case 3:
					return reinterpret_cast<instance<>(*)(instance<>, instance<>, instance<>, VarArgs)>(f._fn)(i.args[0], i.args[1], i.args[2], va);
				case 4:
					return reinterpret_cast<instance<>(*)(instance<>, instance<>, instance<>, instance<>, VarArgs)>(f._fn)(i.args[0], i.args[1], i.args[2], i.args[3], va);
				default:
					throw type_runtime_error("invoke not compiled for this.");
				}
			}
			// withOUT ret
			else
			{
				switch (t_size)
				{
				case 0:
					reinterpret_cast<void(*)(VarArgs)>(f._fn)(va); break;
				case 1:
					reinterpret_cast<void(*)(instance<>, VarArgs)>(f._fn)(i.args[0], va); break;
				case 2:
					reinterpret_cast<void(*)(instance<>, instance<>, VarArgs)>(f._fn)(i.args[0], i.args[1], va); break;
				case 3:
					reinterpret_cast<void(*)(instance<>, instance<>, instance<>, VarArgs)>(f._fn)(i.args[0], i.args[1], i.args[2], va); break;
				case 4:
					reinterpret_cast<void(*)(instance<>, instance<>, instance<>, instance<>, VarArgs)>(f._fn)(i.args[0], i.args[1], i.args[2], i.args[3], va); break;
				default:
					throw type_runtime_error("invoke not compiled for this.");
				}

				return instance<>();
			}
		}
		// standard
		else
		{
			assert(t_size == i.args.size());

			// WITH ret
			if (arrow->output != ExpressionSpecial::Void)
			{
				switch (t_size)
				{
				case 0:
					return reinterpret_cast<instance<>(*)()>(f._fn)();
				case 1:
					return reinterpret_cast<instance<>(*)(instance<>)>(f._fn)(i.args[0]);
				case 2:
					return reinterpret_cast<instance<>(*)(instance<>, instance<>)>(f._fn)(i.args[0], i.args[1]);
				case 3:
					return reinterpret_cast<instance<>(*)(instance<>, instance<>, instance<>)>(f._fn)(i.args[0], i.args[1], i.args[2]);
				case 4:
					return reinterpret_cast<instance<>(*)(instance<>, instance<>, instance<>, instance<>)>(f._fn)(i.args[0], i.args[1], i.args[2], i.args[3]);
				default:
					throw type_runtime_error("invoke not compiled for this.");
				}
			}
			// withOUT ret
			else
			{
				switch (t_size)
				{
				case 0:
					reinterpret_cast<void(*)()>(f._fn)(); break;
				case 1:
					reinterpret_cast<void(*)(instance<>)>(f._fn)(i.args[0]); break;
				case 2:
					reinterpret_cast<void(*)(instance<>, instance<>)>(f._fn)(i.args[0], i.args[1]); break;
				case 3:
					reinterpret_cast<void(*)(instance<>, instance<>, instance<>)>(f._fn)(i.args[0], i.args[1], i.args[2]); break;
				case 4:
					reinterpret_cast<void(*)(instance<>, instance<>, instance<>, instance<>)>(f._fn)(i.args[0], i.args[1], i.args[2], i.args[3]); break;
				default:
					throw type_runtime_error("invoke not compiled for this.");
				}

				return instance<>();
			}
		}

		throw stdext::exception("Malformed Invoke");
	}
}}