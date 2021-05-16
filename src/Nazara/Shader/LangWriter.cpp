// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Nazara Engine - Shader generator"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Shader/LangWriter.hpp>
#include <Nazara/Core/Algorithm.hpp>
#include <Nazara/Core/CallOnExit.hpp>
#include <Nazara/Math/Algorithm.hpp>
#include <Nazara/Shader/ShaderBuilder.hpp>
#include <Nazara/Shader/Ast/AstCloner.hpp>
#include <Nazara/Shader/Ast/AstRecursiveVisitor.hpp>
#include <Nazara/Shader/Ast/AstUtils.hpp>
#include <Nazara/Shader/Ast/SanitizeVisitor.hpp>
#include <optional>
#include <stdexcept>
#include <Nazara/Shader/Debug.hpp>

namespace Nz
{
	namespace
	{
		template<typename T> const T& Retrieve(const std::unordered_map<std::size_t, T>& map, std::size_t id)
		{
			auto it = map.find(id);
			assert(it != map.end());
			return it->second;
		}
	}

	struct LangWriter::BindingAttribute
	{
		std::optional<unsigned int> bindingIndex;

		inline bool HasValue() const { return bindingIndex.has_value(); }
	};

	struct LangWriter::BuiltinAttribute
	{
		std::optional<ShaderAst::BuiltinEntry> builtin;

		inline bool HasValue() const { return builtin.has_value(); }
	};

	struct LangWriter::EntryAttribute
	{
		std::optional<ShaderStageType> stageType;

		inline bool HasValue() const { return stageType.has_value(); }
	};

	struct LangWriter::LayoutAttribute
	{
		std::optional<StructLayout> layout;

		inline bool HasValue() const { return layout.has_value(); }
	};

	struct LangWriter::LocationAttribute
	{
		std::optional<unsigned int> locationIndex;

		inline bool HasValue() const { return locationIndex.has_value(); }
	};

	struct LangWriter::State
	{
		const States* states = nullptr;
		std::stringstream stream;
		std::unordered_map<std::size_t, std::string> optionNames;
		std::unordered_map<std::size_t, ShaderAst::StructDescription> structs;
		std::unordered_map<std::size_t, std::string> variableNames;
		bool isInEntryPoint = false;
		unsigned int indentLevel = 0;
	};

	std::string LangWriter::Generate(ShaderAst::StatementPtr& shader, const States& states)
	{
		State state;
		m_currentState = &state;
		CallOnExit onExit([this]()
		{
			m_currentState = nullptr;
		});

		ShaderAst::SanitizeVisitor::Options options;
		options.removeOptionDeclaration = false;

		ShaderAst::StatementPtr sanitizedAst = ShaderAst::Sanitize(shader, options);

		AppendHeader();

		sanitizedAst->Visit(*this);

		return state.stream.str();
	}

	void LangWriter::SetEnv(Environment environment)
	{
		m_environment = std::move(environment);
	}

	void LangWriter::Append(const ShaderAst::ExpressionType& type)
	{
		std::visit([&](auto&& arg)
		{
			Append(arg);
		}, type);
	}

	void LangWriter::Append(const ShaderAst::IdentifierType& /*identifierType*/)
	{
		throw std::runtime_error("unexpected identifier type");
	}

	void LangWriter::Append(const ShaderAst::MatrixType& matrixType)
	{
		if (matrixType.columnCount == matrixType.rowCount)
		{
			Append("mat");
			Append(matrixType.columnCount);
		}
		else
		{
			Append("mat");
			Append(matrixType.columnCount);
			Append("x");
			Append(matrixType.rowCount);
		}

		Append("<", matrixType.type, ">");
	}

	void LangWriter::Append(ShaderAst::PrimitiveType type)
	{
		switch (type)
		{
			case ShaderAst::PrimitiveType::Boolean: return Append("bool");
			case ShaderAst::PrimitiveType::Float32: return Append("f32");
			case ShaderAst::PrimitiveType::Int32:   return Append("i32");
			case ShaderAst::PrimitiveType::UInt32:  return Append("ui32");
		}
	}

