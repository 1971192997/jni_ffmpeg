#include <windows.h>
long check_CoTaskMemFree(void) { return (long) CoTaskMemFree; }
int main(void) { return 0; }
