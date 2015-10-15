#ifndef RANDOM_H
#define RANDOM_H

class CRandom
{
	CRandom();

public:
	static INT32 Random();
	static INT32 SimpleRandom();
	static INT32 ComplexRandom();
	static INT32 RandRange(INT32 lower, INT32 upper);
};

#endif // RANDOM_H 