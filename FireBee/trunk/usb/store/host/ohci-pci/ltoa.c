/*
 * File:        ltoa.c
 * Purpose:     Function normally found in a standard C lib.
 *
 * Notes:       This supports ASCII only!!!
 */

void ltoa(char *buf, long n, unsigned long base)
{
	unsigned long un;
	char *tmp, ch;
	un = n;
	if((base == 10) && (n < 0))
	{
		*buf++ = '-';
		un = -n;
	}
	tmp = buf;
	do
	{
		ch = un % base;
		un = un / base;
		if(ch <= 9)
			ch += '0';
		else
			ch += 'a' - 10;
		*tmp++ = ch;
	}
	while(un);
	*tmp = '\0';
	while(tmp > buf)
	{
		ch = *buf;
		*buf++ = *--tmp;
		*tmp = ch;
	}
}

