#include "seq/seq.h"

using namespace seq;
using namespace llvm;

static void ensureNonVoid(types::Type *type)
{
	if (type->is(types::Void))
		throw exc::SeqException("cannot load or store void variable");
}

Var::Var(types::Type *type) :
    type(type), ptr(nullptr), global(false), mapped()
{
}

void Var::allocaIfNeeded(BaseFunc *base)
{
	if (!mapped.empty())
		mapped.top()->allocaIfNeeded(base);

	if (ptr)
		return;

	LLVMContext& context = base->getContext();
	if (global) {
		Type *llvmType = getType()->getLLVMType(context);
		ptr = new GlobalVariable(*base->getPreamble()->getModule(),
		                         llvmType,
		                         false,
		                         GlobalValue::PrivateLinkage,
		                         Constant::getNullValue(llvmType),
		                         "var");
	} else {
		ptr = makeAlloca(getType()->getLLVMType(context), base->getPreamble());
	}
}

bool Var::isGlobal()
{
	if (!mapped.empty())
		return mapped.top()->isGlobal();
	return global;
}

void Var::setGlobal()
{
	if (!mapped.empty())
		mapped.top()->setGlobal();
	else
		global = true;
}

void Var::mapTo(Var *other)
{
	mapped.push(other);
}

void Var::unmap()
{
	mapped.pop();
}

Value *Var::load(BaseFunc *base, BasicBlock *block)
{
	if (!mapped.empty())
		return mapped.top()->load(base, block);

	ensureNonVoid(getType());
	allocaIfNeeded(base);
	IRBuilder<> builder(block);
	return builder.CreateLoad(ptr);
}

void Var::store(BaseFunc *base, Value *val, BasicBlock *block)
{
	if (!mapped.empty()) {
		mapped.top()->store(base, val, block);
		return;
	}

	ensureNonVoid(getType());
	allocaIfNeeded(base);
	IRBuilder<> builder(block);
	builder.CreateStore(val, ptr);
}

void Var::setType(types::Type *type)
{
	if (!mapped.empty())
		mapped.top()->setType(type);
	else
		this->type = type;
}

types::Type *Var::getType()
{
	if (!mapped.empty())
		return mapped.top()->getType();

	assert(type);
	return type;
}

Var *Var::clone(Generic *ref)
{
	if (isGlobal())
		return this;

	if (ref->seenClone(this))
		return (Var *)ref->getClone(this);

	// we intentionally don't clone this->mapped; should be set in codegen if needed
	auto *x = new Var();
	ref->addClone(this, x);
	if (type) x->setType(type->clone(ref));
	return x;
}
