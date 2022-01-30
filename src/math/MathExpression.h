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

	// Set by the maths keyboard
	int render_x, render_y;
	int render_w, render_h;
	// Clear space to the right of the symbol
	int render_offset;
};



class MathExpression
{
private:

	int prev_cursorx0 = -1, prev_cursory = -1, prev_cursorx1 = -1;

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

	void draw(int sx, int sy, framebuffer::FB* fb);
	void draw_simple(int sx, int sy, framebuffer::FB* fb);
	void draw_advanced(int sx, int sy, framebuffer::FB* fb);
	void draw_cursor(size_t working_pos, framebuffer::FB* fb);
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
