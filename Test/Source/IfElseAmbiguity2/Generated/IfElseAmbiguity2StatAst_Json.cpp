/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:StatAst
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "IfElseAmbiguity2StatAst_Json.h"

namespace ifelseambiguity2
{
	namespace json_visitor
	{
		void StatAstVisitor::PrintFields(BlockStat* node)
		{
			BeginField(L"stats");
			BeginArray();
			for (auto&& listItem : node->stats)
			{
				BeginArrayItem();
				Print(listItem.Obj());
				EndArrayItem();
			}
			EndArray();
			EndField();
		}
		void StatAstVisitor::PrintFields(DoStat* node)
		{
		}
		void StatAstVisitor::PrintFields(IfContent* node)
		{
		}
		void StatAstVisitor::PrintFields(IfContentCandidate* node)
		{
			BeginField(L"elseBranch");
			Print(node->elseBranch.Obj());
			EndField();
			BeginField(L"thenBranch");
			Print(node->thenBranch.Obj());
			EndField();
		}
		void StatAstVisitor::PrintFields(IfContentToResolve* node)
		{
			BeginField(L"candidates");
			BeginArray();
			for (auto&& listItem : node->candidates)
			{
				BeginArrayItem();
				Print(listItem.Obj());
				EndArrayItem();
			}
			EndArray();
			EndField();
		}
		void StatAstVisitor::PrintFields(IfStat* node)
		{
			BeginField(L"content");
			Print(node->content.Obj());
			EndField();
		}
		void StatAstVisitor::PrintFields(Module* node)
		{
			BeginField(L"stat");
			Print(node->stat.Obj());
			EndField();
		}
		void StatAstVisitor::PrintFields(Stat* node)
		{
		}

		void StatAstVisitor::Visit(DoStat* node)
		{
			if (!node)
			{
				WriteNull();
				return;
			}
			BeginObject();
			WriteType(L"DoStat", node);
			PrintFields(static_cast<Stat*>(node));
			PrintFields(static_cast<DoStat*>(node));
			EndObject();
		}

		void StatAstVisitor::Visit(IfStat* node)
		{
			if (!node)
			{
				WriteNull();
				return;
			}
			BeginObject();
			WriteType(L"IfStat", node);
			PrintFields(static_cast<Stat*>(node));
			PrintFields(static_cast<IfStat*>(node));
			EndObject();
		}

		void StatAstVisitor::Visit(BlockStat* node)
		{
			if (!node)
			{
				WriteNull();
				return;
			}
			BeginObject();
			WriteType(L"BlockStat", node);
			PrintFields(static_cast<Stat*>(node));
			PrintFields(static_cast<BlockStat*>(node));
			EndObject();
		}

		void StatAstVisitor::Visit(IfContentToResolve* node)
		{
			if (!node)
			{
				WriteNull();
				return;
			}
			BeginObject();
			WriteType(L"IfContentToResolve", node);
			PrintFields(static_cast<IfContent*>(node));
			PrintFields(static_cast<IfContentToResolve*>(node));
			EndObject();
		}

		void StatAstVisitor::Visit(IfContentCandidate* node)
		{
			if (!node)
			{
				WriteNull();
				return;
			}
			BeginObject();
			WriteType(L"IfContentCandidate", node);
			PrintFields(static_cast<IfContent*>(node));
			PrintFields(static_cast<IfContentCandidate*>(node));
			EndObject();
		}

		StatAstVisitor::StatAstVisitor(vl::stream::StreamWriter& _writer)
			: vl::glr::JsonVisitorBase(_writer)
		{
		}

		void StatAstVisitor::Print(Stat* node)
		{
			if (!node)
			{
				WriteNull();
				return;
			}
			node->Accept(static_cast<Stat::IVisitor*>(this));
		}

		void StatAstVisitor::Print(IfContent* node)
		{
			if (!node)
			{
				WriteNull();
				return;
			}
			node->Accept(static_cast<IfContent::IVisitor*>(this));
		}

		void StatAstVisitor::Print(Module* node)
		{
			if (!node)
			{
				WriteNull();
				return;
			}
			BeginObject();
			WriteType(L"Module", node);
			PrintFields(static_cast<Module*>(node));
			EndObject();
		}

	}
}