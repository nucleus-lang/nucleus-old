#include <iostream>

int printi(int p)
{
	printf("%i", p);
	return 0;
}

int main(int argc, char const *argv[])
{
	for(int i = 0; i < 1000; i++)
	{
		printi(i);
	}

	return 0;
}