//
// Created by bohdan on 07.02.26.
//

#ifndef CVOLT_DEBUGOUTPUT_H
#define CVOLT_DEBUGOUTPUT_H

#include "Volt/Core/CompilationContext/CompilationContext.h"

namespace Volt
{
	class DebugOutput
	{
	private:
		llvm::raw_ostream&Os;
		CompilationContext& CContext;

	public:
		DebugOutput(llvm::raw_ostream& Os, CompilationContext& CContext)
			: Os(Os), CContext(CContext) {}

		void WriteTokens() const;
		void WriteAST() const { WriteAST(CContext.ASTTree, 0); }
		void WriteIR() const { CContext.Module->print(Os, nullptr); }

	private:
		void WriteAST(ASTNode* Node, size_t Indent) const;
		void WriteIndent(size_t Indent) const;
		void WriteCompileTimeValue(SemaResult* Value) const;
	};
}

#endif //CVOLT_DEBUGOUTPUT_H