	void LangWriter::Append(const ShaderAst::SamplerType& samplerType)
	{
		Append("sampler");

		switch (samplerType.dim)
		{
			case ImageType_1D:       Append("1D");      break;
			case ImageType_1D_Array: Append("1DArray"); break;
			case ImageType_2D:       Append("2D");      break;
			case ImageType_2D_Array: Append("2DArray"); break;
			case ImageType_3D:       Append("3D");      break;
			case ImageType_Cubemap:  Append("Cube");    break;
		}

		Append("<", samplerType.sampledType, ">");
	}

	void LangWriter::Append(const ShaderAst::StructType& structType)
	{
		const auto& structDesc = Retrieve(m_currentState->structs, structType.structIndex);
		Append(structDesc.name);
	}

	void LangWriter::Append(const ShaderAst::UniformType& uniformType)
	{
		Append("uniform<");
		std::visit([&](auto&& arg)
		{
			Append(arg);
		}, uniformType.containedType);
		Append(">");
	}

	void LangWriter::Append(const ShaderAst::VectorType& vecType)
	{
		Append("vec", vecType.componentCount, "<", vecType.type, ">");
	}

	void LangWriter::Append(ShaderAst::NoType)
	{
		return Append("()");
	}

	template<typename T>
	void LangWriter::Append(const T& param)
	{
		NazaraAssert(m_currentState, "This function should only be called while processing an AST");

		m_currentState->stream << param;
	}

	template<typename T1, typename T2, typename... Args>
	void LangWriter::Append(const T1& firstParam, const T2& secondParam, Args&&... params)
	{
		Append(firstParam);
		Append(secondParam, std::forward<Args>(params)...);
	}

	template<typename ...Args>
	void LangWriter::AppendAttributes(bool appendLine, Args&&... params)
	{
		bool hasAnyAttribute = (params.HasValue() || ...);
		if (!hasAnyAttribute)
			return;

		Append("[");
		(AppendAttribute(params), ...);
		Append("]");

		if (appendLine)
			AppendLine();
		else
			Append(" ");
	}

	void LangWriter::AppendAttribute(BindingAttribute builtin)
	{
		if (!builtin.HasValue())
			return;

		Append("binding(", *builtin.bindingIndex, ")");
	}

	void LangWriter::AppendAttribute(BuiltinAttribute builtin)
	{
		if (!builtin.HasValue())
			return;

		switch (*builtin.builtin)
		{
			case ShaderAst::BuiltinEntry::VertexPosition:
				Append("builtin(position)");
				break;
		}
	}

	void LangWriter::AppendAttribute(EntryAttribute entry)
	{
		if (!entry.HasValue())
			return;

		switch (*entry.stageType)
		{
			case ShaderStageType::Fragment:
				Append("entry(frag)");
				break;

			case ShaderStageType::Vertex:
				Append("entry(vert)");
				break;
		}
	}

	void LangWriter::AppendAttribute(LayoutAttribute entry)
	{
		if (!entry.HasValue())
			return;

		switch (*entry.layout)
		{
			case StructLayout_Std140:
				Append("layout(std140)");
				break;
		}
	}

	void LangWriter::AppendAttribute(LocationAttribute location)
	{
		if (!location.HasValue())
			return;

		Append("location(", *location.locationIndex, ")");
	}

	void LangWriter::AppendCommentSection(const std::string& section)
	{
		NazaraAssert(m_currentState, "This function should only be called while processing an AST");

		std::string stars((section.size() < 33) ? (36 - section.size()) / 2 : 3, '*');
		m_currentState->stream << "/*" << stars << ' ' << section << ' ' << stars << "*/";
		AppendLine();
	}

	void LangWriter::AppendField(std::size_t structIndex, const std::size_t* memberIndices, std::size_t remainingMembers)
	{
		const auto& structDesc = Retrieve(m_currentState->structs, structIndex);

		const auto& member = structDesc.members[*memberIndices];

		Append(".");
		Append(member.name);

		if (remainingMembers > 1)
		{
			assert(IsStructType(member.type));
			AppendField(std::get<ShaderAst::StructType>(member.type).structIndex, memberIndices + 1, remainingMembers - 1);
		}
	}

	void LangWriter::AppendLine(const std::string& txt)
	{
		NazaraAssert(m_currentState, "This function should only be called while processing an AST");

		m_currentState->stream << txt << '\n' << std::string(m_currentState->indentLevel, '\t');
	}

