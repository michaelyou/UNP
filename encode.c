#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
char from_hex(char ch)
{
	return isdigit(ch) ? ch - '0': tolower(ch) - 'a' + 10;
}
int main()
{
	char *url = "%e6%88%91%e7%88%b1%e4%bd%a0%e4%b8%ad%e5%9b%bd";
	printf("%s\n",url);
	if(strstr(url,"%"))
	{
		char *buf = malloc(strlen(url) + 1);
		char *pstr = url;
		char *pbuf = buf;
		while(*pstr)
		{
			if(*pstr == '%')
			{
				if(pstr[1] && pstr[2])
				{
					*pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
					pstr += 2;
				}
			}
			else if(*pstr == '+')
			{
				*pbuf++ = ' ';
			}
			else
			{
				*pbuf++ = *pstr;
			}
			pstr++;
		}
		*pbuf = '\0';
		printf("%s\n", buf);
	    free(buf);
	}
	return 0;
}

