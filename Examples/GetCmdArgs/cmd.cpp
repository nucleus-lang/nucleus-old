#include <iostream>

int printString(const char* s)
{
	printf(s);
	return 0;
}

int main(int argc, char const *argv[])
{
	const char* StringOne = "This is String One!";
	const char* StringTwo = "This is String Two!";
	const char* StringThr = "This is String Three!";

	const char* Array[3] = {StringOne, StringTwo, StringThr};

	printString(Array[0]);

	return 0;
}