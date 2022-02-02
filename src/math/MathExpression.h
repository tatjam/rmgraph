#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <rmkit.h>

struct MathContext;

using numer_t = double;

struct MathToken
{
	enum Type
	{
		VARIABLE,  			// x, y, a, b, c...
		NUMBER, 			// 10, 15, 1.4, pi, e...
		OPERATOR,			// + - * / ^
		COMMA,				// It's simply ignored
		FUNCTION,			// sqrt, cos... Affects the next group of tokens, so use parens!
		DUMMY,				// [] Created by the editor to always have valid RPN
		LPAREN,				// (
		RPAREN,				// )
	};

	Type type;
	std::string value;

	int get_precedence();
	int get_argument_count() const;
	std::optional<numer_t> get_value(MathContext* context);
	numer_t operate(numer_t x, numer_t y) const;
	numer_t function(std::vector<numer_t> x);

	int render_x, render_y;
	int render_w, render_h;
	// For parenthesis rendering
	int old_height;
	// Clear space to the right of the symbol
	int render_offset;
};



class MathExpression
{
private:

	int prev_cursorx0 = -1, prev_cursory = -1, prev_cursorx1 = -1;

	// finds the operators of given type of minimum depth within two parenthesis
	std::vector<size_t> find_minimum_depth_operators(std::string op_value, int lpar, int rpar);
	// Finds leftwards right paren, and rightwards left paren (the others are obvious, pos + 1, pos - 1)
	std::pair<int, int> find_left_right_pair(size_t pos);
	// Optionally pass yoff to return how much we move upwards
	void dimension_div(size_t pos, int* yoff = nullptr);
	void find_div_num_denum_sizes(size_t pos, bool dimension_child, int& nw, int& dw, int& nh, int& dh);
	// Recursive, draws all child divs
	void draw_div(int sx, int sy, size_t pos, framebuffer::FB* fb);
	void draw_advanced_expr(int x, int y, int start, int end, framebuffer::FB* fb, bool in_pars);

	std::vector<std::pair<int, int>>
	find_simple_exponents(int start, int end, std::vector<int>& ignore,std::vector<int>& exit_par);

public:
	// If set, contains the error code
	std::string error;
	// For difference rendering
	int last_maxx;
	std::vector<MathToken> old_tokens;
	std::vector<MathToken> tokens;
	// If it returns a value, it was possible to evaluate the expression down to a numeric
	// value, otherwise it was not!
	std::optional<numer_t> evaluate(MathContext* in_context);
	std::optional<std::queue<MathToken>> get_rpn();

	void draw(int sx, int sy, framebuffer::FB* fb, bool is_editing = true);
	void draw_simple(int sx, int sy, framebuffer::FB* fb, bool is_editing);
	void draw_advanced(int sx, int sy, framebuffer::FB* fb, bool is_editing);
	// Adjusts y as equations may also grow upwards
	std::pair<int, int> get_dimensions(int& y);
	void dimension(int& target_size, int& target_height, int start, int end,
				   std::vector<std::pair<int, int>> pars, std::vector<size_t> divs);
	void draw_cursor(size_t working_pos, framebuffer::FB* fb);
	void get_render(size_t pos, int& width, int& off, int& offx, std::string& text, bool simp_mult);

	// We only allow = y or = x to be implicit, we don't check if it can be cleared for x / y
	bool is_explicit();
	// is the side just an x or just an y?
	bool is_x_or_y();
	// first value if dependency on x, second on y
	std::pair<bool, bool> depends_on_xy();
	// For explicit equations
	std::optional<numer_t> get_value_of_x(MathContext* in_context);
	std::optional<numer_t> get_value_of_y(MathContext* in_context);

	std::pair<MathExpression, MathExpression> get_sides();
};

// A context associates variables with expressions
struct MathContext
{
	// Fixed values are an expression which can be simplified to free values only (or to a numerical value)
	std::unordered_map<std::string, MathExpression> fixed_values;
	// Free values may be set externally to finally obtain a numerical result,
	// for example, parameters or the x and y variables
	std::unordered_map<std::string, numer_t> free_values;
};
