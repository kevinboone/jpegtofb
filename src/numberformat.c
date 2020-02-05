/*==========================================================================

  boilerplate
  numberformat.c
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

==========================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <wchar.h>
#include <time.h>
#include <math.h>
#include "string.h" 
#include "wstring.h" 
#include "log.h" 
#include "numberformat.h" 
#include "feature.h" 


#ifdef FEATURE_NUMCONVERSION

/*==========================================================================

  numberformat_eng

  Format a number in engineering notation -- if the exponent is < 15,
    use one of the standard engineering suffices -- m, k, μ, etc. 
  If the exponent is not in this range, write "E23" or "E-50" or
    whatever. Note that this function is defined to return UTF8*,
    not char*, as a reminder that the result may contain multi-byte
    characters.

  As is common practice, the mantissa is displayed to no more than
    three significant figures.

==========================================================================*/
UTF8* numberformat_eng (double v)
  {
  LOG_IN

  BOOL free_suffix = FALSE;
  double m;
  int e;
  numberformat_eng_reduce (v, &m, &e);
  char *suffix = NULL;
  switch (e)
    {
    case 0: suffix = ""; break;
    case 3: suffix = "k"; break;
    case 6: suffix = "M"; break;
    case 9: suffix = "G"; break;
    case 12: suffix = "T"; break;
    case 15: suffix = "P"; break;
    case -3: suffix = "m"; break;
    case -6: suffix = "μ"; break;
    case -9: suffix = "n"; break;
    case -12: suffix = "p"; break;
    case -15: suffix = "f"; break;
    default: asprintf (&suffix, "E%d", e); free_suffix = TRUE;
    }
 
  char *ret;
  asprintf (&ret, "%1.3G%s", m, suffix);
  if (free_suffix) free (suffix);

  LOG_OUT
  return (UTF8 *)ret;
  }


/*==========================================================================

  numberformat_eng_reduce 

  Convert a number to a mantissa and exponent, where the exponent is
    an integer power of three. Note that exponents are not symmetrical
    about exponent zero; that is, 0.1 is 100E-3, while 10 is 1E1 This
    is standard engineering practice -- a number less than one, but
    greater than 10-3, is a mili-something.

==========================================================================*/
void numberformat_eng_reduce (double v, double *mantissa, int *exponent)
  {
  LOG_IN

  if (v == 0) 
    {
    *mantissa = 0; *exponent = 0;
    }
  else
    {
    BOOL neg = (v < 0);
    if (neg) v = -v;
    double exp = log10 (v);
    int iexp = (int)exp;
    if (exp < 0)
      iexp -= 3;
    iexp = iexp / 3 * 3;
    *exponent = iexp;
     
    *mantissa = v / (pow (10, iexp));
    if (neg) *mantissa = -*mantissa;
    }
  
  
  LOG_OUT
  }

#endif // FEATURE_NUMCONVERSION

/*==========================================================================

  numberformat_size_64

  Format a 64-bit unsigned number, assumed to be the size of something
   (file, disk), so that if possible it is expressed using only 
   three digits (e.g., 1.23KB) and a suffix. 
  
  If 'binary' is TRUE, the units are KiB, MiB, GiB, and TiB, which
    are all integer powers of two. Otherwise, the units are KB, MB, 
    GB, and TB, all integer powers of 10. Note that in decimal
    mode, 1KB=1000B, not 1024, as is the modern practice. Nearly all
    software that displays sizes now uses the IEC standard binary
    units -- GiB, TiB, etc -- with the exception of Microsoft.

  If sep is non-null, then numbers over
   1000 are separated into groups of three digits using the supplied
   separator. This will only happen in binary mode. 

==========================================================================*/
char *numberformat_size_64 (uint64_t n, const char *sep, BOOL binary)
  {
  LOG_IN
  char *ret;

  uint64_t kil = binary ? 1024 : 1000; 

  char *ksuff = binary ? "KiB" : "KB";
  char *msuff = binary ? "MiB" : "MB";
  char *gsuff = binary ? "GiB" : "GB";
  char *tsuff = binary ? "TiB" : "TB";

  uint64_t meg = kil * kil; 
  uint64_t gig = kil * meg; 
  uint64_t ter = kil * gig; 

  if (1) // For future expansion
    {
    if (n < 1024) 
      {
      char *out= numberformat_space_64 (n, sep);
      asprintf (&ret, "%sB", out);
      }
    else if (n < meg)
      {
      uint64_t iwhole = n / kil;
      uint64_t ifrac = 100 * (n % kil) / kil; 
      uint64_t t = 1000 * (n % kil) / kil - 10 * ifrac; 
      if (t >= 5) ifrac++;
      char *whole = numberformat_space_64 (iwhole, sep);
      asprintf (&ret, "%s.%02d%s", whole, (int)ifrac, ksuff);
      free (whole); 
      }
    else if (n < gig)
      {
      uint64_t iwhole = n / meg;
      uint64_t ifrac = 100 * (n % meg) / meg; 
      uint64_t t = 1000 * (n % meg) / meg - 10 * ifrac; 
      if (t >= 5) ifrac++;
      char *whole = numberformat_space_64 (iwhole, sep);
      asprintf (&ret, "%s.%02d%s", whole, (int)ifrac, msuff);
      free (whole); 
      }
    else if (n < ter)
      {
      uint64_t iwhole = n / gig;
      uint64_t ifrac = 100 * (n % gig) / gig; 
      uint64_t t = 1000 * (n % gig) / gig - 10 * ifrac; 
      if (t >= 5) ifrac++;
      char *whole = numberformat_space_64 (iwhole, sep);
      asprintf (&ret, "%s.%02d%s", whole, (int)ifrac, gsuff);
      free (whole); 
      }
    else 
      {
      uint64_t iwhole = n / ter;
      uint64_t ifrac = 100 * (n % ter) / ter; 
      uint64_t t = 1000 * (n % ter) / ter - 10 * ifrac; 
      if (t >= 5) ifrac++;
      char *whole = numberformat_space_64 (iwhole, sep);
      asprintf (&ret, "%s.%02d%s", whole, (int)ifrac, tsuff);
      free (whole); 
      }
    }

  LOG_OUT
  return ret;
  }