	template<typename... Args>
	void LangWriter::AppendLine(Args&&... params)
	{
		(Append(std::forward<Args>(params)), ...);
		AppendLine();
	}

	void LangWriter::AppendStatementList(std::vector<ShaderAst::StatementPtr>& statements)
	{
		bool first = true;
		for (const ShaderAst::StatementPtr& statement : statements)
		{
			if (!first && statement->GetType() != ShaderAst::NodeType::NoOpStatement)
				AppendLine();

			statement->Visit(*this);

			first = false;
		}
	}

	void LangWriter::EnterScope()
	{
		NazaraAssert(m_currentState, "This function should only be called while processing an AST");

		m_currentState->indentLevel++;
		AppendLine("{");
	}

	void LangWriter::LeaveScope(bool skipLine)
	{
		NazaraAssert(m_currentState, "This function should only be called while processing an AST");

		m_currentState->indentLevel--;
		AppendLine();

		if (skipLine)
			AppendLine("}");
		else
			Append("}");
	}

	void LangWriter::RegisterOption(std::size_t optionIndex, std::string optionName)
	{
		assert(m_currentState->optionNames.find(optionIndex) == m_currentState->optionNames.end());
		m_currentState->optionNames.emplace(optionIndex, std::move(optionName));
	}

	void LangWriter::RegisterStruct(std::size_t structIndex, ShaderAst::StructDescription desc)
	{
		assert(m_currentState->structs.find(structIndex) == m_currentState->structs.end());
		m_currentState->structs.emplace(structIndex, std::move(desc));
	}

	void LangWriter::RegisterVariable(std::size_t varIndex, std::string varName)
	{
		assert(m_currentState->variableNames.find(varIndex) == m_currentState->variableNames.end());
		m_currentState->variableNames.emplace(varIndex, std::move(varName));
	}

	void LangWriter::Visit(ShaderAst::ExpressionPtr& expr, bool encloseIfRequired)
	{
		bool enclose = encloseIfRequired && (GetExpressionCategory(*expr) != ShaderAst::ExpressionCategory::LValue);

		if (enclose)
			Append("(");

		expr->Visit(*this);

		if (enclose)
			Append(")");
	}

	void LangWriter::Visit(ShaderAst::AccessMemberIndexExpression& node)
	{
		Visit(node.structExpr, true);

		const ShaderAst::ExpressionType& exprType = GetExpressionType(*node.structExpr);
		assert(IsStructType(exprType));

		AppendField(std::get<ShaderAst::StructType>(exprType).structIndex, node.memberIndices.data(), node.memberIndices.size());
	}

	void LangWriter::Visit(ShaderAst::AssignExpression& node)
	{
		node.left->Visit(*this);

		switch (node.op)
		{
			case ShaderAst::AssignType::Simple:
				Append(" = ");
				break;
		}

		node.right->Visit(*this);
	}

	void LangWriter::Visit(ShaderAst::BranchStatement& node)
	{
		bool first = true;
		for (const auto& statement : node.condStatements)
		{
			if (!first)
				Append("else ");

			Append("if (");
			statement.condition->Visit(*this);
			AppendLine(")");

			EnterScope();
			statement.statement->Visit(*this);
			LeaveScope();

			first = false;
		}

		if (node.elseStatement)
		{
			AppendLine("else");

			EnterScope();
			node.elseStatement->Visit(*this);
			LeaveScope();
		}
	}

	void LangWriter::Visit(ShaderAst::BinaryExpression& node)
	{
		Visit(node.left, true);

		switch (node.op)
		{
			case ShaderAst::BinaryType::Add:       Append(" + "); break;
			case ShaderAst::BinaryType::Subtract:  Append(" - "); break;
			case ShaderAst::BinaryType::Multiply:  Append(" * "); break;
			case ShaderAst::BinaryType::Divide:    Append(" / "); break;

			case ShaderAst::BinaryType::CompEq:    Append(" == "); break;
			case ShaderAst::BinaryType::CompGe:    Append(" >= "); break;
			case ShaderAst::BinaryType::CompGt:    Append(" > ");  break;
			case ShaderAst::BinaryType::CompLe:    Append(" <= "); break;
			case ShaderAst::BinaryType::CompLt:    Append(" < ");  break;
			case ShaderAst::BinaryType::CompNe:    Append(" != "); break;
		}

		Visit(node.right, true);
	}

