#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "cpp.h"

extern	int getopt(int, char *const *, const char *);
extern	char	*optarg, rcsid[];
extern	int	optind;
int	verbose;
int	Mflag;	/* only print active include files */
char	*objname; /* "src.$O: " */
int	Cplusplus = 1;

void
setup(int argc, char **argv)
{
	int c, i;
	FILE *fd;
	char *fp, *dp;
	Tokenrow tr;
	extern void setup_kwtab(void);

	setup_kwtab();
	while ((c = getopt(argc, argv, "MNOVv+I:D:U:F:lg")) != -1)
		switch (c) {
		case 'N':
			for (i=0; i<NINCLUDE; i++)
				if (includelist[i].always==1)
					includelist[i].deleted = 1;
			break;
		case 'I':
			for (i=NINCLUDE-2; i>=0; i--) {
				if (includelist[i].file==NULL) {
					includelist[i].always = 1;
					includelist[i].file = optarg;
					break;
				}
			}
			if (i<0)
				error(FATAL, "Too many -I directives");
			break;
		case 'D':
			/* Hack to accept `-Df(x)=x` style definitions. */
			{
				char *p, s[1024];
				snprintf(s, sizeof(s), "define %s", optarg);
				if ((p = strchr(s, '='))) *p = ' ';
				setsource("<cmdarg>", NULL, s);
				maketokenrow(3, &tr);
				gettokens(&tr, 1);
				dodefine(&tr);
				unsetsource();
				break;
			}
		case 'U':
			setsource("<cmdarg>", NULL, optarg);
			maketokenrow(3, &tr);
			gettokens(&tr, 1);
			doundef(&tr);
			unsetsource();
			break;
		case 'M':
			Mflag++;
			break;
		case 'v':
			fprintf(stderr, "%s %s\n", argv[0], rcsid);
			break;
		case 'V':
			verbose++;
			break;
		case '+':
			Cplusplus++;
			break;
		default:
			break;
		}
	dp = ".";
	fp = "<stdin>";
	fd = stdin;
	if (optind<argc) {
		if ((fp = strrchr(argv[optind], '/')) != NULL) {
			int len = fp - argv[optind];
			dp = (char*)newstring((uchar*)argv[optind], len+1, 0);
			dp[len] = '\0';
		}
		fp = (char*)newstring((uchar*)argv[optind], strlen(argv[optind]), 0);
		if ((fd = fopen(fp, "r")) == NULL)
			error(FATAL, "Can't open input file %s", fp);
	}
	if (optind+1<argc) {
		FILE *fdo = freopen(argv[optind+1], "w", stdout);
		if (fdo == NULL)
			error(FATAL, "Can't open output file %s", argv[optind+1]);
	}
	if(Mflag)
		setobjname(fp);
	includelist[NINCLUDE-1].always = 0;
	includelist[NINCLUDE-1].file = dp;
	setsource(fp, fd, NULL);
}
