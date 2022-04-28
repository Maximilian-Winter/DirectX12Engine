#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_
#include "Singleton.h"


class Input : public Singleton<Input>
{
	friend class Singleton<Input>;
public:
	Input();
	Input(const Input&);
	~Input();

	void Initialize();

	void KeyDown(unsigned int);
	void KeyUp(unsigned int);

	bool IsKeyDown(unsigned int);

private:
	bool m_keys[256];
};

#endif