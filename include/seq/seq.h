#ifndef SEQ_SEQ_H
#define SEQ_SEQ_H

#include <cstdlib>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <initializer_list>

#include "llvm.h"
#include "func.h"
#include "pipeline.h"
#include "stageutil.h"
#include "var.h"
#include "mem.h"
#include "lambda.h"
#include "io.h"
#include "exc.h"
#include "common.h"

namespace seq {

	namespace types {
		static AnyType&   Any   = *AnyType::get();
		static BaseType&  Base  = *BaseType::get();
		static VoidType&  Void  = *VoidType::get();
		static SeqType&   Seq   = *SeqType::get();
		static IntType&   Int   = *IntType::get();
		static FloatType& Float = *FloatType::get();
		static BoolType&  Bool  = *BoolType::get();
		static ArrayType& Array = *ArrayType::get();
	}

	struct PipelineAggregator {
		SeqModule *base;
		std::vector<Pipeline> pipelines;
		explicit PipelineAggregator(SeqModule *base);
		void add(Pipeline pipeline);
		Pipeline addWithIndex(Pipeline to, seq_int_t idx);
		Pipeline operator|(Pipeline to);
		Pipeline operator|(PipelineList to);
		Pipeline operator|(Var& to);
	};

	struct PipelineAggregatorProxy {
		PipelineAggregator& aggr;
		seq_int_t idx;
		PipelineAggregatorProxy(PipelineAggregator& aggr, seq_int_t idx);
		explicit PipelineAggregatorProxy(PipelineAggregator& aggr);
		Pipeline operator|(Pipeline to);
		Pipeline operator|(PipelineList to);
		Pipeline operator|(Var& to);
	};

	class SeqModule : public BaseFunc {
	private:
		std::vector<std::string> sources;
		std::array<ValMap, io::MAX_INPUTS> outs;

		friend PipelineAggregator;
	public:
		SeqModule();

		PipelineAggregator main;
		PipelineAggregator once;
		PipelineAggregator last;

		void source(std::string s);

		template<typename ...T>
		void source(std::string s, T... etc)
		{
			source(std::move(s));
			source(etc...);
		}

		void codegen(llvm::Module *module) override;
		void codegenCall(BaseFunc *base,
		                 ValMap ins,
		                 ValMap outs,
		                 llvm::BasicBlock *block) override;
		void add(Pipeline pipeline) override;
		void execute(bool debug=false);

		Pipeline operator|(Pipeline to);
		Pipeline operator|(PipelineList to);
		Pipeline operator|(Var& to);

		PipelineAggregatorProxy operator[](unsigned idx);
	};

}

#endif /* SEQ_SEQ_H */