	void LangWriter::Visit(ShaderAst::CastExpression& node)
	{
		Append(node.targetType);
		Append("(");

		bool first = true;
		for (const auto& exprPtr : node.expressions)
		{
			if (!exprPtr)
				break;

			if (!first)
				m_currentState->stream << ", ";

			exprPtr->Visit(*this);
			first = false;
		}

		Append(")");
	}

	void LangWriter::Visit(ShaderAst::ConditionalExpression& node)
	{
		Append("select_opt(", Retrieve(m_currentState->optionNames, node.optionIndex), ", ");
		node.truePath->Visit(*this);
		Append(", ");
		node.falsePath->Visit(*this);
		Append(")");
	}

	void LangWriter::Visit(ShaderAst::ConditionalStatement& node)
	{
		Append("[opt(", Retrieve(m_currentState->optionNames, node.optionIndex), ")]");
		node.statement->Visit(*this);
	}

	void LangWriter::Visit(ShaderAst::ConstantExpression& node)
	{
		std::visit([&](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, bool>)
				Append((arg) ? "true" : "false");
			else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, Int32> || std::is_same_v<T, UInt32>)
				Append(std::to_string(arg));
			else if constexpr (std::is_same_v<T, Vector2f>)
				Append("vec2<f32>(" + std::to_string(arg.x) + ", " + std::to_string(arg.y) + ")");
			else if constexpr (std::is_same_v<T, Vector2i32>)
				Append("vec2<i32>(" + std::to_string(arg.x) + ", " + std::to_string(arg.y) + ")");
			else if constexpr (std::is_same_v<T, Vector3f>)
				Append("vec3<f32>(" + std::to_string(arg.x) + ", " + std::to_string(arg.y) + ", " + std::to_string(arg.z) + ")");
			else if constexpr (std::is_same_v<T, Vector3i32>)
				Append("vec3<i32>(" + std::to_string(arg.x) + ", " + std::to_string(arg.y) + ", " + std::to_string(arg.z) + ")");
			else if constexpr (std::is_same_v<T, Vector4f>)
				Append("vec4<f32>(" + std::to_string(arg.x) + ", " + std::to_string(arg.y) + ", " + std::to_string(arg.z) + ", " + std::to_string(arg.w) + ")");
			else if constexpr (std::is_same_v<T, Vector4i32>)
				Append("vec4<i32>(" + std::to_string(arg.x) + ", " + std::to_string(arg.y) + ", " + std::to_string(arg.z) + ", " + std::to_string(arg.w) + ")");
			else
				static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");
		}, node.value);
	}

	void LangWriter::Visit(ShaderAst::DeclareExternalStatement& node)
	{
		assert(node.varIndex);
		std::size_t varIndex = *node.varIndex;

		AppendLine("external");
		EnterScope();

		bool first = true;
		for (const auto& externalVar : node.externalVars)
		{
			if (!first)
				AppendLine(",");

			first = false;

			AppendAttributes(false, BindingAttribute{ externalVar.bindingIndex });
			Append(externalVar.name, ": ", externalVar.type);

			RegisterVariable(varIndex++, externalVar.name);
		}

		LeaveScope();
	}

	void LangWriter::Visit(ShaderAst::DeclareFunctionStatement& node)
	{
		NazaraAssert(m_currentState, "This function should only be called while processing an AST");

		std::optional<std::size_t> varIndexOpt = node.varIndex;

		AppendAttributes(true, EntryAttribute{ node.entryStage });
		Append("fn ", node.name, "(");
		for (std::size_t i = 0; i < node.parameters.size(); ++i)
		{
			if (i != 0)
				Append(", ");

			Append(node.parameters[i].name);
			Append(": ");
			Append(node.parameters[i].type);

			assert(varIndexOpt);
			std::size_t& varIndex = *varIndexOpt;
			RegisterVariable(varIndex++, node.parameters[i].name);
		}
		Append(")");
		if (!IsNoType(node.returnType))
			Append(" -> ", node.returnType);

		AppendLine();
		EnterScope();
		{
			AppendStatementList(node.statements);
		}
		LeaveScope();
	}

	void LangWriter::Visit(ShaderAst::DeclareOptionStatement& node)
	{
		assert(node.optIndex);
		RegisterOption(*node.optIndex, node.optName);

		Append("option ", node.optName, ": ", node.optType);
		if (node.initialValue)
		{
			Append(" = ");
			node.initialValue->Visit(*this);
		}

		Append(";");
	}

	void LangWriter::Visit(ShaderAst::DeclareStructStatement& node)
	{
		assert(node.structIndex);
		RegisterStruct(*node.structIndex, node.description);

		AppendAttributes(true, LayoutAttribute{ node.description.layout });
		Append("struct ");
		AppendLine(node.description.name);
		EnterScope();
		{
			bool first = true;
			for (const auto& member : node.description.members)
			{
				if (!first)
					AppendLine(",");

				first = false;

				AppendAttributes(false, LocationAttribute{ member.locationIndex }, BuiltinAttribute{ member.builtin });
				Append(member.name, ": ", member.type);
			}
		}
		LeaveScope();
	}

	void LangWriter::Visit(ShaderAst::DeclareVariableStatement& node)
	{
		assert(node.varIndex);
		RegisterVariable(*node.varIndex, node.varName);

		Append("let ", node.varName, ": ", node.varType);
		if (node.initialExpression)
		{
			Append(" = ");
			node.initialExpression->Visit(*this);
		}

		Append(";");
	}

	void LangWriter::Visit(ShaderAst::DiscardStatement& /*node*/)
	{
		Append("discard;");
	}

	void LangWriter::Visit(ShaderAst::ExpressionStatement& node)
	{
		node.expression->Visit(*this);
		Append(";");
	}

	void LangWriter::Visit(ShaderAst::IntrinsicExpression& node)
	{
		switch (node.intrinsic)
		{
			case ShaderAst::IntrinsicType::CrossProduct:
				Append("cross");
				break;

			case ShaderAst::IntrinsicType::DotProduct:
				Append("dot");
				break;

			case ShaderAst::IntrinsicType::Length:
				Append("length");
				break;

			case ShaderAst::IntrinsicType::SampleTexture:
				Append("texture");
				break;
		}

		Append("(");
		for (std::size_t i = 0; i < node.parameters.size(); ++i)
		{
			if (i != 0)
				Append(", ");

			node.parameters[i]->Visit(*this);
		}
		Append(")");
	}

	void LangWriter::Visit(ShaderAst::MultiStatement& node)
	{
		AppendStatementList(node.statements);
	}

	void LangWriter::Visit(ShaderAst::NoOpStatement& /*node*/)
	{
		/* nothing to do */
	}

	void LangWriter::Visit(ShaderAst::ReturnStatement& node)
	{
		if (node.returnExpr)
		{
			Append("return ");
			node.returnExpr->Visit(*this);
			Append(";");
		}
		else
			Append("return;");
	}

	void LangWriter::Visit(ShaderAst::SwizzleExpression& node)
	{
		Visit(node.expression, true);
		Append(".");

		for (std::size_t i = 0; i < node.componentCount; ++i)
		{
			switch (node.components[i])
			{
				case ShaderAst::SwizzleComponent::First:
					Append("x");
					break;

				case ShaderAst::SwizzleComponent::Second:
					Append("y");
					break;

				case ShaderAst::SwizzleComponent::Third:
					Append("z");
					break;

				case ShaderAst::SwizzleComponent::Fourth:
					Append("w");
					break;
			}
		}
	}

	void LangWriter::Visit(ShaderAst::VariableExpression& node)
	{
		const std::string& varName = Retrieve(m_currentState->variableNames, node.variableId);
		Append(varName);
	}

	void LangWriter::Visit(ShaderAst::UnaryExpression& node)
	{
		switch (node.op)
		{
			case ShaderAst::UnaryType::LogicalNot:
				Append("!");
				break;

			case ShaderAst::UnaryType::Minus:
				Append("-");
				break;

			case ShaderAst::UnaryType::Plus:
				Append("+");
				break;
		}

		Append(node.expression);
	}

	void LangWriter::AppendHeader()
	{
		// Nothing yet
	}
}