/*==========================================================================

  numberformat_space_64

  Format a 64-bit (or smaller) integer by separating the digits into
    groups of three. The separator can be any sequence of unicode or
    ASCII bytes

  It's safe to supply a NULL for the separator, which is useful 
    in cases where this function is called by other number-formatting
    routines that may, or may not, require separators. 

==========================================================================*/
char *numberformat_space_64 (int64_t n, const char *sep)
  {
  LOG_IN

  char *ret;
  if (n == 0)
    {
    ret = strdup ("0");
    }
  else
    {
    BOOL neg = FALSE;
    if (n < 0) 
      {
      neg = TRUE;
      n = -n;
      }

    char in[30];
    ret = malloc (40);
    ret[0] = 0;
    sprintf (in, "%ld", n);
    int lin = strlen (in);

    int before = (lin % 3);

    for (int i = 0; i < lin; i++)
      {
      char t[2]; t[0] = in[i]; t[1] = 0;
      strncat (ret, t, 2);
      if (sep)
        {
        if ((before - 1) % 3 == 0 && i != lin - 1)
          {
          strcat (ret, sep);
          }
        before--;
        }
      }

    if (neg)
      {
      char *out2 = malloc (strlen (ret) + 2);
      strcpy (out2 + 1, ret);
      out2[0] = '-';
      free (ret);
      ret = out2;
      }
    }

  LOG_OUT 
  return ret; 
  }


/*==========================================================================

  numberformat_read_integer

  Read a 64-bit decimal integer from a string. If strict is TRUE, then
    the entire string must consist of digits, optionally preceded by
    + or -. If FALSE, there may be
    any amount of whitespace at the start of the number, and
    any amount of anything at the end.

  This function can't read hexadecimal numbers, but (weirdly) 
    numberformat_read_double can.

==========================================================================*/
BOOL numberformat_read_integer (const char *s, uint64_t *v, BOOL strict)
  {
  LOG_IN
  BOOL ret = FALSE;
  if (strlen (s) == 0)
    { 
    // Empty -- no way it's a number
    ret = FALSE;
    }
  else
    {
    char first = s[0];
    if ((first >= '0' && first <= '9') || first == '+' || first == '-' 
         || !strict)
      {
      char *endp;
      uint64_t n = strtoll (s, &endp, 10);
      if (endp == s) 
        {
        ret = FALSE; // No digits
        }
      else
        {
        if (*endp == 0)
          {
          ret = TRUE;
          }
        else
          {
          // We read _some_ digits, so this is valid except
          //   in strict mode
          ret = !strict;
          }
        }
      *v = n;
      }
    }
  LOG_OUT
  return ret;
  }


/*==========================================================================

  numberformat_read_double

  Read a decimal floating point number from a string. If strict is TRUE, then
    the number must start at the beginning of the string, and extend to
    the end. If FALSE, there may be
    any amount of whitespace at the start of the number, and
    any amount of anything at the end.

  This function understands scientific notation ("1.2E3"). The decimal
    separator is locale-specific.

  By a weird quirk of the way strtod is implemented, this function
    will also read a hexadecimal number, if it starts with 0x.

==========================================================================*/
BOOL numberformat_read_double (const char *s, double *v, BOOL strict)
  {
  LOG_IN
  BOOL ret = FALSE;
  if (strlen (s) == 0)
    { 
    // Empty -- no way it's a number
    ret = FALSE;
    }
  else
    {
    char first = s[0];
    if ((first >= '0' && first <= '9') || first == '+' || first == '-' 
        || first == '.' || first == ','  || !strict)
      {
      char *endp;
      double n = strtod (s, &endp);
      if (endp == s) 
        {
        ret = FALSE; // No digits
        }
      else
        {
        if (*endp == 0)
          {
          ret = TRUE;
          }
        else
          {
          // We read _some_ digits, so this is valid except
          //   in strict mode
          ret = !strict;
          }
        }
      *v = n;
      }
    }
  LOG_OUT
  return ret;
  }



