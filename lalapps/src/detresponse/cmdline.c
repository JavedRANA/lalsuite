/*
  File autogenerated by gengetopt version 2.8
  generated with the following command:
  gengetopt 

  The developers of gengetopt consider the fixed text that goes in all
  gengetopt output files to be in the public domain:
  we make no copyright claims on it.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* If we use autoconf.  */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
/* Check for configure's getopt check result.  */
#ifndef HAVE_GETOPT_LONG
#include "getopt.h"
#else
#include <getopt.h>
#endif

#ifndef HAVE_STRDUP
#define strdup gengetopt_strdup
#endif /* HAVE_STRDUP */

#include "cmdline.h"


void
cmdline_parser_print_version (void)
{
  printf ("%s %s\n", PACKAGE, VERSION);
}

void
cmdline_parser_print_help (void)
{
  cmdline_parser_print_version ();
  printf("\n"
  "Purpose:\n"
  "  compute time series of detector response function (antenna pattern)\n"
  "\n"
  "Usage: %s [OPTIONS]...\n", PACKAGE);
  printf("   -h         --help                      Print help and exit\n");
  printf("   -V         --version                   Print version and exit\n");
  printf("   -NSTRING   --source-name=STRING        Name of source (default='NONAME_SOURCE')\n");
  printf("   -rDOUBLE   --right-ascension=DOUBLE    Right Ascension of source, in rad\n");
  printf("   -dDOUBLE   --declination=DOUBLE        Declination of source, in rad\n");
  printf("   -oDOUBLE   --orientation=DOUBLE        Orientation of source, in rad\n");
  printf("   -DSTRING   --detector=STRING           Detector name; legal names: lho, llo, virgo, geo, tama, cit, test\n");
  printf("   -sINT      --start-time-sec=INT        GPS seconds field of start time of observation\n");
  printf("   -nINT      --start-time-nanosec=INT    GPS nanoseconds field of start time of observation (default='0')\n");
  printf("   -uINT      --nsample=INT               number of samples\n");
  printf("   -iDOUBLE   --sampling-interval=DOUBLE  sampling time interval, in seconds\n");
  printf("   -vINT      --verbosity=INT             verbosity level for debugging (default='0')\n");
  printf("   -eINT      --debug=INT                 debug level (default='0')\n");
}


#ifndef HAVE_STRDUP
/* gengetopt_strdup(): automatically generated from strdup.c. */
/* strdup.c replacement of strdup, which is not standard */
static char *
gengetopt_strdup (const char *s)
{
  char *result = (char*)malloc(strlen(s) + 1);
  if (result == (char*)0)
    return (char*)0;
  strcpy(result, s);
  return result;
}
#endif /* HAVE_STRDUP */

