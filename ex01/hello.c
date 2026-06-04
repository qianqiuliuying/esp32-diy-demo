#include <stdio.h>

int main() 
{
    printf("Hello, World!\n");
    char a[128];
    printf("Please enter content:");
    scanf("%s",a); 
    printf("Your input: %s\n",a);
    return 0;
}
