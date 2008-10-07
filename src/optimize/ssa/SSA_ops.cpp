
#include "process_ir/General.h"

#include "SSA_ops.h"
#include "optimize/Def_use.h"

using namespace MIR;

SSA_op*
SSA_op::for_bb (Basic_block* bb)
{
	// Fail on Phis

	// TODO:handle formal here?
	if (isa<Branch_block> (bb))
		return new SSA_branch (dyc<Branch_block> (bb));
	else
		return new SSA_stmt (dyc<Statement_block> (bb));
}

SSA_phi::SSA_phi (Basic_block* bb, VARIABLE_NAME* phi_lhs)
: bb (bb)
, phi_lhs (phi_lhs)
{
}

SSA_stmt::SSA_stmt (Statement_block* bb) : bb (bb) {}
SSA_branch::SSA_branch (Branch_block* bb) : bb (bb) {}
SSA_formal::SSA_formal (Entry_block* bb) : bb (bb) {}

Statement* SSA_stmt::get_statement () { return bb->statement; }

Basic_block* SSA_stmt::get_bb () { return bb; }
Basic_block* SSA_formal::get_bb () { return bb; }
Basic_block* SSA_branch::get_bb () { return bb; }
Basic_block* SSA_phi::get_bb () { return bb; }

void SSA_stmt::dump()
{
	DEBUG ("SSA_stmt: ");
	bb->dump ();
}

void SSA_branch::dump()
{
	DEBUG ("SSA_branch: ");
	bb->dump ();
}

void SSA_formal::dump()
{
	DEBUG ("SSA_formal: ");
	bb->dump ();
}

void SSA_phi::dump()
{
	DEBUG ("SSA_phi: " << *phi_lhs->get_ssa_var_name () << ", ");
	bb->dump ();
}


bool ssa_op_ptr_comparison (SSA_op* op1, SSA_op* op2)
{
	// order on types first
	if (typeid (*op1) != typeid (*op2))
		return typeid (*op1).name () < typeid (*op2).name ();


	// Dont compare BB indices, they might be overwritten
	if (isa<SSA_phi> (op1))
	{
		SSA_phi* phi1 = dyc<SSA_phi> (op1);
		SSA_phi* phi2 = dyc<SSA_phi> (op2);
		if (phi1->bb != phi2->bb)
			return phi1->bb->vertex < phi2->bb->vertex;
		else
			// compare variable names
			return *phi1->phi_lhs < *phi2->phi_lhs;
	}
	else
	{
		// Compare vertex pointers, this wont change, even when the
		// graph is updated.
		return op1->get_bb ()->vertex < op2->get_bb ()->vertex;
	}
}

VARIABLE_NAME_list*
SSA_stmt::get_uses ()
{
	return bb->cfg->duw->get_nonphi_uses (bb);
}

VARIABLE_NAME_list*
SSA_phi::get_uses ()
{
	VARIABLE_NAME_list* result = new VARIABLE_NAME_list;
	foreach (Rvalue* use, *bb->get_phi_args (phi_lhs))
		if (isa<VARIABLE_NAME> (use))
			result->push_back (dyc<VARIABLE_NAME> (use));

	return result;
}

VARIABLE_NAME_list*
SSA_branch::get_uses ()
{
	return new VARIABLE_NAME_list (bb->branch->variable_name);
}

VARIABLE_NAME_list*
SSA_formal::get_uses ()
{
	return new VARIABLE_NAME_list ();
}
