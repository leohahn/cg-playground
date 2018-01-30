#ifndef __INPUT_HPP__
#define __INPUT_HPP__

#define NUM_KEYBOARD_KEYS 1024

struct Key
{
	enum Transition
	{
		Transition_None = 0,
		Transition_Up = 1,
		Transition_Down = 2,
	};

	bool is_pressed;
	Transition last_transition;
};

#endif // __INPUT_HPP__