int
cmdline_parser (int argc, char * const *argv, struct gengetopt_args_info *args_info)
{
  int c;	/* Character of the parsed option.  */
  int missing_required_options = 0;

  args_info->help_given = 0 ;
  args_info->version_given = 0 ;
  args_info->source_name_given = 0 ;
  args_info->right_ascension_given = 0 ;
  args_info->declination_given = 0 ;
  args_info->orientation_given = 0 ;
  args_info->detector_given = 0 ;
  args_info->start_time_sec_given = 0 ;
  args_info->start_time_nanosec_given = 0 ;
  args_info->nsample_given = 0 ;
  args_info->sampling_interval_given = 0 ;
  args_info->verbosity_given = 0 ;
  args_info->debug_given = 0 ;
#define clear_args() { \
  args_info->source_name_arg = strdup("NONAME_SOURCE") ;\
  args_info->detector_arg = NULL; \
  args_info->start_time_nanosec_arg = 0 ;\
  args_info->verbosity_arg = 0 ;\
  args_info->debug_arg = 0 ;\
}

  clear_args();

  optarg = 0;
  optind = 1;
  opterr = 1;
  optopt = '?';

  while (1)
    {
      int option_index = 0;
      char *stop_char;
      static struct option long_options[] = {
        { "help",	0, NULL, 'h' },
        { "version",	0, NULL, 'V' },
        { "source-name",	1, NULL, 'N' },
        { "right-ascension",	1, NULL, 'r' },
        { "declination",	1, NULL, 'd' },
        { "orientation",	1, NULL, 'o' },
        { "detector",	1, NULL, 'D' },
        { "start-time-sec",	1, NULL, 's' },
        { "start-time-nanosec",	1, NULL, 'n' },
        { "nsample",	1, NULL, 'u' },
        { "sampling-interval",	1, NULL, 'i' },
        { "verbosity",	1, NULL, 'v' },
        { "debug",	1, NULL, 'e' },
        { NULL,	0, NULL, 0 }
      };

      c = getopt_long (argc, argv, "hVN:r:d:o:D:s:n:u:i:v:e:", long_options, &option_index);

      if (c == -1) break;	/* Exit from `while (1)' loop.  */

      switch (c)
        {
        case 'h':	/* Print help and exit.  */
          clear_args ();
          cmdline_parser_print_help ();
          exit (EXIT_SUCCESS);

        case 'V':	/* Print version and exit.  */
          clear_args ();
          cmdline_parser_print_version ();
          exit (EXIT_SUCCESS);

        case 'N':	/* Name of source.  */
          if (args_info->source_name_given)
            {
              fprintf (stderr, "%s: `--source-name' (`-N') option given more than once\n", PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->source_name_given = 1;
          args_info->source_name_arg = strdup (optarg);
          break;

        case 'r':	/* Right Ascension of source, in rad.  */
          if (args_info->right_ascension_given)
            {
              fprintf (stderr, "%s: `--right-ascension' (`-r') option given more than once\n", PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->right_ascension_given = 1;
          args_info->right_ascension_arg = strtod (optarg, NULL);
          break;

        case 'd':	/* Declination of source, in rad.  */
          if (args_info->declination_given)
            {
              fprintf (stderr, "%s: `--declination' (`-d') option given more than once\n", PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->declination_given = 1;
          args_info->declination_arg = strtod (optarg, NULL);
          break;

        case 'o':	/* Orientation of source, in rad.  */
          if (args_info->orientation_given)
            {
              fprintf (stderr, "%s: `--orientation' (`-o') option given more than once\n", PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->orientation_given = 1;
          args_info->orientation_arg = strtod (optarg, NULL);
          break;

        case 'D':	/* Detector name; legal names: lho, llo, virgo, geo, tama, cit, test.  */
          if (args_info->detector_given)
            {
              fprintf (stderr, "%s: `--detector' (`-D') option given more than once\n", PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->detector_given = 1;
          args_info->detector_arg = strdup (optarg);
          break;

        case 's':	/* GPS seconds field of start time of observation.  */
          if (args_info->start_time_sec_given)
            {
              fprintf (stderr, "%s: `--start-time-sec' (`-s') option given more than once\n", PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->start_time_sec_given = 1;
          args_info->start_time_sec_arg = strtol (optarg,&stop_char,0);
          break;

        case 'n':	/* GPS nanoseconds field of start time of observation.  */
          if (args_info->start_time_nanosec_given)
            {
              fprintf (stderr, "%s: `--start-time-nanosec' (`-n') option given more than once\n", PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->start_time_nanosec_given = 1;
          args_info->start_time_nanosec_arg = strtol (optarg,&stop_char,0);
          break;

        case 'u':	/* number of samples.  */
          if (args_info->nsample_given)
            {
              fprintf (stderr, "%s: `--nsample' (`-u') option given more than once\n", PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->nsample_given = 1;
          args_info->nsample_arg = strtol (optarg,&stop_char,0);
          break;

        case 'i':	/* sampling time interval, in seconds.  */
          if (args_info->sampling_interval_given)
            {
              fprintf (stderr, "%s: `--sampling-interval' (`-i') option given more than once\n", PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->sampling_interval_given = 1;
          args_info->sampling_interval_arg = strtod (optarg, NULL);
          break;

        case 'v':	/* verbosity level for debugging.  */
          if (args_info->verbosity_given)
            {
              fprintf (stderr, "%s: `--verbosity' (`-v') option given more than once\n", PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->verbosity_given = 1;
          args_info->verbosity_arg = strtol (optarg,&stop_char,0);
          break;

        case 'e':	/* debug level.  */
          if (args_info->debug_given)
            {
              fprintf (stderr, "%s: `--debug' (`-e') option given more than once\n", PACKAGE);
              clear_args ();
              exit (EXIT_FAILURE);
            }
          args_info->debug_given = 1;
          args_info->debug_arg = strtol (optarg,&stop_char,0);
          break;


        case 0:	/* Long option with no short option */

        case '?':	/* Invalid option.  */
          /* `getopt_long' already printed an error message.  */
          exit (EXIT_FAILURE);

        default:	/* bug: option not considered.  */
          fprintf (stderr, "%s: option unknown: %c\n", PACKAGE, c);
          abort ();
        } /* switch */
    } /* while */


  if (! args_info->right_ascension_given)
    {
      fprintf (stderr, "%s: '--right-ascension' ('-r') option required\n", PACKAGE);
      missing_required_options = 1;
    }
  if (! args_info->declination_given)
    {
      fprintf (stderr, "%s: '--declination' ('-d') option required\n", PACKAGE);
      missing_required_options = 1;
    }
  if (! args_info->orientation_given)
    {
      fprintf (stderr, "%s: '--orientation' ('-o') option required\n", PACKAGE);
      missing_required_options = 1;
    }
  if (! args_info->detector_given)
    {
      fprintf (stderr, "%s: '--detector' ('-D') option required\n", PACKAGE);
      missing_required_options = 1;
    }
  if (! args_info->start_time_sec_given)
    {
      fprintf (stderr, "%s: '--start-time-sec' ('-s') option required\n", PACKAGE);
      missing_required_options = 1;
    }
  if (! args_info->nsample_given)
    {
      fprintf (stderr, "%s: '--nsample' ('-u') option required\n", PACKAGE);
      missing_required_options = 1;
    }
  if (! args_info->sampling_interval_given)
    {
      fprintf (stderr, "%s: '--sampling-interval' ('-i') option required\n", PACKAGE);
      missing_required_options = 1;
    }
  if ( missing_required_options )
    exit (EXIT_FAILURE);

  return 0;
}
