#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	int a;
	printf("Oi, digite um numero inteiro: ");
	scanf("%d", &a);

	printf("Voce digitou: %d", a);
}