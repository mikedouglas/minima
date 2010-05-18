/*
 *  Copy the character to the memory pointer
 */
memset(pch, c, len)
	void *pch;
	int c;
	int len;
{
	register int n;

	if ((n = len) <= 0)
		return;
	do
		*(char *)pch++ = c;
	while (--n);
}
