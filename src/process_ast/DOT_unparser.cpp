/*
 * phc -- the open source PHP compiler
 * See doc/license/README.license for licensing information
 *
 * Convert the phc AST to DOT format
 */

#include <iostream>

#include "cmdline.h"
#include "lib/demangle.h"
#include "lib/escape.h"

#include "DOT_unparser.h"
#include "process_ir/General.h"

using namespace AST;
using namespace std;

extern struct gengetopt_args_info args_info;

DOT_unparser::DOT_unparser() 
{
	new_node_id = 0;
}

DOT_unparser::~DOT_unparser() 
{
}

void DOT_unparser::visit_php_script(PHP_script* in)
{
	cout << "/*" << endl;
	cout << " * AST in dot format generated by phc -- the PHP compiler" << endl;
	cout << " */" << endl << endl;
	cout << "digraph AST {" << endl;
	cout << "ordering=out;" << endl;

	Visitor::visit_php_script(in);

	cout << "}" << endl;
}

void DOT_unparser::pre_node(Node* in)
{
	new_node(demangle(in, false), in->get_line_number());
}

void DOT_unparser::post_node(Node* in)
{
	node_stack.pop();
}

void DOT_unparser::pre_identifier(Identifier* in)
{
	new_box(in->get_value_as_string());
}

void DOT_unparser::pre_literal(Literal* in)
{
	new_box(escape_DOT (in->get_source_rep()));
}

void DOT_unparser::visit_marker(char const* name, bool value)
{
	if(value)
	{
		cout << "node_" << new_node_id << " [label=\"" << name << "\", shape=plaintext]" << endl;
		add_link(new_node_id++);
	}
}

void DOT_unparser::visit_null(char const* name_space, char const* type_id)
{
	if(!args_info.no_dot_nulls_flag)
	{
		cout << "node_" << new_node_id << " [label=\"NULL\\n(" << type_id << ")\", shape=plaintext]" << endl;
		add_link(new_node_id++);
	}
}

void DOT_unparser::visit_null_list(char const* name_space, char const* type_id)
{
	if(!args_info.no_dot_nulls_flag)
	{
		cout << "node_" << new_node_id << " [label=\"NULL\\n(List<" << type_id << ">)\", shape=plaintext]" << endl;
		add_link(new_node_id++);
	}
}

void DOT_unparser::pre_list(char const* name_space, char const* type_id, int size)
{
	if(!args_info.no_dot_empty_lists_flag || size > 0)
	{
		stringstream s;
		s << "List<" << type_id << ">";
		new_node(s.str().c_str(), 0);
	}
}

void DOT_unparser::post_list(char const* name_space, char const* type_id, int size)
{
	if(!args_info.no_dot_empty_lists_flag || size > 0)
	{
		node_stack.pop();
	}
}

void DOT_unparser::new_node(char const* label, int line_number)
{
	cout << "node_" << new_node_id << " [label=\"" << label;
	if(!args_info.no_dot_line_numbers_flag && line_number != 0)
		cout << " (" << line_number << ")";
	cout << "\"];" << endl;	

	add_link(new_node_id);
	node_stack.push(new_node_id++);
}

void DOT_unparser::new_box(String* source_rep)
{
	if(source_rep)
		cout << "node_" << new_node_id << " [label=\"" << *source_rep << "\", shape=box]" << endl;
	else
		cout << "node_" << new_node_id << " [label=\"???\", shape=box]" << endl;
	add_link(new_node_id++);
}

void DOT_unparser::add_link(int target)
{
	if(!node_stack.empty())
		cout << "node_" << node_stack.top() << " -> node_" << target << ";" << endl;
}

